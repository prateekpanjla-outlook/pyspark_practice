"""
Generate Jupyter notebooks for Questions 1-50
Creates two versions per question:
- Question_xx.ipynb (question only)
- Question_xx_solution.ipynb (question + solution)
"""
import json
import os

# Base notebook template
SPARK_SESSION_CELL = [
    "from pyspark.sql import SparkSession\n",
    "from pyspark.sql import Window\n",
    "from pyspark.sql.functions import *\n",
    "\n",
    "spark = SparkSession.builder \\\n",
    "    .master(\"local[*]\") \\\n",
    "    .appName(\"PostgresConnector\") \\\n",
    "    .config(\"spark.jars\",\"/usr/lib/spark/jars/postgresql-42.7.4.jar\")\\\n",
    "    .getOrCreate()\n",
    ""
]

TABLE_LOAD_CELL = [
    "employee_df = spark.read.jdbc(url=\"jdbc:postgresql://localhost:5432/employees\",table=\"employees.employee\",\n",
    "                         properties={\"user\": \"vagrant\", \"password\": \"vagrant\",\"driver\": \"org.postgresql.Driver\"})\n",
    "\n",
    "department_df = spark.read.jdbc(url=\"jdbc:postgresql://localhost:5432/employees\",table=\"employees.department\",\n",
    "                         properties={\"user\": \"vagrant\", \"password\": \"vagrant\",\"driver\": \"org.postgresql.Driver\"})\n",
    "\n",
    "department_employee_df = spark.read.jdbc(url=\"jdbc:postgresql://localhost:5432/employees\",table=\"employees.department_employee\",\n",
    "                         properties={\"user\": \"vagrant\", \"password\": \"vagrant\",\"driver\": \"org.postgresql.Driver\"})\n",
    ""
]

EMPTY_CELLS = [{"cell_type": "code", "execution_count": None, "id": f"cell{i}", "metadata": {}, "outputs": [], "source": []} for i in range(3, 9)]

def create_notebook_metadata():
    return {
        "kernelspec": {
            "display_name": "Python 3 (System)",
            "language": "python",
            "name": "python3"
        },
        "language_info": {
            "codemirror_mode": {"name": "ipython", "version": 3},
            "file_extension": ".py",
            "mimetype": "text/x-python",
            "name": "python",
            "nbconvert_exporter": "python",
            "pygments_lexer": "ipython3",
            "version": "3.10.12"
        }
    }

def create_notebook(question_num, title, concept, tables_needed, solution_code=None):
    """Create a notebook JSON structure"""
    cells = []

    # Cell 1: Spark session
    cells.append({
        "cell_type": "code",
        "execution_count": None,
        "id": "spark_session",
        "metadata": {},
        "outputs": [],
        "source": SPARK_SESSION_CELL.copy()
    })

    # Cell 2: Load tables and question/solution
    question_cell = [
        f"# Question {question_num}: {title}\n",
        f"# Concept: {concept}\n",
        ""
    ]

    # Add table loading code based on what's needed
    table_load = TABLE_LOAD_CELL.copy()
    if "salary" in tables_needed:
        table_load.append("salary_df = spark.read.jdbc(url=\"jdbc:postgresql://localhost:5432/employees\",table=\"employees.salary\",\n")
        table_load.append("                         properties={\"user\": \"vagrant\", \"password\": \"vagrant\",\"driver\": \"org.postgresql.Driver\"})\n")
        table_load.append("")
    if "title" in tables_needed:
        table_load.append("title_df = spark.read.jdbc(url=\"jdbc:postgresql://localhost:5432/employees\",table=\"employees.title\",\n")
        table_load.append("                         properties={\"user\": \"vagrant\", \"password\": \"vagrant\",\"driver\": \"org.postgresql.Driver\"})\n")
        table_load.append("")
    if "department_manager" in tables_needed:
        table_load.append("department_manager_df = spark.read.jdbc(url=\"jdbc:postgresql://localhost:5432/employees\",table=\"employees.department_manager\",\n")
        table_load.append("                         properties={\"user\": \"vagrant\", \"password\": \"vagrant\",\"driver\": \"org.postgresql.Driver\"})\n")
        table_load.append("")

    question_cell.extend(table_load)

    if solution_code:
        question_cell.append("# Solution:\n")
        question_cell.extend(solution_code)

    cells.append({
        "cell_type": "code",
        "execution_count": None,
        "id": "question_cell",
        "metadata": {},
        "outputs": [],
        "source": question_cell
    })

    # Add empty cells
    cells.extend(EMPTY_CELLS)

    return {
        "cells": cells,
        "metadata": create_notebook_metadata(),
        "nbformat": 4,
        "nbformat_minor": 5
    }

