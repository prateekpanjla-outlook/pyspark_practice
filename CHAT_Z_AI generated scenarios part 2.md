

Here are **50 additional problems and solutions** focused on more advanced analytics, set operations, statistical analysis, and edge-case detection, utilizing your specific schema structure.

### Group 7: Set Operations & Advanced Filtering (51-60)

**51. List employees who have worked in both 'Development' and 'Research' departments.**
*Concept: INTERSECT / Semi-Joins*

```sql
-- SQL
SELECT DISTINCT e.id, e.first_name
FROM employee e
JOIN department_employee de ON e.id = de.employee_id
JOIN department d ON de.department_id = d.id
WHERE d.dept_name = 'Development'
INTERSECT
SELECT DISTINCT e.id, e.first_name
FROM employee e
JOIN department_employee de ON e.id = de.employee_id
JOIN department d ON de.department_id = d.id
WHERE d.dept_name = 'Research';
```

```python
# PySpark
# Get IDs for Development
employee_df = spark.read.jdbc(url="jdbc:postgresql://localhost:5432/employees",table="employees.employee",
                         properties={"user": "vagrant", "password": "vagrant","driver": "org.postgresql.Driver"})


department_df = spark.read.jdbc(url="jdbc:postgresql://localhost:5432/employees",table="employees.department",
                         properties={"user": "vagrant", "password": "vagrant","driver": "org.postgresql.Driver"})


department_employee_df = spark.read.jdbc(url="jdbc:postgresql://localhost:5432/employees",table="employees.department_employee",
                         properties={"user": "vagrant", "password": "vagrant","driver": "org.postgresql.Driver"})

# PySpark
# Get IDs for Development
from pyspark.sql.functions import col

# 1. Create aliases for clarity
e = employee_df.alias("e")
d = department_df.alias("d")
de = department_employee_df.alias("de")

# 2. Use the aliases in the join and select conditions
dev_ids = e.join(de, e["id"] == de["employee_id"]) \
             .join(d, de["department_id"] == d["id"]) \
             .filter(col("d.dept_name") == "Development") \
             .select(col("e.id")).distinct() # Explicitly select e.id

res_ids = e.join(de, e["id"] == de["employee_id"]) \
             .join(d, de["department_id"] == d["id"]) \
             .filter(col("d.dept_name") == "Research") \
             .select(col("e.id")).distinct()

# Intersect
dev_ids.intersect(res_ids).show()
```

**52. List employees who have worked in 'Development' but never in 'Research'.**
*Concept: EXCEPT / Left Anti Join*

```sql
-- SQL
SELECT DISTINCT e.id, e.first_name
FROM employee e
JOIN department_employee de ON e.id = de.employee_id
JOIN department d ON de.department_id = d.id
WHERE d.dept_name = 'Development'
EXCEPT
SELECT DISTINCT e.id, e.first_name
FROM employee e
JOIN department_employee de ON e.id = de.employee_id
JOIN department d ON de.department_id = d.id
WHERE d.dept_name = 'Research';
```

```python
# PySpark
dev = employee_df.join(department_employee_df, employee_df.id == department_employee_df.employee_id) \
                 .join(department_df, department_employee_df.department_id == department_df.id) \
                 .filter(col("dept_name") == "Development") \
                 .select("id").distinct()

res = employee_df.join(department_employee_df, employee_df.id == department_employee_df.employee_id) \
                 .join(department_df, department_employee_df.department_id == department_df.id) \
                 .filter(col("dept_name") == "Research") \
                 .select("id").distinct()

dev.join(res, "id", "left_anti").join(employee_df, "id").show()
```

**53. Create a unified list of all current Employees and all current Managers.**
*Concept: UNION / Union All*

```sql
-- SQL
SELECT e.id, e.first_name, 'Employee' as type
FROM employee e
JOIN department_employee de ON e.id = de.employee_id AND de.to_date = '9999-01-01'
UNION
SELECT e.id, e.first_name, 'Manager' as type
FROM employee e
JOIN department_manager dm ON e.id = dm.employee_id AND dm.to_date = '9999-01-01';
```

