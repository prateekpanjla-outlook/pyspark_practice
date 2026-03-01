"""
Generate Jupyter notebooks for Questions 53-100
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

# Define all questions 53-100
QUESTIONS = {
    53: {
        "title": "Create a unified list of all current Employees and all current Managers.",
        "concept": "UNION / Union All",
        "tables": ["department_manager"],
        "solution": [
            "emp = employee_df.join(department_employee_df, employee_df.id == department_employee_df.employee_id) \\\n",
            "                 .filter(col(\"to_date\") == '9999-01-01') \\\n",
            "                 .select(employee_df.id, employee_df.first_name, lit(\"Employee\").alias(\"type\"))\n",
            "\n",
            "mgr = employee_df.join(department_manager_df, employee_df.id == department_manager_df.employee_id) \\\n",
            "                 .filter(col(\"to_date\") == '9999-01-01') \\\n",
            "                 .select(employee_df.id, employee_df.first_name, lit(\"Manager\").alias(\"type\"))\n",
            "\n",
            "emp.union(mgr).show()\n"
        ]
    },
    54: {
        "title": "Find employees who were hired on a weekend (Saturday or Sunday).",
        "concept": "Date Function (Day of Week)",
        "tables": [],
        "solution": [
            "# Spark: 1=Sunday, 2=Monday ... 7=Saturday\n",
            "employee_df.filter(dayofweek(col(\"hire_date\")).isin(1, 7)).show()\n"
        ]
    },
    55: {
        "title": "Find employees born in a Leap Year.",
        "concept": "Date Math Modulo",
        "tables": [],
        "solution": [
            "employee_df.filter((year(col(\"birth_date\")) % 4 == 0) & \\\n",
            "                   ((year(col(\"birth_date\")) % 100 != 0) | (year(col(\"birth_date\")) % 400 == 0))) \\\n",
            "           .show()\n"
        ]
    },
    56: {
        "title": "Employees whose last name starts with the same letter as their first name.",
        "concept": "String Manipulation",
        "tables": [],
        "solution": [
            "employee_df.filter(substring(col(\"first_name\"), 1, 1) == substring(col(\"last_name\"), 1, 1)).show()\n"
        ]
    },
    57: {
        "title": "Find employees whose first name contains 'z' or 'Z'.",
        "concept": "Like / Contains",
        "tables": [],
        "solution": [
            "employee_df.filter(lower(col(\"first_name\")).contains(\"z\")).show()\n"
        ]
    },
    58: {
        "title": "Find employees with a 'Senior' title that are younger than 30 years old (at hire).",
        "concept": "Date Diff + Join",
        "tables": ["title"],
        "solution": [
            "emp_with_title = employee_df.join(title_df, employee_df.id == title_df.employee_id)\n",
            "emp_with_title.filter(col(\"title\").contains(\"Senior\")) \\\n",
            "              .filter((datediff(col(\"from_date\"), col(\"birth_date\")) / 365) < 30) \\\n",
            "              .show()\n"
        ]
    },
    59: {
        "title": "Calculate the length of each employee's full name (First + Last + space).",
        "concept": "String Length",
        "tables": [],
        "solution": [
            "employee_df.withColumn(\"full_name\", concat(col(\"first_name\"), lit(\" \"), col(\"last_name\"))) \\\n",
            "           .withColumn(\"length\", length(col(\"full_name\"))) \\\n",
            "           .show()\n"
        ]
    },
    60: {
        "title": "Identify employees who have 'Anne' anywhere in their name.",
        "concept": "Concat + Like",
        "tables": [],
        "solution": [
            "employee_df.filter(concat(col(\"first_name\"), lit(\" \"), col(\"last_name\")).contains(\"Anne\")).show()\n"
        ]
    },
    61: {
        "title": "Calculate Z-Scores for current salaries (Statistical outlier detection).",
        "concept": "Window Aggregation + StdDev",
        "tables": ["salary"],
        "solution": [
            "window_spec = Window.rangeBetween(Window.unboundedPreceding, Window.unboundedFollowing)\n",
            "current_salary = salary_df.filter(col(\"to_date\") == '9999-01-01')\n",
            "\n",
            "# Calculate mean and std\n",
            "stats = current_salary.agg(avg(\"amount\").alias(\"mean\"), stddev(\"amount\").alias(\"std\")).collect()\n",
            "mean_val = stats[0][\"mean\"]\n",
            "std_val = stats[0][\"std\"]\n",
            "\n",
            "current_salary.withColumn(\"z_score\", (col(\"amount\") - lit(mean_val)) / lit(std_val)).show()\n"
        ]
    },
    62: {
        "title": "Find departments with the widest salary range (Max - Min).",
        "concept": "Aggregation Math",
        "tables": ["salary"],
        "solution": [
            "cond = [salary_df.employee_id == department_employee_df.employee_id, \n",
            "        salary_df.to_date == '9999-01-01', department_employee_df.to_date == '9999-01-01',\n",
            "        department_employee_df.department_id == department_df.id]\n",
            "\n",
            "joined = salary_df.join(department_employee_df, cond).join(department_df, \"department_id\")\n",
            "joined.groupBy(\"dept_name\") \\\n",
            "      .agg((max(\"amount\") - min(\"amount\")).alias(\"salary_range\")) \\\n",
            "      .orderBy(col(\"salary_range\").desc()) \\\n",
            "      .show()\n"
        ]
    },
    63: {
        "title": "Find departments where the average female salary is higher than average male salary.",
        "concept": "Group By Case When + Having",
        "tables": ["salary"],
        "solution": [
            "cond = [salary_df.employee_id == department_employee_df.employee_id, salary_df.to_date == '9999-01-01',\n",
            "        department_employee_df.to_date == '9999-01-01', salary_df.employee_id == employee_df.id,\n",
            "        department_employee_df.department_id == department_df.id]\n",
            "\n",
            "stats = salary_df.join(department_employee_df, cond) \\\n",
            "                  .join(employee_df, \"employee_id\") \\\n",
            "                  .join(department_df, \"department_id\") \\\n",
            "                  .groupBy(\"dept_name\", \"gender\") \\\n",
            "                  .agg(avg(\"amount\").alias(\"avg_sal\"))\n",
            "\n",
            "# Pivot to compare\n",
            "stats.groupBy(\"dept_name\") \\\n",
            "     .pivot(\"gender\", [\"M\", \"F\"]) \\\n",
            "     .avg(\"avg_sal\") \\\n",
            "     .filter(col(\"F\") > col(\"M\")) \\\n",
            "     .show()\n"
        ]
    },
    64: {
        "title": 'Identify "Salary Compression": Current employees hired before 1990 earning less than avg of those hired after 2000.',
        "concept": "Complex Correlated Subquery",
        "tables": ["salary"],
        "solution": [
            "old_hires_avg = salary_df.join(employee_df, salary_df.employee_id == employee_df.id) \\\n",
            "                        .filter((employee_df.hire_date > '2000-01-01') & (salary_df.to_date == '9999-01-01')) \\\n",
            "                        .agg(avg(\"amount\")).collect()[0][0]\n",
            "\n",
            "salary_df.join(employee_df, \"employee_id\") \\\n",
            "         .filter((col(\"hire_date\") < '1990-01-01') & (col(\"to_date\") == '9999-01-01')) \\\n",
            "         .filter(col(\"amount\") < old_hires_avg) \\\n",
            "         .show()\n"
        ]
    },
    65: {
        "title": "Rank salary within department and calculate the percentile (0 to 1).",
        "concept": "Percent_Rank",
        "tables": ["salary"],
        "solution": [
            "cond = [salary_df.employee_id == department_employee_df.employee_id, salary_df.to_date == '9999-01-01',\n",
            "        department_employee_df.to_date == '9999-01-01', department_employee_df.department_id == department_df.id]\n",
            "\n",
            "joined = salary_df.join(department_employee_df, cond).join(department_df, \"department_id\")\n",
            "window_spec = Window.partitionBy(\"dept_name\").orderBy(\"amount\")\n",
            "joined.withColumn(\"pct\", percent_rank().over(window_spec)).show()\n"
        ]
    },
    66: {
        "title": "Find the 2nd highest salary in each department.",
        "concept": "Dense Rank Filtering",
        "tables": ["salary"],
        "solution": [
            "cond = [salary_df.employee_id == department_employee_df.employee_id, salary_df.to_date == '9999-01-01',\n",
            "        department_employee_df.to_date == '9999-01-01', department_employee_df.department_id == department_df.id]\n",
            "\n",
            "joined = salary_df.join(department_employee_df, cond).join(department_df, \"department_id\")\n",
            "window_spec = Window.partitionBy(\"dept_name\").orderBy(col(\"amount\").desc())\n",
            "joined.withColumn(\"rn\", dense_rank().over(window_spec)).filter(col(\"rn\") == 2).show()\n"
        ]
    },
    67: {
        "title": "Find employees who have never had a salary change (Single entry in salary table).",
        "concept": "Group By Having Count",
        "tables": ["salary"],
        "solution": [
            "salary_df.groupBy(\"employee_id\").agg(count(\"*\").alias(\"cnt\")) \\\n",
            "        .filter(col(\"cnt\") == 1) \\\n",
            "        .show()\n"
        ]
    },
    68: {
        "title": "Find the most common first name in the company.",
        "concept": "Mode Calculation",
        "tables": [],
        "solution": [
            "employee_df.groupBy(\"first_name\").count() \\\n",
            "            .orderBy(col(\"count\").desc()) \\\n",
            "            .limit(1) \\\n",
            "            .show()\n"
        ]
    },
    69: {
        "title": "Calculate average time (in days) between salary changes for each employee.",
        "concept": "Self Join Date Diff",
        "tables": ["salary"],
        "solution": [
            "window_spec = Window.partitionBy(\"employee_id\").orderBy(\"from_date\")\n",
            "salary_df.withColumn(\"next_date\", lead(\"from_date\").over(window_spec)) \\\n",
            "        .withColumn(\"diff\", datediff(col(\"next_date\"), col(\"from_date\"))) \\\n",
            "        .groupBy(\"employee_id\") \\\n",
            "        .avg(\"diff\") \\\n",
            "        .show()\n"
        ]
    },
    70: {
        "title": "Find employees whose salary has decreased (Self Join).",
        "concept": "Detecting anomalies",
        "tables": ["salary"],
        "solution": [
            "s1 = salary_df.alias(\"s1\")\n",
            "s2 = salary_df.alias(\"s2\")\n",
            "\n",
            "s1.join(s2, (col(\"s1.employee_id\") == col(\"s2.employee_id\")) & \n",
            "           (col(\"s2.from_date\") > col(\"s1.from_date\")) & \n",
            "           (col(\"s2.amount\") < col(\"s1.amount\"))) \\\n",
            "   .select(col(\"s1.employee_id\")) \\\n",
            "   .distinct() \\\n",
            "   .show()\n"
        ]
    },
    71: {
        "title": "Find the department with the highest number of unique job titles.",
        "concept": "Count Distinct Aggregation",
        "tables": ["title"],
        "solution": [
            "joined = department_df.join(department_employee_df, department_df.id == department_employee_df.department_id) \\\n",
            "                      .join(title_df, department_employee_df.employee_id == title_df.employee_id)\n",
            "\n",
            "joined.groupBy(\"dept_name\") \\\n",
            "      .agg(countDistinct(\"title\").alias(\"unique_titles\")) \\\n",
            "      .orderBy(col(\"unique_titles\").desc()) \\\n",
            "      .limit(1) \\\n",
            "      .show()\n"
        ]
    },
    72: {
        "title": "Identify departments that shrank in headcount between 1999 and 2000.",
        "concept": "Time Slice Comparison",
        "tables": [],
        "solution": [
            "def get_count(year):\n",
            "    cond = [(department_df.id == department_employee_df.department_id),\n",
            "            (department_employee_df.from_date <= lit(f'{year}-12-31')),\n",
            "            (department_employee_df.to_date >= lit(f'{year}-01-01'))]\n",
            "    return department_df.join(department_employee_df, cond) \\\n",
            "                       .groupBy(department_df.id) \\\n",
            "                       .agg(count(\"*\").alias(f\"cnt{year}\"))\n",
            "\n",
            "c99 = get_count(1999)\n",
            "c00 = get_count(2000)\n",
            "\n",
            "c99.join(c00, \"id\").filter(col(\"cnt1999\") > col(\"cnt2000\")).show()\n"
        ]
    },
    73: {
        "title": "Find the most common 'Next Title' after 'Staff'.",
        "concept": "Lead + Group By",
        "tables": ["title"],
        "solution": [
            "window_spec = Window.partitionBy(\"employee_id\").orderBy(\"from_date\")\n",
            "next_title = title_df.withColumn(\"next_title\", lead(\"title\").over(window_spec))\n",
            "\n",
            "next_title.filter(col(\"title\") == \"Staff\") \\\n",
            "          .filter(col(\"next_title\").isNotNull()) \\\n",
            "          .groupBy(\"next_title\") \\\n",
            "          .count() \\\n",
            "          .orderBy(col(\"count\").desc()) \\\n",
            "          .show()\n"
        ]
    },
    74: {
        "title": "Count how many titles each employee has held in their career.",
        "concept": "Group By",
        "tables": ["title"],
        "solution": [
            "title_df.groupBy(\"employee_id\").count().orderBy(col(\"count\").desc()).show()\n"
        ]
    },
    75: {
        "title": "Find employees who held a 'Manager' title but are not currently managers.",
        "concept": "Anti-Join on History",
        "tables": ["title", "department_manager"],
        "solution": [
            "current_mgrs = department_manager_df.filter(col(\"to_date\") == '9999-01-01').select(\"employee_id\")\n",
            "\n",
            "mgrs_history = title_df.filter(col(\"title\").contains(\"Manager\")).select(\"employee_id\").distinct()\n",
            "\n",
            "mgrs_history.join(current_mgrs, \"employee_id\", \"left_anti\") \\\n",
            "             .join(employee_df, \"employee_id\") \\\n",
            "             .show()\n"
        ]
    },
    76: {
        "title": "Calculate the average tenure (in days) per department.",
        "concept": "Average Date Diff",
        "tables": [],
        "solution": [
            "joined = department_employee_df.join(department_df, department_employee_df.department_id == department_df.id)\n",
            "joined.withColumn(\"tenure\", datediff(col(\"to_date\"), col(\"from_date\"))) \\\n",
            "      .groupBy(\"dept_name\") \\\n",
            "      .avg(\"tenure\") \\\n",
            "      .show()\n"
        ]
    },
    77: {
        "title": "List all departments and the number of current employees, ensuring empty departments are listed.",
        "concept": "Left Join",
        "tables": [],
        "solution": [
            "department_df.join(department_employee_df, (department_df.id == department_employee_df.department_id) & (col(\"to_date\") == '9999-01-01'), \"left\") \\\n",
            "              .groupBy(\"dept_name\") \\\n",
            "              .count() \\\n",
            "              .show()\n"
        ]
    },
    78: {
        "title": "Find departments with a higher average salary than the company average.",
        "concept": "Having > Global Avg",
        "tables": ["salary"],
        "solution": [
            "global_avg = salary_df.filter(col(\"to_date\") == '9999-01-01').agg(avg(\"amount\")).collect()[0][0]\n",
            "\n",
            "cond = [salary_df.employee_id == department_employee_df.employee_id, salary_df.to_date == '9999-01-01',\n",
            "        department_employee_df.to_date == '9999-01-01', department_employee_df.department_id == department_df.id]\n",
            "\n",
            "joined = salary_df.join(department_employee_df, cond).join(department_df, \"department_id\")\n",
            "joined.groupBy(\"dept_name\") \\\n",
            "      .agg(avg(\"amount\").alias(\"avg_sal\")) \\\n",
            "      .filter(col(\"avg_sal\") > global_avg) \\\n",
            "      .show()\n"
        ]
    },
    79: {
        "title": 'Find the "Golden Handcuffs": Employees with >10 years tenure but below average salary.',
        "concept": "Complex Filtering",
        "tables": ["salary"],
        "solution": [
            "avg_sal = salary_df.filter(col(\"to_date\") == '9999-01-01').agg(avg(\"amount\")).collect()[0][0]\n",
            "\n",
            "salary_df.join(employee_df, \"employee_id\") \\\n",
            "        .filter((datediff(current_date(), col(\"hire_date\")) > 3650) & (col(\"to_date\") == '9999-01-01')) \\\n",
            "        .filter(col(\"amount\") < avg_sal) \\\n",
            "        .show()\n"
        ]
    },
    80: {
        "title": "Find departments where the headcount doubled (or more) since 1990.",
        "concept": "Time Slice Math",
        "tables": [],
        "solution": [
            "joined = department_df.join(department_employee_df, department_df.id == department_employee_df.department_id)\n",
            "\n",
            "joined.agg(\n",
            "    sum(when(col(\"from_date\") <= '1990-12-31', 1).otherwise(0)).alias(\"c90\"),\n",
            "    sum(when(col(\"to_date\") == '9999-01-01', 1).otherwise(0)).alias(\"cNow\")\n",
            ").filter(col(\"cNow\") >= col(\"c90\") * 2).show()\n"
        ]
    },
    81: {
        "title": "Find duplicate records in the `department_employee` table (Same emp, dept, dates).",
        "concept": "Group By Having Count > 1",
        "tables": [],
        "solution": [
            "department_employee_df.groupBy(\"employee_id\", \"department_id\", \"from_date\", \"to_date\") \\\n",
            "                      .count() \\\n",
            "                      .filter(col(\"count\") > 1) \\\n",
            "                      .show()\n"
        ]
    },
    82: {
        "title": "Find employees with NULL gender (Data Quality check).",
        "concept": "Is Null",
        "tables": [],
        "solution": [
            "employee_df.filter(col(\"gender\").isNull()).show()\n"
        ]
    },
    83: {
        "title": "Convert all gender codes to Full Names ('M' -> 'Male').",
        "concept": "When/Else",
        "tables": [],
        "solution": [
            "employee_df.withColumn(\"full_gender\", \\\n",
            "               when(col(\"gender\") == \"M\", \"Male\")\n",
            "               .when(col(\"gender\") == \"F\", \"Female\")\n",
            "               .otherwise(\"Other\")) \\\n",
            "           .show()\n"
        ]
    },
    84: {
        "title": "Find employees who have 'to_date' in the past (Left the company).",
        "concept": "Date Compare",
        "tables": [],
        "solution": [
            "department_employee_df.join(employee_df, department_employee_df.employee_id == employee_df.id) \\\n",
            "                      .filter(col(\"to_date\") < current_date()) \\\n",
            "                      .show()\n"
        ]
    },
    85: {
        "title": "Standardize 'Department' names to Uppercase.",
        "concept": "Upper",
        "tables": [],
        "solution": [
            "department_df.withColumn(\"dept_upper\", upper(col(\"dept_name\"))).show()\n"
        ]
    },
    86: {
        "title": "Extract the Year of hire and Month of hire into separate columns.",
        "concept": "Extract Year/Month",
        "tables": [],
        "solution": [
            "employee_df.withColumn(\"hire_year\", year(col(\"hire_date\"))) \\\n",
            "           .withColumn(\"hire_month\", month(col(\"hire_date\"))) \\\n",
            "           .show()\n"
        ]
    },
    87: {
        "title": "Find the most frequent starting salary among all employees.",
        "concept": "Mode on Amount",
        "tables": ["salary"],
        "solution": [
            "salary_df.groupBy(\"amount\").count().orderBy(col(\"count\").desc()).limit(1).show()\n"
        ]
    },
    88: {
        "title": "Check for data inconsistency: Employee in `title` table but not in `employee` table.",
        "concept": "Left Anti Join (FK Violation)",
        "tables": ["title"],
        "solution": [
            "title_df.join(employee_df, title_df.employee_id == employee_df.id, \"left_anti\") \\\n",
            "        .select(\"employee_id\") \\\n",
            "        .show()\n"
        ]
    },
    89: {
        "title": "Truncate the `hire_date` to the first of the year.",
        "concept": "Trunc",
        "tables": [],
        "solution": [
            "employee_df.withColumn(\"hire_year_start\", trunc(col(\"hire_date\"), \"year\")).show()\n"
        ]
    },
    90: {
        "title": "Calculate the time difference between joining a department and becoming a manager (if applicable).",
        "concept": "Self Join Date Calculation",
        "tables": ["department_manager"],
        "solution": [
            "cond = [department_employee_df.employee_id == department_manager_df.employee_id,\n",
            "        department_employee_df.department_id == department_manager_df.department_id,\n",
            "        department_manager_df.from_date > department_employee_df.from_date]\n",
            "\n",
            "department_employee_df.join(department_manager_df, cond) \\\n",
            "                      .withColumn(\"days_to_mgr\", datediff(col(\"from_date\"), col(\"from_date\"))) \\\n",
            "                      .show()\n"
        ]
    },
    91: {
        "title": "Repartition the employee DataFrame by department_id to optimize joins.",
        "concept": "Repartition",
        "tables": [],
        "solution": [
            "joined = employee_df.join(department_employee_df, employee_df.id == department_employee_df.employee_id)\n",
            "# Repartition by department_id to ensure all employees of a dept are in same partition\n",
            "joined.repartition(10, \"department_id\")\n"
        ]
    },
    92: {
        "title": "Cache the salary table as it is used frequently.",
        "concept": "Cache/Persist",
        "tables": ["salary"],
        "solution": [
            "salary_df.cache()\n",
            "# Perform actions...\n",
            "salary_df.count()\n"
        ]
    },
    93: {
        "title": "Sample 10% of the employees to create a test dataset.",
        "concept": "Sample",
        "tables": [],
        "solution": [
            "sample_df = employee_df.sample(withReplacement=False, fraction=0.1)\n",
            "sample_df.show()\n"
        ]
    },
    94: {
        "title": "Create a view from the DataFrame for temporary SQL usage.",
        "concept": "CreateOrReplaceTempView",
        "tables": [],
        "solution": [
            "employee_df.createOrReplaceTempView(\"emp_view\")\n",
            "# (This allows using spark.sql, though prompts asked to avoid it, creating views is a valid DataFrame API method).\n"
        ]
    },
    95: {
        "title": "Calculate approximate count of distinct salaries (HyperLogLog).",
        "concept": "ApproxCountDistinct",
        "tables": ["salary"],
        "solution": [
            "salary_df.select(approxCountDistinct(\"amount\")).show()\n"
        ]
    },
    96: {
        "title": "Sort the entire DataFrame globally by salary (Expensive operation).",
        "concept": "Sort",
        "tables": ["salary"],
        "solution": [
            "salary_df.orderBy(col(\"amount\").desc()).show()\n"
        ]
    },
    97: {
        "title": "Drop duplicate salary records for an employee (keeping the latest).",
        "concept": "DropDuplicates",
        "tables": ["salary"],
        "solution": [
            "window_spec = Window.partitionBy(\"employee_id\").orderBy(col(\"from_date\").desc())\n",
            "latest_salaries = salary_df.withColumn(\"rn\", row_number().over(window_spec)) \\\n",
            "                          .filter(col(\"rn\") == 1) \\\n",
            "                          .drop(\"rn\")\n",
            "latest_salaries.show()\n"
        ]
    },
    98: {
        "title": "Fill NULL values in `to_date` with current date (hypothetical scenario).",
        "concept": "FillNa",
        "tables": [],
        "solution": [
            "# Assuming some to_dates are null (not the case in this DB strictly, but good practice)\n",
            "cleaned_df = department_employee_df.fillna({'to_date': '2023-01-01'})\n"
        ]
    },
    99: {
        "title": "Check the schema of the DataFrame programmatically.",
        "concept": "PrintSchema",
        "tables": [],
        "solution": [
            "employee_df.printSchema()\n"
        ]
    },
    100: {
        "title": "Calculate the total number of partitions in the employee DataFrame.",
        "concept": "RDD manipulation",
        "tables": [],
        "solution": [
            "print(employee_df.rdd.getNumPartitions())\n"
        ]
    }
}

def main():
    output_dir = r"C:\Users\prateek\Desktop\spark\pyspark_practice\notebooks"
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