# Define all questions 1-50
QUESTIONS = {
    1: {
        "title": "List all current employees with their department names.",
        "concept": "Inner Join, Current Date Filter",
        "tables": [],
        "solution": [
            "cond = [employee_df.id == department_employee_df.employee_id, \n",
            "        department_employee_df.to_date == '9999-01-01', \n",
            "        department_employee_df.department_id == department_df.id]\n",
            "\n",
            "employee_df.join(department_employee_df, \"id\") \\\n",
            "           .join(department_df, cond) \\\n",
            "           .select(employee_df.id, employee_df.first_name, \n",
            "                   employee_df.last_name, department_df.dept_name) \\\n",
            "           .show()\n"
        ]
    },
    2: {
        "title": "Find the current head of each department.",
        "concept": "Inner Join, Manager Table",
        "tables": ["department_manager"],
        "solution": [
            "# Create aliases for the tables\n",
            "dm = department_manager_df.alias(\"dm\")\n",
            "e = employee_df.alias(\"e\")\n",
            "d = department_df.alias(\"d\")\n",
            "\n",
            "dm.filter(col(\"to_date\") == '9999-01-01') \\\n",
            "  .join(e, dm.employee_id == e.id) \\\n",
            "  .join(d, dm.department_id == d.id) \\\n",
            "  .select(d.dept_name, e.first_name, e.last_name) \\\n",
            "  .show()\n"
        ]
    },
    3: {
        "title": "List employees who are currently 'Senior Engineers'.",
        "concept": "Multi-table Join",
        "tables": ["title"],
        "solution": [
            "employee_df.join(title_df, employee_df.id == title_df.employee_id) \\\n",
            "           .filter((col(\"title\") == \"Senior Engineer\") & (col(\"to_date\") == '9999-01-01')) \\\n",
            "           .select(employee_df.id, employee_df.first_name) \\\n",
            "           .show()\n"
        ]
    },
    4: {
        "title": "Calculate the average salary per department.",
        "concept": "Aggregation",
        "tables": ["salary"],
        "solution": [
            "cond = [salary_df.employee_id == department_employee_df.employee_id, \n",
            "        salary_df.to_date == '9999-01-01', \n",
            "        department_employee_df.to_date == '9999-01-01',\n",
            "        department_employee_df.department_id == department_df.id]\n",
            "\n",
            "joined = salary_df.join(department_employee_df, cond).join(department_df, \"department_id\")\n",
            "joined.groupBy(\"dept_name\").avg(\"amount\").show()\n"
        ]
    },
    5: {
        "title": "Find departments with more than 10,000 current employees.",
        "concept": "Having Clause",
        "tables": [],
        "solution": [
            "department_employee_df.filter(col(\"to_date\") == '9999-01-01') \\\n",
            "                      .join(department_df, department_employee_df.department_id == department_df.id) \\\n",
            "                      .groupBy(\"dept_name\") \\\n",
            "                      .agg(count(\"*\").alias(\"cnt\")) \\\n",
            "                      .filter(col(\"cnt\") > 10000) \\\n",
            "                      .show()\n"
        ]
    },
    6: {
        "title": "Find employees hired in the year 1995.",
        "concept": "Date Function",
        "tables": [],
        "solution": [
            "employee_df.filter(year(\"hire_date\") == 1995).show()\n"
        ]
    },
    7: {
        "title": "Count how many distinct job titles exist.",
        "concept": "Distinct Count",
        "tables": ["title"],
        "solution": [
            "title_df.select(countDistinct(\"title\")).show()\n"
        ]
    },
    8: {
        "title": "List employees who have never held a manager title.",
        "concept": "Left Join (Anti-Join)",
        "tables": ["title"],
        "solution": [
            "mgrs = title_df.filter(col(\"title\").contains(\"Manager\")).select(\"employee_id\").distinct()\n",
            "employee_df.join(mgrs, employee_df.id == mgrs.employee_id, \"left_anti\") \\\n",
            "           .select(\"first_name\", \"last_name\") \\\n",
            "           .show()\n"
        ]
    },
    9: {
        "title": "Show the salary history for employee ID 10001.",
        "concept": "Simple Filter",
        "tables": ["salary"],
        "solution": [
            "salary_df.filter(col(\"employee_id\") == 10001).orderBy(\"from_date\").show()\n"
        ]
    },
    10: {
        "title": "Find the maximum salary ever paid.",
        "concept": "Max",
        "tables": ["salary"],
        "solution": [
            "salary_df.agg(max(\"amount\")).show()\n"
        ]
    },
    11: {
        "title": "Find the top 3 earners in each department.",
        "concept": "row_number",
        "tables": ["salary"],
        "solution": [
            "cond = [salary_df.employee_id == department_employee_df.employee_id, \n",
            "        salary_df.to_date == '9999-01-01', \n",
            "        department_employee_df.to_date == '9999-01-01',\n",
            "        department_employee_df.department_id == department_df.id]\n",
            "\n",
            "joined = salary_df.join(department_employee_df, cond) \\\n",
            "                  .join(department_df, \"department_id\") \\\n",
            "                  .join(employee_df, salary_df.employee_id == employee_df.id)\n",
            "\n",
            "window_spec = Window.partitionBy(\"dept_name\").orderBy(col(\"amount\").desc())\n",
            "joined.withColumn(\"rn\", row_number().over(window_spec)) \\\n",
            "      .filter(col(\"rn\") <= 3) \\\n",
            "      .show()\n"
        ]
    },
    12: {
        "title": "Rank employees by salary within their title.",
        "concept": "dense_rank",
        "tables": ["salary", "title"],
        "solution": [
            "cond = [employee_df.id == salary_df.employee_id, salary_df.to_date == '9999-01-01',\n",
            "        employee_df.id == title_df.employee_id, title_df.to_date == '9999-01-01']\n",
            "joined = employee_df.join(salary_df, employee_df.id == salary_df.employee_id) \\\n",
            "                   .join(title_df, employee_df.id == title_df.employee_id)\n",
            "\n",
            "window_spec = Window.partitionBy(\"title\").orderBy(col(\"amount\").desc())\n",
            "joined.withColumn(\"rnk\", dense_rank().over(window_spec)).show()\n"
        ]
    },
    13: {
        "title": "Compare current salary with the previous salary (Raise amount).",
        "concept": "lag",
        "tables": ["salary"],
        "solution": [
            "window_spec = Window.partitionBy(\"employee_id\").orderBy(\"from_date\")\n",
            "salary_df.withColumn(\"prev_amount\", lag(\"amount\").over(window_spec)) \\\n",
            "        .withColumn(\"raise\", col(\"amount\") - col(\"prev_amount\")) \\\n",
            "        .show()\n"
        ]
    },
    14: {
        "title": "Find the date of the next job title change.",
        "concept": "lead",
        "tables": ["title"],
        "solution": [
            "window_spec = Window.partitionBy(\"employee_id\").orderBy(\"from_date\")\n",
            "title_df.withColumn(\"next_change_date\", lead(\"from_date\").over(window_spec)).show()\n"
        ]
    },
    15: {
        "title": "Calculate salary quartiles (NTILE).",
        "concept": "ntile",
        "tables": ["salary"],
        "solution": [
            "current_salary = salary_df.filter(col(\"to_date\") == '9999-01-01')\n",
            "current_salary.withColumn(\"quartile\", ntile(4).over(Window.orderBy(col(\"amount\").desc()))).show()\n"
        ]
    },
    16: {
        "title": "Find the percentage rank of a salary within the company.",
        "concept": "percent_rank",
        "tables": ["salary"],
        "solution": [
            "current_salary = salary_df.filter(col(\"to_date\") == '9999-01-01')\n",
            "current_salary.withColumn(\"pct_rank\", percent_rank().over(Window.orderBy(\"amount\"))).show()\n"
        ]
    },
    17: {
        "title": "Count the number of employees hired in the same year as each employee.",
        "concept": "count Window",
        "tables": [],
        "solution": [
            "window_spec = Window.partitionBy(year(\"hire_date\"))\n",
            "employee_df.withColumn(\"peers\", count(\"*\").over(window_spec)).show()\n"
        ]
    },
    18: {
        "title": "Find the first hire date for every department.",
        "concept": "first_value",
        "tables": [],
        "solution": [
            "cond = [employee_df.id == department_employee_df.employee_id, \n",
            "        department_employee_df.department_id == department_df.id]\n",
            "joined = employee_df.join(department_employee_df, employee_df.id == department_employee_df.employee_id) \\\n",
            "                   .join(department_df, department_employee_df.department_id == department_df.id)\n",
            "\n",
            "window_spec = Window.partitionBy(\"department_id\").orderBy(\"hire_date\")\n",
            "joined.withColumn(\"first_hire\", first(\"hire_date\").over(window_spec)) \\\n",
            "      .select(\"dept_name\", \"first_hire\") \\\n",
            "      .distinct() \\\n",
            "      .show()\n"
        ]
    },
    19: {
        "title": "Find employees with duplicate titles.",
        "concept": "Logic with Window",
        "tables": ["title"],
        "solution": [
            "window_spec = Window.partitionBy(\"employee_id\", \"title\")\n",
            "title_df.withColumn(\"cnt\", count(\"*\").over(window_spec)) \\\n",
            "       .filter(col(\"cnt\") > 1) \\\n",
            "       .select(\"employee_id\") \\\n",
            "       .distinct() \\\n",
            "       .show()\n"
        ]
    },
    20: {
        "title": "Median salary approximation.",
        "concept": "Logic using percent_rank or row_number",
        "tables": ["salary"],
        "solution": [
            "salary_df.filter(col(\"to_date\") == '9999-01-01') \\\n",
            "        .agg(expr(\"percentile_approx(amount, 0.5)\").alias(\"median\")) \\\n",
            "        .show()\n"
        ]
    },
    21: {
        "title": "3-Month Moving Average of Salary changes (Time Series).",
        "concept": "rowsBetween",
        "tables": ["salary"],
        "solution": [
            "window_spec = Window.partitionBy(\"employee_id\") \\\n",
            "                   .orderBy(\"from_date\") \\\n",
            "                   .rowsBetween(-2, 0)\n",
            "\n",
            "salary_df.filter(col(\"employee_id\") == 10001) \\\n",
            "        .withColumn(\"moving_avg\", avg(\"amount\").over(window_spec)) \\\n",
            "        .show()\n"
        ]
    },
    22: {
        "title": "Running Total of salaries paid globally by date.",
        "concept": "rangeBetween (Unbounded)",
        "tables": ["salary"],
        "solution": [
            "daily_cost = salary_df.groupBy(\"from_date\").sum(\"amount\").withColumnRenamed(\"sum(amount)\", \"daily_cost\")\n",
            "window_spec = Window.orderBy(\"from_date\").rangeBetween(Window.unboundedPreceding, Window.currentRow)\n",
            "\n",
            "daily_cost.withColumn(\"running_total\", sum(\"daily_cost\").over(window_spec)).show()\n"
        ]
    },
    23: {
        "title": "Sum of Current Salary + Previous 2 Salaries (Centered Window).",
        "concept": "rowsBetween (1 Preceding, 1 Following)",
        "tables": ["salary"],
        "solution": [
            "window_spec = Window.partitionBy(\"employee_id\").orderBy(\"from_date\").rowsBetween(-1, 1)\n",
            "salary_df.filter(col(\"employee_id\") == 10001) \\\n",
            "        .withColumn(\"sum_neighbors\", sum(\"amount\").over(window_spec)) \\\n",
            "        .show()\n"
        ]
    },
    24: {
        "title": "Department Average Salary excluding the current row.",
        "concept": "Window Filter Frame",
        "tables": ["salary"],
        "solution": [
            "cond = [salary_df.employee_id == department_employee_df.employee_id, salary_df.to_date == '9999-01-01',\n",
            "        department_employee_df.department_id == department_df.id, department_employee_df.to_date == '9999-01-01']\n",
            "\n",
            "joined = salary_df.join(department_employee_df, cond).join(department_df, \"department_id\")\n",
            "window_spec = Window.partitionBy(\"department_id\").orderBy(\"amount\").rowsBetween(Window.unboundedPreceding, -1)\n",
            "\n",
            "joined.withColumn(\"avg_lower\", avg(\"amount\").over(window_spec)).show()\n"
        ]
    },
    25: {
        "title": "Rolling 12-month total hires.",
        "concept": "Time-based Window (requires Date math)",
        "tables": [],
        "solution": [
            "# Using rangeBetween with timestamps\n",
            "emp_ts = employee_df.withColumn(\"ts\", to_timestamp(col(\"hire_date\")))\n",
            "window_spec = Window.orderBy(\"ts\").rangeBetween(-31536000, 0) # Approx 1 year in seconds\n",
            "\n",
            "emp_ts.withColumn(\"rolling_hires\", count(\"*\").over(window_spec)).show()\n"
        ]
    },
    26: {
        "title": "Count of Employees by Gender in each Department (Pivot).",
        "concept": "pivot",
        "tables": [],
        "solution": [
            "cond = [employee_df.id == department_employee_df.employee_id, department_employee_df.to_date == '9999-01-01',\n",
            "        department_employee_df.department_id == department_df.id]\n",
            "\n",
            "joined = employee_df.join(department_employee_df, employee_df.id == department_employee_df.employee_id) \\\n",
            "                   .join(department_df, department_employee_df.department_id == department_df.id)\n",
            "joined.groupBy(\"dept_name\").pivot(\"gender\", [\"M\", \"F\"]).count().show()\n"
        ]
    },
    27: {
        "title": "Average Salary by Department and Title (Pivot).",
        "concept": "pivot with Aggregation",
        "tables": ["salary", "title"],
        "solution": [
            "cond = [salary_df.employee_id == title_df.employee_id, title_df.to_date == '9999-01-01',\n",
            "        salary_df.employee_id == department_employee_df.employee_id, department_employee_df.to_date == '9999-01-01',\n",
            "        department_employee_df.department_id == department_df.id,\n",
            "        salary_df.to_date == '9999-01-01']\n",
            "\n",
            "joined = salary_df.join(title_df, salary_df.employee_id == title_df.employee_id) \\\n",
            "                 .join(department_employee_df, salary_df.employee_id == department_employee_df.employee_id) \\\n",
            "                 .join(department_df, \"department_id\")\n",
            "joined.groupBy(\"dept_name\") \\\n",
            "      .pivot(\"title\", [\"Staff\", \"Senior Engineer\", \"Manager\"]) \\\n",
            "      .avg(\"amount\") \\\n",
            "      .show()\n"
        ]
    },
    28: {
        "title": "List all employees in a department as a comma-separated string.",
        "concept": "collect_list / group_concat",
        "tables": [],
        "solution": [
            "cond = [department_df.id == department_employee_df.department_id, department_employee_df.to_date == '9999-01-01',\n",
            "        department_employee_df.employee_id == employee_df.id]\n",
            "\n",
            "joined = department_df.join(department_employee_df, cond).join(employee_df, department_employee_df.employee_id == employee_df.id)\n",
            "result = joined.groupBy(\"dept_name\") \\\n",
            "               .agg(concat_ws(\", \", collect_list(\"first_name\")).alias(\"employees\"))\n",
            "result.show(truncate=False)\n"
        ]
    },
    29: {
        "title": "Histogram of Salary Ranges.",
        "concept": "Bucketing",
        "tables": ["salary"],
        "solution": [
            "current_salary = salary_df.filter(col(\"to_date\") == '9999-01-01')\n",
            "current_salary.withColumn(\"range\", \n",
            "                   when(col(\"amount\") < 50000, \"Low\")\n",
            "                   .when(col(\"amount\") < 80000, \"Medium\")\n",
            "                   .otherwise(\"High\")) \\\n",
            "               .groupBy(\"range\").count() \\\n",
            "               .show()\n"
        ]
    },
    30: {
        "title": "Most frequent title per department (Mode).",
        "concept": "Window count aliasing",
        "tables": ["title"],
        "solution": [
            "cond = [department_employee_df.department_id == department_df.id, department_employee_df.to_date == '9999-01-01',\n",
            "        department_employee_df.employee_id == title_df.employee_id, title_df.to_date == '9999-01-01']\n",
            "\n",
            "counts = department_employee_df.join(department_df, cond) \\\n",
            "                              .join(title_df, department_employee_df.employee_id == title_df.employee_id) \\\n",
            "                              .groupBy(department_df.dept_name, title_df.title).count()\n",
            "\n",
            "window_spec = Window.partitionBy(\"dept_name\").orderBy(col(\"count\").desc())\n",
            "counts.withColumn(\"rn\", row_number().over(window_spec)) \\\n",
            "      .filter(col(\"rn\") == 1) \\\n",
            "      .show()\n"
        ]
    },
    31: {
        "title": "Employees who earn more than their current manager.",
        "concept": "Complex Join",
        "tables": ["salary", "department_manager"],
        "solution": [
            "mgr_cond = [department_manager_df.employee_id == salary_df.employee_id, salary_df.to_date == '9999-01-01',\n",
            "            department_manager_df.to_date == '9999-01-01']\n",
            "managers = department_manager_df.join(salary_df, mgr_cond) \\\n",
            "                               .select(department_manager_df.department_id, salary_df.amount.alias(\"mgr_amount\"))\n",
            "\n",
            "emp_cond = [employee_df.id == salary_df.employee_id, salary_df.to_date == '9999-01-01',\n",
            "            employee_df.id == department_employee_df.employee_id, department_employee_df.to_date == '9999-01-01',\n",
            "            department_employee_df.department_id == department_df.id]\n",
            "\n",
            "emps = employee_df.join(salary_df, employee_df.id == salary_df.employee_id) \\\n",
            "                  .join(department_employee_df, employee_df.id == department_employee_df.employee_id) \\\n",
            "                  .join(department_df, department_employee_df.department_id == department_df.id)\n",
            "\n",
            "emps.join(managers, emps.department_id == managers.department_id) \\\n",
            "    .filter(emps.amount > managers.mgr_amount) \\\n",
            "    .select(employee_df.id, department_df.dept_name, emps.amount) \\\n",
            "    .show()\n"
        ]
    },
    32: {
        "title": "Pairs of employees sharing the same birthday.",
        "concept": "Self Join with inequality",
        "tables": [],
        "solution": [
            "employee_df.alias(\"e1\").join(employee_df.alias(\"e2\"), \n",
            "    (col(\"e1.birth_date\") == col(\"e2.birth_date\")) & (col(\"e1.id\") < col(\"e2.id\"))) \\\n",
            "    .select(col(\"e1.id\"), col(\"e1.first_name\"), col(\"e2.id\"), col(\"e2.first_name\")) \\\n",
            "    .show()\n"
        ]
    },
    33: {
        "title": "Identify employees who held multiple titles.",
        "concept": "Self Join Count",
        "tables": ["title"],
        "solution": [
            "employee_df.join(title_df, employee_df.id == title_df.employee_id) \\\n",
            "           .groupBy(\"id\", \"first_name\") \\\n",
            "           .agg(count(\"title\").alias(\"cnt\")) \\\n",
            "           .filter(col(\"cnt\") > 1) \\\n",
            "           .show()\n"
        ]
    },
    34: {
        "title": "Employees who have changed departments.",
        "concept": "Self Join History",
        "tables": [],
        "solution": [
            "d1 = department_employee_df.alias(\"d1\")\n",
            "d2 = department_employee_df.alias(\"d2\")\n",
            "\n",
            "d1.join(d2, d1.employee_id == d2.employee_id) \\\n",
            "  .filter(col(\"d1.department_id\") != col(\"d2.department_id\")) \\\n",
            "  .select(col(\"d1.employee_id\")) \\\n",
            "  .distinct() \\\n",
            "  .join(employee_df, col(\"d1.employee_id\") == employee_df.id) \\\n",
            "  .show()\n"
        ]
    },
    35: {
        "title": "Employees whose salary decreased.",
        "concept": "Self Join with Date Logic",
        "tables": ["salary"],
        "solution": [
            "s1 = salary_df.alias(\"s1\")\n",
            "s2 = salary_df.alias(\"s2\")\n",
            "\n",
            "s1.join(s2, (col(\"s1.employee_id\") == col(\"s2.employee_id\")) & (col(\"s2.from_date\") == col(\"s1.to_date\"))) \\\n",
            "  .filter(col(\"s2.amount\") < col(\"s1.amount\")) \\\n",
            "  .select(col(\"s1.employee_id\")) \\\n",
            "  .distinct() \\\n",
            "  .show()\n"
        ]
    },
    36: {
        "title": "Find gaps in employment history (if any).",
        "concept": "Date Logic Self Join",
        "tables": [],
        "solution": [
            "d1 = department_employee_df.alias(\"d1\")\n",
            "d2 = department_employee_df.alias(\"d2\")\n",
            "\n",
            "d1.join(d2, col(\"d1.employee_id\") == col(\"d2.employee_id\")) \\\n",
            "  .filter(col(\"d1.to_date\") < col(\"d2.from_date\")) \\\n",
            "  .filter(col(\"d1.employee_id\") == 10001) \n",
            "  .show()\n"
        ]
    },
    37: {
        "title": "Tenure of employees in current department.",
        "concept": "Date Diff",
        "tables": [],
        "solution": [
            "cond = [employee_df.id == department_employee_df.employee_id, department_employee_df.to_date == '9999-01-01',\n",
            "        department_employee_df.department_id == department_df.id]\n",
            "\n",
            "joined = employee_df.join(department_employee_df, employee_df.id == department_employee_df.employee_id) \\\n",
            "                   .join(department_df, \"department_id\")\n",
            "joined.withColumn(\"days\", datediff(current_date(), col(\"from_date\"))) \\\n",
            "     .select(\"first_name\", \"dept_name\", \"days\") \\\n",
            "     .show()\n"
        ]
    },
    38: {
        "title": "Salary increase percentage on the last raise.",
        "concept": "Aggregation over dates",
        "tables": ["salary"],
        "solution": [
            "window_spec = Window.partitionBy(\"employee_id\").orderBy(col(\"from_date\").desc())\n",
            "ranked = salary_df.withColumn(\"rn\", row_number().over(window_spec))\n",
            "\n",
            "cur = ranked.filter(col(\"rn\") == 1).withColumnRenamed(\"amount\", \"cur_amount\")\n",
            "prev = ranked.filter(col(\"rn\") == 2).withColumnRenamed(\"amount\", \"prev_amount\")\n",
            "\n",
            "cur.join(prev, \"employee_id\") \\\n",
            "    .withColumn(\"pct\", ((col(\"cur_amount\") - col(\"prev_amount\")) / col(\"prev_amount\")) * 100) \\\n",
            "    .show()\n"
        ]
    },
    39: {
        "title": "Departments with average salary above company average.",
        "concept": "Scalar Subquery equivalent (Cross Join)",
        "tables": ["salary"],
        "solution": [
            "global_avg = salary_df.filter(col(\"to_date\") == '9999-01-01').agg(avg(\"amount\")).collect()[0][0]\n",
            "\n",
            "cond = [salary_df.employee_id == department_employee_df.employee_id, department_employee_df.to_date == '9999-01-01',\n",
            "        salary_df.to_date == '9999-01-01', department_employee_df.department_id == department_df.id]\n",
            "\n",
            "joined = salary_df.join(department_employee_df, salary_df.employee_id == department_employee_df.employee_id) \\\n",
            "                  .join(department_df, \"department_id\")\n",
            "joined.groupBy(\"dept_name\") \\\n",
            "      .agg(avg(\"amount\").alias(\"avg_amount\")) \\\n",
            "      .filter(col(\"avg_amount\") > global_avg) \\\n",
            "      .show()\n"
        ]
    },
    40: {
        "title": "Find the longest serving manager of each department.",
        "concept": "Duration Calculation & Max",
        "tables": ["department_manager"],
        "solution": [
            "joined = department_manager_df.join(employee_df, department_manager_df.employee_id == employee_df.id)\n",
            "joined = joined.withColumn(\"days\", datediff(col(\"to_date\"), col(\"from_date\")))\n",
            "\n",
            "window_spec = Window.partitionBy(\"department_id\").orderBy(col(\"days\").desc())\n",
            "joined.withColumn(\"rn\", row_number().over(window_spec)) \\\n",
            "      .filter(col(\"rn\") == 1) \\\n",
            "      .show()\n"
        ]
    },
    41: {
        "title": "Generate a report of total hires per month, filling months with 0 hires.",
        "concept": "Time Series Generation (SQL Recursive vs PySpark Range Join)",
        "tables": [],
        "solution": [
            "import pandas as pd\n",
            "\n",
            "# 1. Generate Month Series\n",
            "min_date = employee_df.agg(min(\"hire_date\")).collect()[0][0].replace(day=1)\n",
            "max_date = employee_df.agg(max(\"hire_date\")).collect()[0][0].replace(day=1)\n",
            "date_range = pd.date_range(start=min_date, end=max_date, freq='MS')\n",
            "months_df = spark.createDataFrame(date_range.to_frame(index=False), DateType()).toDF(\"month_start\")\n",
            "\n",
            "# 2. Prepare hires data\n",
            "hires_df = employee_df.withColumn(\"month_start\", trunc(\"hire_date\", \"month\")) \\\n",
            "                       .groupBy(\"month_start\") \\\n",
            "                       .count()\n",
            "\n",
            "# 3. Left Join to fill zeros\n",
            "months_df.join(hires_df, \"month_start\", \"left\") \\\n",
            "         .na.fill(0, subset=[\"count\"]) \\\n",
            "         .orderBy(\"month_start\") \\\n",
            "         .show()\n"
        ]
    },
    42: {
        "title": "Calculate cumulative headcount of the company over time.",
        "concept": "Running Aggregation on Time Series",
        "tables": [],
        "solution": [
            "# Create daily sequence\n",
            "start = employee_df.agg(min(\"hire_date\")).collect()[0][0]\n",
            "end = employee_df.agg(max(\"hire_date\")).collect()[0][0]\n",
            "num_days = (end - start).days\n",
            "days_df = spark.range(num_days).select(expr(\"date_add(date('{}'), cast(id as int)) as dt\".format(start)))\n",
            "\n",
            "hires_df = employee_df.groupBy(\"hire_date\").count().withColumnRenamed(\"hire_date\", \"dt\")\n",
            "joined = days_df.join(hires_df, \"dt\", \"left\").na.fill(0)\n",
            "\n",
            "joined.withColumn(\"running_total\", sum(\"count\").over(Window.orderBy(\"dt\").rowsBetween(Window.unboundedPreceding, Window.currentRow))) \\\n",
            "      .show()\n"
        ]
    },
    43: {
        "title": "Monthly Active Users (MAU) - Count of employees active in each month.",
        "concept": "Expanding date ranges into monthly records",
        "tables": [],
        "solution": [
            "# Generate Month Series (similar to Q41)\n",
            "min_date = department_employee_df.agg(min(\"from_date\")).collect()[0][0].replace(day=1)\n",
            "max_date = department_employee_df.agg(max(\"to_date\")).collect()[0][0].replace(day=1)\n",
            "date_list = pd.date_range(min_date, max_date, freq='MS')\n",
            "months_df = spark.createDataFrame(date_list.to_frame(index=False), DateType()).toDF(\"month_start\")\n",
            "\n",
            "# Range Join: Active if month_start is between from_date and to_date\n",
            "active_counts = months_df.crossJoin(department_employee_df) \\\n",
            "    .filter((col(\"month_start\") >= trunc(col(\"from_date\"), \"month\")) & \n",
            "            (col(\"month_start\") <= trunc(col(\"to_date\"), \"month\"))) \\\n",
            "    .groupBy(\"month_start\") \\\n",
            "    .agg(countDistinct(\"employee_id\").alias(\"active_employees\"))\n",
            "\n",
            "active_counts.show()\n"
        ]
    },
    44: {
        "title": "Find months where salary expenses spiked compared to previous month.",
        "concept": "Time Series Comparison",
        "tables": ["salary"],
        "solution": [
            "monthly = salary_df.groupBy(trunc(\"from_date\", \"month\").alias(\"mth\")).sum(\"amount\").withColumnRenamed(\"sum(amount)\", \"cost\")\n",
            "window_spec = Window.orderBy(\"mth\")\n",
            "monthly.withColumn(\"prev_cost\", lag(\"cost\").over(window_spec)) \\\n",
            "       .withColumn(\"growth\", (col(\"cost\") - col(\"prev_cost\")) / col(\"prev_cost\")) \\\n",
            "       .show()\n"
        ]
    },
    45: {
        "title": "Employee Tenure distribution (Buckets).",
        "concept": "Window based on time elapsed",
        "tables": [],
        "solution": [
            "employee_df.withColumn(\"days\", datediff(current_date(), \"hire_date\")) \\\n",
            "            .withColumn(\"tenure\",\n",
            "                when(col(\"days\") < 365, \"Junior\")\n",
            "                .when(col(\"days\") < 1825, \"Mid\")\n",
            "                .otherwise(\"Senior\")) \\\n",
            "            .groupBy(\"tenure\").count() \\\n",
            "            .show()\n"
        ]
    },
    46: {
        "title": "Find consecutive years where an employee got a raise.",
        "concept": "Complex Logic (Self Join)",
        "tables": ["salary"],
        "solution": [
            "s1 = salary_df.withColumn(\"y\", year(\"from_date\"))\n",
            "s2 = salary_df.withColumn(\"y\", year(\"from_date\"))\n",
            "\n",
            "s1.alias(\"a\").join(s2.alias(\"b\"), \n",
            "    (col(\"a.employee_id\") == col(\"b.employee_id\")) & \n",
            "    (col(\"a.y\") + 1 == col(\"b.y\")) & \n",
            "    (col(\"b.amount\") > col(\"a.amount\"))) \\\n",
            "    .select(col(\"a.employee_id\")).distinct().show()\n"
        ]
    },
    47: {
        "title": "Determine the hire date cohort (Year) and retention count.",
        "concept": "Grouping + Window",
        "tables": [],
        "solution": [
            "window_spec = Window.partitionBy(year(\"hire_date\")).orderBy(\"hire_date\")\n",
            "employee_df.withColumn(\"cohort\", year(\"hire_date\")) \\\n",
            "            .withColumn(\"running_retention\", count(\"*\").over(window_spec)) \\\n",
            "            .show()\n"
        ]
    },
    48: {
        "title": "Department headcount trend (Simple Time Series).",
        "concept": "Generate dates and count active dept_emp records.",
        "tables": [],
        "solution": [
            "# Just current headcount\n",
            "department_employee_df.filter(col(\"to_date\") == '9999-01-01') \\\n",
            "                      .groupBy(\"department_id\") \\\n",
            "                      .count() \\\n",
            "                      .join(department_df, department_employee_df.department_id == department_df.id) \\\n",
            "                      .show()\n"
        ]
    },
    49: {
        "title": "Identify employees whose current salary is not the highest they've ever earned.",
        "concept": "Window vs Current Value",
        "tables": ["salary"],
        "solution": [
            "max_ever = salary_df.groupBy(\"employee_id\").agg(max(\"amount\").alias(\"max_ever\"))\n",
            "current = salary_df.filter(col(\"to_date\") == '9999-01-01').withColumnRenamed(\"amount\", \"current\")\n",
            "\n",
            "current.join(max_ever, \"employee_id\") \\\n",
            "       .filter(col(\"current\") < col(\"max_ever\")) \\\n",
            "       .join(employee_df, current.employee_id == employee_df.id) \\\n",
            "       .select(\"first_name\", \"last_name\", \"current\", \"max_ever\") \\\n",
            "       .show()\n"
        ]
    },
    50: {
        "title": "Find the department with the highest salary variance.",
        "concept": "Statistical Aggregation (VarPop/VarSamp)",
        "tables": ["salary"],
        "solution": [
            "from pyspark.sql.functions import var_samp\n",
            "\n",
            "cond = [salary_df.employee_id == department_employee_df.employee_id, department_employee_df.to_date == '9999-01-01',\n",
            "        salary_df.to_date == '9999-01-01', department_employee_df.department_id == department_df.id]\n",
            "\n",
            "joined = salary_df.join(department_employee_df, cond).join(department_df, \"department_id\")\n",
            "joined.groupBy(\"dept_name\") \\\n",
            "      .agg(var_samp(\"amount\").alias(\"variance\")) \\\n",
            "      .orderBy(col(\"variance\").desc()) \\\n",
            "      .limit(1) \\\n",
            "      .show()\n"
        ]
    }
}