```python
# PySpark
emp = employee_df.join(department_employee_df, employee_df.id == department_employee_df.employee_id) \
                 .filter(col("to_date") == '9999-01-01') \
                 .select(employee_df.id, employee_df.first_name, lit("Employee").alias("type"))

mgr = employee_df.join(department_manager_df, employee_df.id == department_manager_df.employee_id) \
                 .filter(col("to_date") == '9999-01-01') \
                 .select(employee_df.id, employee_df.first_name, lit("Manager").alias("type"))

emp.union(mgr).show()
```

**54. Find employees who were hired on a weekend (Saturday or Sunday).**
*Concept: Date Function (Day of Week)*

```sql
-- SQL
-- Postgres uses extract(dow from date), MySQL uses DAYOFWEEK
SELECT id, first_name, hire_date
FROM employee
WHERE EXTRACT(ISODOW FROM hire_date) IN (6, 7);
```

```python
# PySpark
# Spark: 1=Sunday, 2=Monday ... 7=Saturday. OR use next_day function
employee_df.filter(dayofweek(col("hire_date")).isin(1, 7)).show()
```

**55. Find employees born in a Leap Year.**
*Concept: Date Math Modulo*

```sql
-- SQL
SELECT id, first_name, birth_date
FROM employee
WHERE (EXTRACT(YEAR FROM birth_date) % 4 = 0 
       AND EXTRACT(YEAR FROM birth_date) % 100 <> 0) 
       OR (EXTRACT(YEAR FROM birth_date) % 400 = 0);
```

```python
# PySpark
employee_df.filter((year(col("birth_date")) % 4 == 0) & 
                   ((year(col("birth_date")) % 100 != 0) | (year(col("birth_date")) % 400 == 0))) \
           .show()
```

**56. Employees whose last name starts with the same letter as their first name.**
*Concept: String Manipulation*

```sql
-- SQL
SELECT id, first_name, last_name
FROM employee
WHERE LEFT(first_name, 1) = LEFT(last_name, 1);
```

```python
# PySpark
employee_df.filter(substring(col("first_name"), 1, 1) == substring(col("last_name"), 1, 1)).show()
```

**57. Find employees whose first name contains 'z' or 'Z'.**
*Concept: Like / Contains*

```sql
-- SQL
SELECT * FROM employee WHERE first_name ILIKE '%z%';
```

```python
# PySpark
employee_df.filter(lower(col("first_name")).contains("z")).show()
```

**58. Find employees with a 'Senior' title that are younger than 30 years old (at hire).**
*Concept: Date Diff + Join*

```sql
-- SQL
SELECT e.id, e.first_name, t.title
FROM employee e
JOIN title t ON e.id = t.employee_id
WHERE t.title LIKE '%Senior%'
AND DATEDIFF(t.from_date, e.birth_date) / 365 < 30;
```

```python
# PySpark
emp_with_title = employee_df.join(title_df, employee_df.id == title_df.employee_id)
emp_with_title.filter(col("title").contains("Senior")) \
              .filter((datediff(col("from_date"), col("birth_date")) / 365) < 30) \
              .show()
```

**59. Calculate the length of each employee's full name (First + Last + space).**
*Concept: String Length*

```sql
-- SQL
SELECT id, first_name, last_name, 
       LENGTH(first_name || ' ' || last_name) as full_name_length
FROM employee;
```

```python
# PySpark
employee_df.withColumn("full_name", concat(col("first_name"), lit(" "), col("last_name"))) \
           .withColumn("length", length(col("full_name"))) \
           .show()
```

**60. Identify employees who have 'Anne' anywhere in their name.**
*Concept: Concat + Like*

```sql
-- SQL
SELECT * FROM employee 
WHERE first_name || ' ' || last_name ILIKE '%Anne%';
```

```python
# PySpark
employee_df.filter(concat(col("first_name"), lit(" "), col("last_name")).contains("Anne")).show()
```

---

### Group 8: Statistical Analysis & Outliers (61-70)

**61. Calculate Z-Scores for current salaries (Statistical outlier detection).**
*Concept: Window Aggregation + StdDev*

