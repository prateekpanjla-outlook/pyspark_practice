"""
SQL Query Executor
Handles execution of SQL queries in isolated Docker containers
"""

import docker
import asyncio
import uuid
import json
from typing import Dict, List, Any
from docker.errors import DockerException, APIError


class SQLExecutor:
    """Executes SQL queries in isolated Docker containers"""

    def __init__(self):
        self.client = docker.from_env()
        self.active_containers: Dict[str, docker.models.containers.Container] = {}
        self.max_containers = int(os.getenv("MAX_CONTAINERS", "10"))
        self.query_timeout = int(os.getenv("QUERY_TIMEOUT", "30"))
        self.memory_limit = os.getenv("MEMORY_LIMIT", "512m")
        self.cpu_limit = float(os.getenv("CPU_LIMIT", "0.5"))

    async def execute(
        self,
        question_id: str,
        sql: str,
        schema_data: Dict[str, Any]
    ) -> Dict[str, Any]:
        """
        Execute SQL query in isolated container

        Args:
            question_id: Question identifier
            sql: SQL query to execute
            schema_data: Schema and sample data for the question

        Returns:
            Dictionary with columns, rows, execution_time_ms
        """
        container_name = f"sql-sandbox-{uuid.uuid4().hex[:8]}"

        try:
            # Create container
            container = self._create_container(container_name, schema_data)

            # Wait for container to be ready
            await self._wait_for_container(container)

            # Execute query
            result = await self._run_query(container, sql)

            return {
                "columns": result.get("columns", []),
                "rows": result.get("rows", []),
                "execution_time_ms": result.get("execution_time_ms", 0)
            }

        except Exception as e:
            return {
                "columns": [],
                "rows": [],
                "execution_time_ms": 0,
                "error": str(e)
            }

        finally:
            # Cleanup container
            await self._cleanup_container(container_name)

    def _create_container(self, name: str, schema_data: Dict) -> docker.models.containers.Container:
        """Create and start a new PostgreSQL container with sample data"""

        # Generate initialization SQL
        init_sql = self._generate_init_sql(schema_data)

        # Create container
        try:
            container = self.client.containers.run(
                image="postgres:16-alpine",
                name=name,
                environment={
                    "POSTGRES_DB": "sqldb",
                    "POSTGRES_USER": "sqluser",
                    "POSTGRES_PASSWORD": "sqlpass"
                },
                command=["postgres", "-c", "max_connections=200"],
                detach=True,
                mem_limit=self.memory_limit,
                cpu_quota=int(self.cpu_limit * 100000),
                cpu_period=100000,
                network_mode="bridge",  # Isolated network
                remove=True  # Auto-remove on stop
            )

            # Store reference
            self.active_containers[name] = container

            # Wait for PostgreSQL to start
            import time
            time.sleep(3)

            # Execute initialization SQL
            exit_code, output = container.exec_run(
                f'psql -U sqluser -d sqldb -c "{init_sql}"',
                workdir="/"
            )

            return container

        except APIError as e:
            raise DockerException(f"Failed to create container: {str(e)}")

    async def _wait_for_container(self, container: docker.models.containers.Container, timeout: int = 10):
        """Wait for container to be ready"""
        import time

        for _ in range(timeout):
            container.reload()
            if container.status == "running":
                return True
            await asyncio.sleep(1)

        raise TimeoutError("Container failed to start")

    async def _run_query(self, container: docker.models.containers.Container, sql: str) -> Dict[str, Any]:
        """Execute SQL query and return results"""

        import time

        start_time = time.time()

        try:
            # Execute query
            exit_code, output = container.exec_run(
                f'psql -U sqluser -d sqldb -c "{sql.replace(chr(34), chr(92)+chr(34))}" -t -A',
                workdir="/"
            )

            execution_time = int((time.time() - start_time) * 1000)

            if exit_code != 0:
                return {
                    "columns": [],
                    "rows": [],
                    "execution_time_ms": execution_time,
                    "error": output.decode("utf-8")
                }

            # Parse output
            lines = output.decode("utf-8").strip().split("\n")

            if not lines or lines[0] == "":
                return {
                    "columns": [],
                    "rows": [],
                    "execution_time_ms": execution_time
                }

            # First line is column headers
            columns = lines[0].split("|")
            rows = []

            # Subsequent lines are data
            for line in lines[1:]:
                if line.strip():
                    values = line.split("|")
                    row = {}
                    for i, col in enumerate(columns):
                        row[col.strip()] = values[i].strip() if i < len(values) else None
                    rows.append(row)

            return {
                "columns": [c.strip() for c in columns],
                "rows": rows,
                "execution_time_ms": execution_time
            }

        except Exception as e:
            return {
                "columns": [],
                "rows": [],
                "execution_time_ms": int((time.time() - start_time) * 1000),
                "error": str(e)
            }

    async def _cleanup_container(self, container_name: str):
        """Remove and cleanup container"""
        try:
            if container_name in self.active_containers:
                container = self.active_containers[container_name]
                container.stop(timeout=5)
                del self.active_containers[container_name]
        except Exception:
            pass  # Container may already be removed

    def _generate_init_sql(self, schema_data: Dict) -> str:
        """Generate SQL to create tables and insert sample data"""

        tables = schema_data.get("tables", [])
        sample_data = schema_data.get("sample_data", {})

        sql_statements = []

        for table in tables:
            table_name = table["name"]
            columns = table["columns"]

            # CREATE TABLE
            col_defs = []
            for col in columns:
                col_def = f'{col["name"]} {col["type"]}'
                col_defs.append(col_def)

            sql_statements.append(f"CREATE TABLE {table_name} ({', '.join(col_defs)});")

            # INSERT sample data
            if table_name in sample_data:
                for row in sample_data[table_name]:
                    values = []
                    for col in columns:
                        col_name = col["name"]
                        value = row.get(col_name)

                        if value is None:
                            values.append("NULL")
                        elif isinstance(value, str):
                            values.append(f"'{value}'")
                        elif isinstance(value, bool):
                            values.append("TRUE" if value else "FALSE")
                        else:
                            values.append(str(value))

                    sql_statements.append(
                        f"INSERT INTO {table_name} VALUES ({', '.join(values)});"
                    )

        return " ".join(sql_statements)

    def compare_results(self, result: Dict, expected: Dict) -> bool:
        """Compare query result with expected output"""

        result_columns = result.get("columns", [])
        result_rows = result.get("rows", [])

        expected_columns = expected.get("columns", [])
        expected_rows = expected.get("rows", [])

        # Compare columns
        if set(result_columns) != set(expected_columns):
            return False

        # Compare row count
        if len(result_rows) != len(expected_rows):
            return False

        # Compare data
        for i, result_row in enumerate(result_rows):
            if i >= len(expected_rows):
                return False

            expected_row = expected_rows[i]

            for col in expected_columns:
                if str(result_row.get(col)) != str(expected_row.get(col)):
                    return False

        return True

    async def cleanup(self):
        """Cleanup all active containers"""
        for container_name in list(self.active_containers.keys()):
            await self._cleanup_container(container_name)
