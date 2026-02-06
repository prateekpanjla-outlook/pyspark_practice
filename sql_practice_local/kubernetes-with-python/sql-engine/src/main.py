"""
SQL Execution Engine
FastAPI service for executing user SQL queries in isolated Docker containers
"""

from fastapi import FastAPI, HTTPException
from fastapi.middleware.cors import CORSMiddleware
import os

from src.core.executor import SQLExecutor
from src.config.settings import settings

# Create FastAPI app
app = FastAPI(
    title="SQL Execution Engine",
    description="Secure SQL query execution in isolated containers",
    version="1.0.0"
)

# CORS middleware
app.add_middleware(
    CORSMiddleware,
    allow_origins=["*"],
    allow_credentials=True,
    allow_methods=["*"],
    allow_headers=["*"],
)

# Initialize SQL executor
executor = SQLExecutor()


@app.on_event("startup")
async def startup_event():
    """Initialize the SQL engine"""
    print("🔧 Starting SQL Execution Engine")
    print(f"⏱️  Query Timeout: {settings.query_timeout}s")
    print(f"💾 Memory Limit: {settings.memory_limit}")
    print(f"🐳 Max Containers: {settings.max_containers}")


@app.on_event("shutdown")
async def shutdown_event():
    """Cleanup resources"""
    print("🧹 Shutting down SQL Execution Engine")
    await executor.cleanup()


@app.get("/")
async def root():
    """Root endpoint"""
    return {
        "message": "SQL Execution Engine",
        "version": "1.0.0",
        "status": "running"
    }


@app.get("/health")
async def health_check():
    """Health check endpoint for Kubernetes probes"""
    try:
        # Check if Docker is available
        import docker
        client = docker.from_env()
        client.ping()
        return {
            "status": "healthy",
            "docker": "connected",
            "active_containers": len(executor.active_containers)
        }
    except Exception as e:
        raise HTTPException(status_code=503, detail=f"Unhealthy: {str(e)}")


@app.post("/api/execute")
async def execute_sql(request: dict):
    """
    Execute SQL query in isolated container

    Expected request body:
    {
        "question_id": "uuid",
        "user_sql": "SELECT * FROM employees",
        "schema_data": { ... },
        "expected_output": { ... }
    }

    Returns:
    {
        "columns": ["id", "name", "salary"],
        "rows": [{"id": 1, "name": "Alice", "salary": 90000}],
        "is_correct": true,
        "execution_time_ms": 125,
        "error": null
    }
    """
    question_id = request.get("question_id")
    user_sql = request.get("user_sql")
    schema_data = request.get("schema_data")
    expected_output = request.get("expected_output")

    if not user_sql:
        raise HTTPException(status_code=400, detail="user_sql is required")

    if not schema_data:
        raise HTTPException(status_code=400, detail="schema_data is required")

    try:
        # Execute SQL in isolated container
        result = await executor.execute(
            question_id=question_id,
            sql=user_sql,
            schema_data=schema_data
        )

        # Compare with expected output
        is_correct = executor.compare_results(result, expected_output)

        return {
            "columns": result.get("columns", []),
            "rows": result.get("rows", []),
            "is_correct": is_correct,
            "execution_time_ms": result.get("execution_time_ms", 0),
            "error": result.get("error")
        }

    except Exception as e:
        return {
            "columns": [],
            "rows": [],
            "is_correct": False,
            "execution_time_ms": 0,
            "error": str(e)
        }


@app.post("/api/validate")
async def validate_sql(request: dict):
    """
    Validate SQL without executing (syntax check only)

    Expected request body:
    {
        "sql": "SELECT * FROM employees"
    }
    """
    sql = request.get("sql")

    if not sql:
        raise HTTPException(status_code=400, detail="sql is required")

    # Basic SQL safety checks
    from src.core.security import is_safe_query

    safety_result = is_safe_query(sql)

    return {
        "is_safe": safety_result["is_safe"],
        "errors": safety_result.get("errors", []),
        "warnings": safety_result.get("warnings", [])
    }


if __name__ == "__main__":
    import uvicorn
    uvicorn.run(
        "main:app",
        host="0.0.0.0",
        port=8001,
        reload=True
    )