```sql
-- SQL
SELECT 
    employee_id, amount,
    (amount - AVG(amount) OVER ()) / STDDEV(amount) OVER () as z_score
FROM salary
WHERE to_date = '9999-01-01';
```

```python
# PySpark
window_spec = Window.rangeBetween(Window.unboundedPreceding, Window.unboundedFollowing)
current_salary = salary_df.filter(col("to_date") == '9999-01-01')

# Calculate mean and std
stats = current_salary.agg(avg("amount").alias("mean"), stddev("amount").alias("std")).collect()
mean_val = stats[0]["mean"]
std_val = stats[0]["std"]

current_salary.withColumn("z_score", (col("amount") - lit(mean_val)) / lit(std_val)).show()
```

**62. Find departments with the widest salary range (Max - Min).**
*Concept: Aggregation Math*

```sql
-- SQL
SELECT d.dept_name, 
       MAX(s.amount) - MIN(s.amount) as salary_range
FROM salary s
JOIN department_employee de ON s.employee_id = de.employee_id AND de.to_date = '9999-01-01' AND s.to_date = '9999-01-01'
JOIN department d ON de.department_id = d.id
GROUP BY d.dept_name
ORDER BY salary_range DESC;
```

```python
# PySpark
cond = [salary_df.employee_id == department_employee_df.employee_id, 
        salary_df.to_date == '9999-01-01', department_employee_df.to_date == '9999-01-01',
        department_employee_df.department_id == department_df.id]

joined = salary_df.join(department_employee_df, cond).join(department_df, "department_id")
joined.groupBy("dept_name") \
      .agg((max("amount") - min("amount")).alias("salary_range")) \
      .orderBy(col("salary_range").desc()) \
      .show()
```

**63. Find departments where the average female salary is higher than average male salary.**
*Concept: Group By Case When + Having*

```sql
-- SQL
SELECT d.dept_name, e.gender, AVG(s.amount) as avg_sal
FROM salary s
JOIN department_employee de ON s.employee_id = de.employee_id AND de.to_date = '9999-01-01'
JOIN employee e ON s.employee_id = e.id
JOIN department d ON de.department_id = d.id
WHERE s.to_date = '9999-01-01'
GROUP BY d.dept_name, e.gender
HAVING AVG(s.amount) > (
    SELECT AVG(s2.amount)
    FROM salary s2
    JOIN department_employee de2 ON s2.employee_id = de2.employee_id AND de2.to_date = '9999-01-01'
    JOIN employee e2 ON s2.employee_id = e2.id
    WHERE e2.gender = 'M' AND s2.to_date = '9999-01-01' AND de2.department_id = d.id
);
```

```python
# PySpark
# Simplify: Calculate per dept per gender avg, then pivot/self join
cond = [salary_df.employee_id == department_employee_df.employee_id, salary_df.to_date == '9999-01-01',
        department_employee_df.to_date == '9999-01-01', salary_df.employee_id == employee_df.id,
        department_employee_df.department_id == department_df.id]

stats = salary_df.join(department_employee_df, cond) \
                  .join(employee_df, "employee_id") \
                  .join(department_df, "department_id") \
                  .groupBy("dept_name", "gender") \
                  .agg(avg("amount").alias("avg_sal"))

# Pivot to compare
stats.groupBy("dept_name") \
     .pivot("gender", ["M", "F"]) \
     .avg("avg_sal") \
     .filter(col("F") > col("M")) \
     .show()
```

**64. Identify "Salary Compression": Current employees hired before 1990 earning less than avg of those hired after 2000.**
*Concept: Complex Correlated Subquery*

```sql
-- SQL
SELECT e.id, e.hire_date, s.amount
FROM employee e
JOIN salary s ON e.id = s.employee_id AND s.to_date = '9999-01-01'
WHERE e.hire_date < '1990-01-01'
AND s.amount < (
    SELECT AVG(s2.amount)
    FROM salary s2
    JOIN employee e2 ON s2.employee_id = e2.id
    WHERE e2.hire_date > '2000-01-01' AND s2.to_date = '9999-01-01'
);
```

