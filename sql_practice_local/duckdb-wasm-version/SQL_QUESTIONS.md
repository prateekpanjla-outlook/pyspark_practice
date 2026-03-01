# SQL Practice Questions - MySQL Employees Sample Database

This document contains 100 SQL practice questions based on the **MySQL Employees Sample Database** (test_db), an open dataset provided by [datacharmer/test_db](https://github.com/datacharmer/test_db).

## Database Overview

### Statistics
| Table | Record Count |
|-------|--------------|
| employees | 300,024 |
| departments | 9 |
| dept_emp | 331,603 |
| dept_manager | 24 |
| titles | 443,308 |
| salaries | 2,844,047 |
| **Total** | **~3.6M records** |

### Schema Visualization

```
┌─────────────┐     ┌──────────┐     ┌─────────────┐
│  employees  │────>│  titles  │     │ departments │
└──────┬──────┘     └──────────┘     └──────┬──────┘
       │                                      ▲
       │                                      │
       ▼                                      │
┌──────────┐     ┌──────────────┐             │
│ salaries │     │   dept_emp   │─────────────┘
└──────────┘     └──────┬───────┘
                       │
                       ▼
                ┌──────────────┐
                │ dept_manager │
                └──────────────┘
```

### Table Definitions

#### `employees` (300,024 records)
Contains core employee information.

| Column | Type | Description |
|--------|------|-------------|
| emp_no | INT | Primary Key |
| birth_date | DATE | Date of birth |
| first_name | VARCHAR | First name |
| last_name | VARCHAR | Last name |
| gender | ENUM | 'M' or 'F' |
| hire_date | DATE | Date hired |

#### `departments` (9 records)
Contains department information.

| Column | Type | Description |
|--------|------|-------------|
| dept_no | CHAR(4) | Primary Key (e.g., 'd001') |
| dept_name | VARCHAR | Department name |

#### `dept_emp` (331,603 records)
Junction table mapping employees to departments (historical).

| Column | Type | Description |
|--------|------|-------------|
| emp_no | INT | Foreign Key → employees.emp_no |
| dept_no | CHAR(4) | Foreign Key → departments.dept_no |
| from_date | DATE | Start date in department |
| to_date | DATE | End date ('9999-01-01' = current) |

#### `dept_manager` (24 records)
Maps department managers to departments.

| Column | Type | Description |
|--------|------|-------------|
| dept_no | CHAR(4) | Foreign Key → departments.dept_no |
| emp_no | INT | Foreign Key → employees.emp_no |
| from_date | DATE | Start date as manager |
| to_date | DATE | End date ('9999-01-01' = current) |

#### `titles` (443,308 records)
Historical job titles for each employee.

| Column | Type | Description |
|--------|------|-------------|
| emp_no | INT | Foreign Key → employees.emp_no |
| title | VARCHAR | Job title (e.g., 'Staff', 'Senior Engineer') |
| from_date | DATE | Start date for title |
| to_date | DATE | End date ('9999-01-01' = current) |

#### `salaries` (2,844,047 records)
Historical salary information for each employee.

| Column | Type | Description |
|--------|------|-------------|
| emp_no | INT | Foreign Key → employees.emp_no |
| salary | INT | Annual salary |
| from_date | DATE | Start date for salary |
| to_date | DATE | End date ('9999-01-01' = current) |

---

## Questions by Category

### Group 1: Joins, Filtering & Basic Aggregates (1-10)

| # | Question | Difficulty | Concepts |
|---|-----------|------------|----------|
| 1 | List all current employees with their department names | Beginner | Inner Join, Current Date Filter |
| 2 | Find the current head of each department | Beginner | Inner Join, Manager Table |
| 3 | List employees who are currently "Senior Engineers" | Beginner | Multi-table Join |
| 4 | Calculate the average salary per department | Beginner | Aggregation |
| 5 | Find departments with more than 10,000 current employees | Intermediate | Having Clause |
| 6 | Find employees hired in the year 1995 | Beginner | Date Function |
| 7 | Count how many distinct job titles exist | Beginner | Distinct Count |
| 8 | List employees who have never held a manager title | Intermediate | Left Join (Anti-Join) |
| 9 | Show the salary history for employee ID 10001 | Beginner | Simple Filter |
| 10 | Find the maximum salary ever paid | Beginner | Max |

---

### Group 2: Window Functions - Ranking & Offsets (11-20)

| # | Question | Difficulty | Concepts |
|---|-----------|------------|----------|
| 11 | Find the top 3 earners in each department | Intermediate | `row_number` |
| 12 | Rank employees by salary within their title | Intermediate | `dense_rank` |
| 13 | Compare current salary with the previous salary (Raise amount) | Intermediate | `lag` |
| 14 | Find the date of the next job title change | Intermediate | `lead` |
| 15 | Calculate salary quartiles (NTILE) | Intermediate | `ntile` |
| 16 | Find the percentage rank of a salary within the company | Advanced | `percent_rank` |
| 17 | Count the number of employees hired in the same year as each employee | Intermediate | `count` Window |
| 18 | Find the first hire date for every department | Intermediate | `first_value` |
| 19 | Find employees with duplicate titles | Intermediate | Window Logic |
| 20 | Median salary approximation | Advanced | Window + Row Number |

---

### Group 3: Sliding Windows (Frames) (21-25)

| # | Question | Difficulty | Concepts |
|---|-----------|------------|----------|
| 21 | 3-Month Moving Average of Salary changes (Time Series) | Advanced | `rowsBetween` |
| 22 | Running Total of salaries paid globally by date | Advanced | `rangeBetween` (Unbounded) |
| 23 | Sum of Current Salary + Previous 2 Salaries | Intermediate | `rowsBetween` (Centered) |
| 24 | Department Average Salary excluding the current row | Advanced | Window Filter Frame |
| 25 | Rolling 12-month total hires | Advanced | Time-based Window |

---

### Group 4: Pivots & Complex Aggregations (26-30)

| # | Question | Difficulty | Concepts |
|---|-----------|------------|----------|
| 26 | Count of Employees by Gender in each Department (Pivot) | Intermediate | `pivot` |
| 27 | Average Salary by Department and Title (Pivot) | Advanced | `pivot` with Aggregation |
| 28 | List all employees in a department as a comma-separated string | Advanced | `group_concat` / `collect_list` |
| 29 | Histogram of Salary Ranges | Intermediate | Bucketing (CASE WHEN) |
| 30 | Most frequent title per department (Mode) | Advanced | Window count aliasing |

---

### Group 5: Self Joins & Comparisons (31-40)

| # | Question | Difficulty | Concepts |
|---|-----------|------------|----------|
| 31 | Employees who earn more than their current manager | Advanced | Complex Join |
| 32 | Pairs of employees sharing the same birthday | Intermediate | Self Join with inequality |
| 33 | Identify employees who held multiple titles | Intermediate | Self Join Count |
| 34 | Employees who have changed departments | Intermediate | Self Join History |
| 35 | Employees whose salary decreased | Intermediate | Self Join with Date Logic |
| 36 | Find gaps in employment history | Advanced | Date Logic Self Join |
| 37 | Tenure of employees in current department | Intermediate | Date Diff |
| 38 | Salary increase percentage on the last raise | Advanced | Aggregation over dates |
| 39 | Departments with average salary above company average | Intermediate | Scalar Subquery |
| 40 | Find the longest serving manager of each department | Intermediate | Duration Calculation & Max |

---

### Group 6: Recursion & Time Series Logic (41-50)

| # | Question | Difficulty | Concepts |
|---|-----------|------------|----------|
| 41 | Generate a report of total hires per month, filling months with 0 hires | Advanced | Time Series Generation |
| 42 | Calculate cumulative headcount of the company over time | Advanced | Running Aggregation on Time Series |
| 43 | Monthly Active Users (MAU) - Count of employees active in each month | Advanced | Expanding date ranges |
| 44 | Find months where salary expenses spiked compared to previous month | Intermediate | Time Series Comparison |
| 45 | Employee Tenure distribution (Buckets) | Intermediate | Window based on time |
| 46 | Find consecutive years where an employee got a raise | Intermediate | Complex Logic (Self Join) |
| 47 | Determine the hire date cohort (Year) and retention count | Intermediate | Grouping + Window |
| 48 | Department headcount trend | Beginner | Simple Count |
| 49 | Identify employees whose current salary is not the highest they've ever earned | Intermediate | Window vs Current Value |
| 50 | Find the department with the highest salary variance | Advanced | Statistical Aggregation (VarPop/VarSamp) |

---

### Group 7: Set Operations & Advanced Filtering (51-60)

| # | Question | Difficulty | Concepts |
|---|-----------|------------|----------|
| 51 | List employees who have worked in both 'Development' and 'Research' departments | Intermediate | INTERSECT / Semi-Joins |
| 52 | List employees who have worked in 'Development' but never in 'Research' | Intermediate | EXCEPT / Left Anti Join |
| 53 | Create a unified list of all current Employees and all current Managers | Beginner | UNION / Union All |
| 54 | Find employees who were hired on a weekend (Saturday or Sunday) | Beginner | Date Function (Day of Week) |
| 55 | Find employees born in a Leap Year | Intermediate | Date Math Modulo |
| 56 | Employees whose last name starts with the same letter as their first name | Beginner | String Manipulation |
| 57 | Find employees whose first name contains 'z' or 'Z' | Beginner | Like / Contains |
| 58 | Find employees with a 'Senior' title that are younger than 30 years old | Intermediate | Date Diff + Join |
| 59 | Calculate the length of each employee's full name | Beginner | String Length |
| 60 | Identify employees who have 'Anne' anywhere in their name | Beginner | Concat + Like |

---

### Group 8: Statistical Analysis & Outliers (61-70)

| # | Question | Difficulty | Concepts |
|---|-----------|------------|----------|
| 61 | Calculate Z-Scores for current salaries (Statistical outlier detection) | Advanced | Window Aggregation + StdDev |
| 62 | Find departments with the widest salary range (Max - Min) | Intermediate | Aggregation Math |
| 63 | Find departments where the average female salary is higher than average male salary | Advanced | Group By Case When + Having |
| 64 | Identify "Salary Compression": Current employees hired before 1990 earning less than avg of those hired after 2000 | Advanced | Complex Correlated Subquery |
| 65 | Rank salary within department and calculate the percentile (0 to 1) | Intermediate | Percent_Rank |
| 66 | Find the 2nd highest salary in each department | Intermediate | Dense Rank Filtering |
| 67 | Find employees who have never had a salary change | Beginner | Group By Having Count |
| 68 | Find the most common first name in the company | Beginner | Mode Calculation |
| 69 | Calculate average time (in days) between salary changes for each employee | Intermediate | Self Join Date Diff |
| 70 | Find employees whose salary has decreased | Intermediate | Detecting anomalies |

---

### Group 9: Department & Title Dynamics (71-80)

| # | Question | Difficulty | Concepts |
|---|-----------|------------|----------|
| 71 | Find the department with the highest number of unique job titles | Intermediate | Count Distinct Aggregation |
| 72 | Identify departments that shrank in headcount between 1999 and 2000 | Advanced | Time Slice Comparison |
| 73 | Find the most common "Next Title" after "Staff" | Intermediate | Lead + Group By |
| 74 | Count how many titles each employee has held in their career | Beginner | Group By |
| 75 | Find employees who held a "Manager" title but are not currently managers | Intermediate | Anti-Join on History |
| 76 | Calculate the average tenure (in days) per department | Intermediate | Average Date Diff |
| 77 | List all departments and the number of current employees, ensuring empty departments are listed | Intermediate | Left Join |
| 78 | Find departments with a higher average salary than the company average | Intermediate | Having > Global Avg |
| 79 | Find the "Golden Handcuffs": Employees with >10 years tenure but below average salary | Advanced | Complex Filtering |
| 80 | Find departments where the headcount doubled (or more) since 1990 | Advanced | Time Slice Math |

---

### Group 10: Data Hygiene & Advanced Logic (81-90)

| # | Question | Difficulty | Concepts |
|---|-----------|------------|----------|
| 81 | Find duplicate records in the `dept_emp` table | Intermediate | Group By Having Count > 1 |
| 82 | Find employees with NULL gender (Data Quality check) | Beginner | Is Null |
| 83 | Convert all gender codes to Full Names ('M' -> 'Male') | Beginner | When/Else |
| 84 | Find employees who have 'to_date' in the past (Left the company) | Beginner | Date Compare |
| 85 | Standardize 'Department' names to Uppercase | Beginner | Upper |
| 86 | Extract the Year of hire and Month of hire into separate columns | Beginner | Extract Year/Month |
| 87 | Find the most frequent starting salary among all employees | Beginner | Mode on Amount |
| 88 | Check for data inconsistency: Employee in `titles` table but not in `employees` table | Intermediate | Left Anti Join (FK Violation) |
| 89 | Truncate the `hire_date` to the first of the year | Beginner | Trunc |
| 90 | Calculate the time difference between joining a department and becoming a manager | Intermediate | Self Join Date Calculation |

---

### Group 11: PySpark Optimization & Utility (91-100)

| # | Question | Difficulty | Concepts |
|---|-----------|------------|----------|
| 91 | Repartition the employee DataFrame by department_id to optimize joins | Advanced | Repartition |
| 92 | Cache the salary table as it is used frequently | Intermediate | Cache/Persist |
| 93 | Sample 10% of the employees to create a test dataset | Beginner | Sample |
| 94 | Create a view from the DataFrame for temporary SQL usage | Intermediate | CreateOrReplaceTempView |
| 95 | Calculate approximate count of distinct salaries (HyperLogLog) | Intermediate | ApproxCountDistinct |
| 96 | Sort the entire DataFrame globally by salary | Intermediate | Sort |
| 97 | Drop duplicate salary records for an employee (keeping the latest) | Intermediate | DropDuplicates |
| 98 | Fill NULL values in `to_date` with current date | Beginner | FillNa |
| 99 | Check the schema of the DataFrame programmatically | Beginner | PrintSchema |
| 100 | Calculate the total number of partitions in the employee DataFrame | Intermediate | RDD manipulation |

---

## Sample Questions with Solutions

### Example 1: Top 3 Earners Per Department (Question #11)

**Question:** Find the top 3 earners in each department.

**SQL Solution:**
```sql
WITH Ranked AS (
    SELECT e.emp_no, d.dept_name, s.salary,
           ROW_NUMBER() OVER (PARTITION BY d.dept_name ORDER BY s.salary DESC) as rn
    FROM salaries s
    JOIN dept_emp de ON s.emp_no = de.emp_no AND de.to_date = '9999-01-01'
    JOIN departments d ON de.dept_no = d.dept_no
    JOIN employees e ON s.emp_no = e.emp_no
    WHERE s.to_date = '9999-01-01'
)
SELECT * FROM Ranked WHERE rn <= 3;
```

**PySpark Solution:**
```python
from pyspark.sql import Window
from pyspark.sql.functions import row_number, col

window_spec = Window.partitionBy("dept_name").orderBy(col("salary").desc())
ranked = joined.withColumn("rn", row_number().over(window_spec))
result = ranked.filter(col("rn") <= 3)
```

---

### Example 2: Employees with Multiple Titles (Question #33)

**Question:** Identify employees who held multiple titles.

**SQL Solution:**
```sql
SELECT e.emp_no, e.first_name, COUNT(t.title) as title_count
FROM employees e
JOIN titles t ON e.emp_no = t.emp_no
GROUP BY e.emp_no, e.first_name
HAVING COUNT(t.title) > 1;
```

**PySpark Solution:**
```python
from pyspark.sql.functions import count, col

employees_df.join(titles_df, employees_df.emp_no == titles_df.emp_no) \
    .groupBy("emp_no", "first_name") \
    .agg(count("title").alias("title_count")) \
    .filter(col("title_count") > 1) \
    .show()
```

---

### Example 3: Salary Variance by Department (Question #50)

**Question:** Find the department with the highest salary variance.

**SQL Solution:**
```sql
SELECT d.dept_name, VAR_POP(s.salary) as variance
FROM salaries s
JOIN dept_emp de ON s.emp_no = de.emp_no AND de.to_date = '9999-01-01'
JOIN departments d ON de.dept_no = d.dept_no
WHERE s.to_date = '9999-01-01'
GROUP BY d.dept_name
ORDER BY variance DESC
LIMIT 1;
```

---

## Integration with DuckDB-Wasm Platform

### JSON Question Format

For use with the DuckDB-Wasm SQL practice platform, questions should be formatted as follows:

```json
{
  "slug": "top-3-earners-per-dept",
  "title": "Top 3 Earners Per Department",
  "description": "Find the top 3 highest-paid employees in each department. Only consider current salaries and current department assignments.",
  "difficulty": "intermediate",
  "category": "Window Functions",
  "startingCode": "-- Use ROW_NUMBER() window function\nSELECT \n    e.emp_no,\n    e.first_name,\n    e.last_name,\n    d.dept_name,\n    s.salary\nFROM employees e\nJOIN salaries s ON e.emp_no = s.emp_no AND s.to_date = '9999-01-01'\nJOIN dept_emp de ON e.emp_no = de.emp_no AND de.to_date = '9999-01-01'\nJOIN departments d ON de.dept_no = d.dept_no\n",
  "schema": {
    "tables": [
      {
        "name": "employees",
        "columns": [
          {"name": "emp_no", "type": "INTEGER"},
          {"name": "first_name", "type": "VARCHAR"},
          {"name": "last_name", "type": "VARCHAR"},
          {"name": "birth_date", "type": "DATE"},
          {"name": "gender", "type": "VARCHAR"},
          {"name": "hire_date", "type": "DATE"}
        ]
      }
    ]
  },
  "expectedOutput": {
    "columns": ["emp_no", "first_name", "last_name", "dept_name", "salary"],
    "rows": [
      [10001, "Georgi", "Facello", "Development", 88958],
      ...
    ]
  }
}
```

---

## Difficulty Distribution

| Difficulty | Count | Percentage |
|------------|-------|------------|
| Beginner | 30 | 30% |
| Intermediate | 50 | 50% |
| Advanced | 20 | 20% |

---

## Concept Coverage

| Concept | Questions |
|---------|-----------|
| Joins (Inner, Left, Anti) | 1-10, 31-40 |
| Window Functions | 11-25, 65-70 |
| Aggregation | 4-10, 26-30, 71-80 |
| Set Operations (INTERSECT, EXCEPT, UNION) | 51-53 |
| String Functions | 56-60, 85-86 |
| Date Functions | 6, 37, 54-55, 84, 86, 89-90 |
| Time Series | 21-25, 41-50 |
| Statistical Analysis | 50, 61-70 |
| Data Quality | 67, 81-82, 87-88 |

---

## References

- [MySQL Employees Sample Database](https://github.com/datacharmer/test_db)
- [DuckDB-Wasm Documentation](https://duckdb.org/api/wasm/)
- License: Creative Commons Attribution-Share Alike 3.0 Unported License

---

*This document contains 100 SQL practice questions generated based on the MySQL Employees Sample Database. The questions are designed for use with browser-based SQL learning platforms like DuckDB-Wasm.*