def main():
    output_dir = r"C:\Users\prateek\Desktop\spark\pyspark_practice\notebooks_q1_50"
    os.makedirs(output_dir, exist_ok=True)

    for q_num, q_data in QUESTIONS.items():
        # Create question-only notebook
        notebook_question = create_notebook(
            q_num, q_data["title"], q_data["concept"], q_data["tables"], solution_code=None
        )
        question_path = os.path.join(output_dir, f"Question_{q_num}.ipynb")
        with open(question_path, "w", encoding="utf-8") as f:
            json.dump(notebook_question, f, indent=1, ensure_ascii=False)

        # Create solution notebook
        notebook_solution = create_notebook(
            q_num, q_data["title"], q_data["concept"], q_data["tables"], solution_code=q_data["solution"]
        )
        solution_path = os.path.join(output_dir, f"Question_{q_num}_solution.ipynb")
        with open(solution_path, "w", encoding="utf-8") as f:
            json.dump(notebook_solution, f, indent=1, ensure_ascii=False)

        print(f"Created: Question_{q_num}.ipynb and Question_{q_num}_solution.ipynb")

    print(f"\nAll notebooks created in: {output_dir}")
    print(f"Total: {len(QUESTIONS)} questions x 2 versions = {len(QUESTIONS) * 2} notebooks")

if __name__ == "__main__":
    main()