```python
# PySpark
old_hires_avg = salary_df.join(employee_df, salary_df.employee_id == employee_df.id) \
                        .filter((employee_df.hire_date > '2000-01-01') & (salary_df.to_date == '9999-01-01')) \
                        .agg(avg("amount")).collect()[0][0]

salary_df.join(employee_df, "employee_id") \
         .filter((col("hire_date") < '1990-01-01') & (col("to_date") == '9999-01-01')) \
         .filter(col("amount") < old_hires_avg) \
         .show()
```

**65. Rank salary within department and calculate the percentile (0 to 1).**
*Concept: Percent_Rank*

```sql
-- SQL
SELECT e.id, d.dept_name, s.amount,
       PERCENT_RANK() OVER (PARTITION BY d.dept_name ORDER BY s.amount) as pct
FROM salary s
JOIN department_employee de ON s.employee_id = de.employee_id AND de.to_date = '9999-01-01'
JOIN department d ON de.department_id = d.id
JOIN employee e ON s.employee_id = e.id
WHERE s.to_date = '9999-01-01';
```

```python
# PySpark
cond = [salary_df.employee_id == department_employee_df.employee_id, salary_df.to_date == '9999-01-01',
        department_employee_df.to_date == '9999-01-01', department_employee_df.department_id == department_df.id]

joined = salary_df.join(department_employee_df, cond).join(department_df, "department_id")
window_spec = Window.partitionBy("dept_name").orderBy("amount")
joined.withColumn("pct", percent_rank().over(window_spec)).show()
```

**66. Find the 2nd highest salary in each department.**
*Concept: Dense Rank Filtering*

```sql
-- SQL
WITH Ranked AS (
    SELECT d.dept_name, s.amount,
           DENSE_RANK() OVER (PARTITION BY d.dept_name ORDER BY s.amount DESC) as rn
    FROM salary s
    JOIN department_employee de ON s.employee_id = de.employee_id AND de.to_date = '9999-01-01'
    JOIN department d ON de.department_id = d.id
    WHERE s.to_date = '9999-01-01'
)
SELECT * FROM Ranked WHERE rn = 2;
```

```python
# PySpark
# Same setup as #65
joined = salary_df.join(department_employee_df, cond).join(department_df, "department_id")
window_spec = Window.partitionBy("dept_name").orderBy(col("amount").desc())
joined.withColumn("rn", dense_rank().over(window_spec)).filter(col("rn") == 2).show()
```

**67. Find employees who have never had a salary change (Single entry in salary table).**
*Concept: Group By Having Count*

```sql
-- SQL
SELECT employee_id, COUNT(*) as cnt
FROM salary
GROUP BY employee_id
HAVING COUNT(*) = 1;
```

```python
# PySpark
salary_df.groupBy("employee_id").agg(count("*").alias("cnt")) \
        .filter(col("cnt") == 1) \
        .show()
```

**68. Find the most common first name in the company.**
*Concept: Mode Calculation*

```sql
-- SQL
SELECT first_name, COUNT(*) as cnt
FROM employee
GROUP BY first_name
ORDER BY cnt DESC
LIMIT 1;
```

```python
# PySpark
employee_df.groupBy("first_name").count() \
            .orderBy(col("count").desc()) \
            .limit(1) \
            .show()
```

**69. Calculate average time (in days) between salary changes for each employee.**
*Concept: Self Join Date Diff*

```sql
-- SQL
SELECT s1.employee_id,
       AVG(DATEDIFF(s2.from_date, s1.from_date)) as avg_days_between_raises
FROM salary s1
JOIN salary s2 ON s1.employee_id = s2.employee_id AND s2.from_date > s1.from_date
GROUP BY s1.employee_id;
```

```python
# PySpark
# Lag approach is easier
window_spec = Window.partitionBy("employee_id").orderBy("from_date")
salary_df.withColumn("next_date", lead("from_date").over(window_spec)) \
        .withColumn("diff", datediff(col("next_date"), col("from_date"))) \
        .groupBy("employee_id") \
        .avg("diff") \
        .show()
```

**70. Find employees whose salary has decreased (Self Join).**
*Concept: Detecting anomalies*

```sql
-- SQL
SELECT DISTINCT s1.employee_id
FROM salary s1
JOIN salary s2 ON s1.employee_id = s2.employee_id 
    AND s2.from_date > s1.from_date
    AND s2.amount < s1.amount;
```

```python
# PySpark
s1 = salary_df.alias("s1")
s2 = salary_df.alias("s2")

s1.join(s2, (col("s1.employee_id") == col("s2.employee_id")) & 
           (col("s2.from_date") > col("s1.from_date")) & 
           (col("s2.amount") < col("s1.amount"))) \
   .select(col("s1.employee_id")) \
   .distinct() \
   .show()
```

---

### Group 9: Department & Title Dynamics (71-80)

**71. Find the department with the highest number of unique job titles.**
*Concept: Count Distinct Aggregation*

```sql
-- SQL
SELECT d.dept_name, COUNT(DISTINCT t.title) as unique_titles
FROM department d
JOIN department_employee de ON d.id = de.department_id
JOIN title t ON de.employee_id = t.employee_id
GROUP BY d.dept_name
ORDER BY unique_titles DESC
LIMIT 1;
```

```python
# PySpark
joined = department_df.join(department_employee_df, department_df.id == department_employee_df.department_id) \
                      .join(title_df, department_employee_df.employee_id == title_df.employee_id)

joined.groupBy("dept_name") \
      .agg(countDistinct("title").alias("unique_titles")) \
      .orderBy(col("unique_titles").desc()) \
      .limit(1) \
      .show()
```

**72. Identify departments that shrank in headcount between 1999 and 2000.**
*Concept: Time Slice Comparison*

```sql
-- SQL
WITH Count1999 AS (
    SELECT d.id, COUNT(*) as c99
    FROM department d
    JOIN department_employee de ON d.id = de.department_id
    WHERE de.from_date <= '1999-12-31' AND de.to_date >= '1999-01-01'
    GROUP BY d.id
),
Count2000 AS (
    SELECT d.id, COUNT(*) as c00
    FROM department d
    JOIN department_employee de ON d.id = de.department_id
    WHERE de.from_date <= '2000-12-31' AND de.to_date >= '2000-01-01'
    GROUP BY d.id
)
SELECT c99.id FROM Count1999 
JOIN Count2000 ON c99.id = Count2000.id
WHERE c99.c99 > Count2000.c00;
```

```python
# PySpark
# Helper function to get count for a year
def get_count(year):
    cond = [(department_df.id == department_employee_df.department_id),
            (department_employee_df.from_date <= lit(f'{year}-12-31')),
            (department_employee_df.to_date >= lit(f'{year}-01-01'))]
    return department_df.join(department_employee_df, cond) \
                       .groupBy(department_df.id) \
                       .agg(count("*").alias(f"cnt{year}"))

c99 = get_count(1999)
c00 = get_count(2000)

c99.join(c00, "id").filter(col("cnt1999") > col("cnt2000")).show()
```

**73. Find the most common "Next Title" after "Staff".**
*Concept: Lead + Group By*

```sql
-- SQL
WITH NextTitle AS (
    SELECT employee_id, title,
           LEAD(title) OVER (PARTITION BY employee_id ORDER BY from_date) as next_title
    FROM title
)
SELECT next_title, COUNT(*) as cnt
FROM NextTitle
WHERE title = 'Staff' AND next_title IS NOT NULL
GROUP BY next_title
ORDER BY cnt DESC;
```

```python
# PySpark
window_spec = Window.partitionBy("employee_id").orderBy("from_date")
next_title = title_df.withColumn("next_title", lead("title").over(window_spec))

next_title.filter(col("title") == "Staff") \
          .filter(col("next_title").isNotNull()) \
          .groupBy("next_title") \
          .count() \
          .orderBy(col("count").desc()) \
          .show()
```

**74. Count how many titles each employee has held in their career.**
*Concept: Group By*

```sql
-- SQL
SELECT employee_id, COUNT(*) as title_count
FROM title
GROUP BY employee_id
ORDER BY title_count DESC;
```

```python
# PySpark
title_df.groupBy("employee_id").count().orderBy(col("count").desc()).show()
```

**75. Find employees who held a "Manager" title but are not currently managers.**
*Concept: Anti-Join on History*

```sql
-- SQL
SELECT DISTINCT e.id, e.first_name
FROM employee e
JOIN title t ON e.id = t.employee_id
WHERE t.title LIKE '%Manager%'
AND e.id NOT IN (
    SELECT dm.employee_id 
    FROM department_manager dm 
    WHERE dm.to_date = '9999-01-01'
);
```

```python
# PySpark
current_mgrs = department_manager_df.filter(col("to_date") == '9999-01-01').select("employee_id")

mgrs_history = title_df.filter(col("title").contains("Manager")).select("employee_id").distinct()

mgrs_history.join(current_mgrs, "employee_id", "left_anti") \
             .join(employee_df, "employee_id") \
             .show()
```

**76. Calculate the average tenure (in days) per department.**
*Concept: Average Date Diff*

```sql
-- SQL
SELECT d.dept_name, AVG(DATEDIFF(de.to_date, de.from_date)) as avg_tenure_days
FROM department_employee de
JOIN department d ON de.department_id = d.id
GROUP BY d.dept_name;
```

```python
# PySpark
joined = department_employee_df.join(department_df, department_employee_df.department_id == department_df.id)
joined.withColumn("tenure", datediff(col("to_date"), col("from_date"))) \
      .groupBy("dept_name") \
      .avg("tenure") \
      .show()
```

**77. List all departments and the number of current employees, ensuring empty departments are listed.**
*Concept: Left Join*

```sql
-- SQL
SELECT d.dept_name, COUNT(de.employee_id) as current_emp_count
FROM department d
LEFT JOIN department_employee de ON d.id = de.department_id AND de.to_date = '9999-01-01'
GROUP BY d.dept_name;
```

```python
# PySpark
department_df.join(department_employee_df, (department_df.id == department_employee_df.department_id) & (col("to_date") == '9999-01-01'), "left") \
              .groupBy("dept_name") \
              .count() \
              .show()
```

**78. Find departments with a higher average salary than the company average.**
*Concept: Having > Global Avg*

```sql
-- SQL
SELECT d.dept_name, AVG(s.amount) as avg_sal
FROM salary s
JOIN department_employee de ON s.employee_id = de.employee_id AND de.to_date = '9999-01-01'
JOIN department d ON de.department_id = d.id
WHERE s.to_date = '9999-01-01'
GROUP BY d.dept_name
HAVING AVG(s.amount) > (SELECT AVG(amount) FROM salary WHERE to_date = '9999-01-01');
```

```python
# PySpark
global_avg = salary_df.filter(col("to_date") == '9999-01-01').agg(avg("amount")).collect()[0][0]

cond = [salary_df.employee_id == department_employee_df.employee_id, salary_df.to_date == '9999-01-01',
        department_employee_df.to_date == '9999-01-01', department_employee_df.department_id == department_df.id]

joined = salary_df.join(department_employee_df, cond).join(department_df, "department_id")
joined.groupBy("dept_name") \
      .agg(avg("amount").alias("avg_sal")) \
      .filter(col("avg_sal") > global_avg) \
      .show()
```

**79. Find the "Golden Handcuffs": Employees with >10 years tenure but below average salary.**
*Concept: Complex Filtering*

```sql
-- SQL
SELECT e.id, e.hire_date, s.amount
FROM employee e
JOIN salary s ON e.id = s.employee_id AND s.to_date = '9999-01-01'
WHERE DATEDIFF(CURRENT_DATE, e.hire_date) > 3650
AND s.amount < (SELECT AVG(amount) FROM salary WHERE to_date = '9999-01-01');
```

```python
# PySpark
avg_sal = salary_df.filter(col("to_date") == '9999-01-01').agg(avg("amount")).collect()[0][0]

salary_df.join(employee_df, "employee_id") \
        .filter((datediff(current_date(), col("hire_date")) > 3650) & (col("to_date") == '9999-01-01')) \
        .filter(col("amount") < avg_sal) \
        .show()
```

**80. Find departments where the headcount doubled (or more) since 1990.**
*Concept: Time Slice Math*

```sql
-- SQL
WITH Counts AS (
    SELECT d.id, 
           SUM(CASE WHEN de.from_date <= '1990-12-31' THEN 1 ELSE 0 END) as cnt_1990,
           SUM(CASE WHEN de.to_date = '9999-01-01' THEN 1 ELSE 0 END) as cnt_now
    FROM department d
    JOIN department_employee de ON d.id = de.department_id
    GROUP BY d.id
)
SELECT id FROM Counts WHERE cnt_now >= cnt_1990 * 2;
```

```python
# PySpark
# This is verbose in PySpark without SQL, but doable via pivot or conditional sum
joined = department_df.join(department_employee_df, department_df.id == department_employee_df.department_id)

joined.agg(
    sum(when(col("from_date") <= '1990-12-31', 1).otherwise(0)).alias("c90"),
    sum(when(col("to_date") == '9999-01-01', 1).otherwise(0)).alias("cNow")
).filter(col("cNow") >= col("c90") * 2).show()
```

---

### Group 10: Data Hygiene & Advanced Logic (81-90)

**81. Find duplicate records in the `department_employee` table (Same emp, dept, dates).**
*Concept: Group By Having Count > 1*

```sql
-- SQL
SELECT employee_id, department_id, from_date, to_date, COUNT(*) as dup
FROM department_employee
GROUP BY employee_id, department_id, from_date, to_date
HAVING COUNT(*) > 1;
```

```python
# PySpark
department_employee_df.groupBy("employee_id", "department_id", "from_date", "to_date") \
                      .count() \
                      .filter(col("count") > 1) \
                      .show()
```

**82. Find employees with NULL gender (Data Quality check).**
*Concept: Is Null*

```sql
-- SQL
SELECT * FROM employee WHERE gender IS NULL;
```

```python
# PySpark
employee_df.filter(col("gender").isNull()).show()
```

**83. Convert all gender codes to Full Names ('M' -> 'Male').**
*Concept: When/Else*

```sql
-- SQL
SELECT id, first_name,
       CASE WHEN gender = 'M' THEN 'Male'
            WHEN gender = 'F' THEN 'Female'
            ELSE 'Other' END as full_gender
FROM employee;
```

```python
# PySpark
employee_df.withColumn("full_gender", 
               when(col("gender") == "M", "Male")
               .when(col("gender") == "F", "Female")
               .otherwise("Other")) \
           .show()
```

**84. Find employees who have 'to_date' in the past (Left the company).**
*Concept: Date Compare*

```sql
-- SQL
SELECT e.id, e.first_name, de.to_date
FROM employee e
JOIN department_employee de ON e.id = de.employee_id
WHERE de.to_date < CURRENT_DATE;
```

```python
# PySpark
department_employee_df.join(employee_df, department_employee_df.employee_id == employee_df.id) \
                      .filter(col("to_date") < current_date()) \
                      .show()
```

**85. Standardize 'Department' names to Uppercase.**
*Concept: Upper*

```sql
-- SQL
SELECT id, UPPER(dept_name) as dept_upper
FROM department;
```

```python
# PySpark
department_df.withColumn("dept_upper", upper(col("dept_name"))).show()
```

**86. Extract the Year of hire and Month of hire into separate columns.**
*Concept: Extract Year/Month*

```sql
-- SQL
SELECT id, hire_date, 
       EXTRACT(YEAR FROM hire_date) as hire_year,
       EXTRACT(MONTH FROM hire_date) as hire_month
FROM employee;
```

```python
# PySpark
employee_df.withColumn("hire_year", year(col("hire_date"))) \
           .withColumn("hire_month", month(col("hire_date"))) \
           .show()
```

**87. Find the most frequent starting salary among all employees.**
*Concept: Mode on Amount*

```sql
-- SQL
SELECT amount, COUNT(*) as freq
FROM salary
GROUP BY amount
ORDER BY freq DESC
LIMIT 1;
```

```python
# PySpark
salary_df.groupBy("amount").count().orderBy(col("count").desc()).limit(1).show()
```

**88. Check for data inconsistency: Employee in `title` table but not in `employee` table.**
*Concept: Left Anti Join (FK Violation)*

```sql
-- SQL
SELECT employee_id FROM title
WHERE employee_id NOT IN (SELECT id FROM employee);
```

```python
# PySpark
title_df.join(employee_df, title_df.employee_id == employee_df.id, "left_anti") \
        .select("employee_id") \
        .show()
```

**89. Truncate the `hire_date` to the first of the year.**
*Concept: Trunc*

```sql
-- SQL
SELECT id, DATE_TRUNC('year', hire_date) as hire_year_start
FROM employee;
```

```python
# PySpark
employee_df.withColumn("hire_year_start", trunc(col("hire_date"), "year")).show()
```

**90. Calculate the time difference between joining a department and becoming a manager (if applicable).**
*Concept: Self Join Date Calculation*

```sql
-- SQL
SELECT de.employee_id,
       DATEDIFF(dm.from_date, de.from_date) as days_to_mgr
FROM department_employee de
JOIN department_manager dm ON de.employee_id = dm.employee_id AND de.department_id = dm.department_id
WHERE dm.from_date > de.from_date;
```

```python
# PySpark
cond = [department_employee_df.employee_id == department_manager_df.employee_id,
        department_employee_df.department_id == department_manager_df.department_id,
        department_manager_df.from_date > department_employee_df.from_date]

department_employee_df.join(department_manager_df, cond) \
                      .withColumn("days_to_mgr", datediff(col("from_date"), col("from_date"))) \
                      .show()
```

---

### Group 11: PySpark Optimization & Utility (91-100)

**91. Repartition the employee DataFrame by department_id to optimize joins.**
*Concept: Repartition*

```python
# PySpark
# Assuming we have department_id joined on
joined = employee_df.join(department_employee_df, employee_df.id == department_employee_df.employee_id)
# Repartition by department_id to ensure all employees of a dept are in same partition
joined.repartition(10, "department_id")
```

**92. Cache the salary table as it is used frequently.**
*Concept: Cache/Persist*

```python
# PySpark
salary_df.cache()
# Perform actions...
salary_df.count() 
```

**93. Sample 10% of the employees to create a test dataset.**
*Concept: Sample*

```python
# PySpark
sample_df = employee_df.sample(withReplacement=False, fraction=0.1)
sample_df.show()
```

**94. Create a view from the DataFrame for temporary SQL usage.**
*Concept: CreateOrReplaceTempView*

```python
# PySpark
employee_df.createOrReplaceTempView("emp_view")
# (This allows using spark.sql, though prompts asked to avoid it, creating views is a valid DataFrame API method).
```

**95. Calculate approximate count of distinct salaries (HyperLogLog).**
*Concept: ApproxCountDistinct*

```python
# PySpark
salary_df.select(approxCountDistinct("amount")).show()
```

**96. Sort the entire DataFrame globally by salary (Expensive operation).**
*Concept: Sort*

```python
# PySpark
salary_df.orderBy(col("amount").desc()).show()
```

**97. Drop duplicate salary records for an employee (keeping the latest).**
*Concept: DropDuplicates*

```python
# PySpark
window_spec = Window.partitionBy("employee_id").orderBy(col("from_date").desc())
latest_salaries = salary_df.withColumn("rn", row_number().over(window_spec)) \
                          .filter(col("rn") == 1) \
                          .drop("rn")
latest_salaries.show()
```

**98. Fill NULL values in `to_date` with current date (hypothetical scenario).**
*Concept: FillNa*

```python
# PySpark
# Assuming some to_dates are null (not the case in this DB strictly, but good practice)
cleaned_df = department_employee_df.fillna({'to_date': '2023-01-01'})
```

**99. Check the schema of the DataFrame programmatically.**
*Concept: PrintSchema*

```python
# PySpark
employee_df.printSchema()
```

**100. Calculate the total number of partitions in the employee DataFrame.**
*Concept: RDD manipulation*

```python
# PySpark
print(employee_df.rdd.getNumPartitions())
```
