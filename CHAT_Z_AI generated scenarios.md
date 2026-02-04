

Here is the complete consolidated list of **50 Sample Questions and Solutions** for the MySQL Employees Sample Database.

These cover:
1.  **Joins & Filtering**
2.  **Window Functions (Ranking, Lead, Lag, Ntile)**
3.  **Sliding Windows (Moving Averages, Running Totals, Row Frames)**
4.  **Pivots & Aggregations**
5.  **Self Joins & Complex Logic**
6.  **Recursive / Time Series Logic**

**PySpark Setup**
```python
from pyspark.sql import Window
from pyspark.sql.functions import *
```

---

### Group 1: Joins, Filtering & Basic Aggregates (1-10)

**1. List all current employees with their department names.**
*Concept: Inner Join, Current Date Filter*

```sql
-- SQL
SELECT e.emp_no, e.first_name, e.last_name, d.dept_name
FROM employees e
JOIN dept_emp de ON e.emp_no = de.emp_no AND de.to_date = '9999-01-01'
JOIN departments d ON de.dept_no = d.dept_no;
```

```python
# PySpark
cond = [employees_df.emp_no == dept_emp_df.emp_no, 
        dept_emp_df.to_date == '9999-01-01', 
        dept_emp_df.dept_no == departments_df.dept_no]

employees_df.join(dept_emp_df, "emp_no") \
            .join(departments_df, cond) \
            .select(employees_df.emp_no, employees_df.first_name, 
                    employees_df.last_name, departments_df.dept_name) \
            .show()
```

**2. Find the current head of each department.**
*Concept: Inner Join, Manager Table*

```sql
-- SQL
SELECT d.dept_name, e.first_name, e.last_name
FROM dept_manager dm
JOIN employees e ON dm.emp_no = e.emp_no
JOIN departments d ON dm.dept_no = d.dept_no
WHERE dm.to_date = '9999-01-01';
```

```python
# PySpark
managers = dept_manager_df.filter(col("to_date") == '9999-01-01')
managers.join(employees_df, "emp_no") \
        .join(departments_df, "dept_no") \
        .select(departments_df.dept_name, employees_df.first_name, employees_df.last_name) \
        .show()
```

**3. List employees who are currently "Senior Engineers".**
*Concept: Multi-table Join*

```sql
-- SQL
SELECT e.emp_no, e.first_name
FROM employees e
JOIN titles t ON e.emp_no = t.emp_no
WHERE t.title = 'Senior Engineer' AND t.to_date = '9999-01-01';
```

```python
# PySpark
employees_df.join(titles_df, "emp_no") \
            .filter((col("title") == "Senior Engineer") & (col("to_date") == '9999-01-01')) \
            .select("emp_no", "first_name") \
            .show()
```

**4. Calculate the average salary per department.**
*Concept: Aggregation*

```sql
-- SQL
SELECT d.dept_name, AVG(s.salary) as avg_salary
FROM salaries s
JOIN dept_emp de ON s.emp_no = de.emp_no AND de.to_date = '9999-01-01' AND s.to_date = '9999-01-01'
JOIN departments d ON de.dept_no = d.dept_no
GROUP BY d.dept_name;
```

```python
# PySpark
cond = [salaries_df.emp_no == dept_emp_df.emp_no, 
        salaries_df.to_date == '9999-01-01', 
        dept_emp_df.to_date == '9999-01-01',
        dept_emp_df.dept_no == departments_df.dept_no]

joined = salaries_df.join(dept_emp_df, cond).join(departments_df, "dept_no")
joined.groupBy("dept_name").avg("salary").show()
```

**5. Find departments with more than 10,000 current employees.**
*Concept: Having Clause*

```sql
-- SQL
SELECT d.dept_name, COUNT(*) as cnt
FROM dept_emp de
JOIN departments d ON de.dept_no = d.dept_no
WHERE de.to_date = '9999-01-01'
GROUP BY d.dept_name
HAVING COUNT(*) > 10000;
```

```python
# PySpark
dept_emp_df.filter(col("to_date") == '9999-01-01') \
           .join(departments_df, "dept_no") \
           .groupBy("dept_name") \
           .agg(count("*").alias("cnt")) \
           .filter(col("cnt") > 10000) \
           .show()
```

**6. Find employees hired in the year 1995.**
*Concept: Date Function*

```sql
-- SQL
SELECT * FROM employees WHERE YEAR(hire_date) = 1995;
```

```python
# PySpark
employees_df.filter(year("hire_date") == 1995).show()
```

**7. Count how many distinct job titles exist.**
*Concept: Distinct Count*

```sql
-- SQL
SELECT COUNT(DISTINCT title) FROM titles;
```

```python
# PySpark
titles_df.select(countDistinct("title")).show()
```

**8. List employees who have never held a manager title.**
*Concept: Left Join (Anti-Join)*

```sql
-- SQL
SELECT e.first_name, e.last_name
FROM employees e
WHERE NOT EXISTS (
    SELECT 1 FROM titles t WHERE t.emp_no = e.emp_no AND t.title LIKE '%Manager%'
);
```

```python
# PySpark
mgrs = titles_df.filter(col("title").contains("Manager")).select("emp_no").distinct()
employees_df.join(mgrs, "emp_no", "left_anti") \
            .select("first_name", "last_name") \
            .show()
```

**9. Show the salary history for employee 10001.**
*Concept: Simple Filter*

```sql
-- SQL
SELECT * FROM salaries WHERE emp_no = 10001 ORDER BY from_date;
```

```python
# PySpark
salaries_df.filter(col("emp_no") == 10001).orderBy("from_date").show()
```

**10. Find the maximum salary ever paid.**
*Concept: Max*

```sql
-- SQL
SELECT MAX(salary) FROM salaries;
```

```python
# PySpark
salaries_df.agg(max("salary")).show()
```

---

### Group 2: Window Functions - Ranking & Offsets (11-20)

**11. Find the top 3 earners in each department.**
*Concept: `row_number`*

```sql
-- SQL
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

```python
# PySpark
cond = [salaries_df.emp_no == dept_emp_df.emp_no, 
        salaries_df.to_date == '9999-01-01', 
        dept_emp_df.to_date == '9999-01-01',
        dept_emp_df.dept_no == departments_df.dept_no]

joined = salaries_df.join(dept_emp_df, "emp_no").join(departments_df, cond).join(employees_df, "emp_no")
window_spec = Window.partitionBy("dept_name").orderBy(col("salary").desc())

joined.withColumn("rn", row_number().over(window_spec)) \
      .filter(col("rn") <= 3) \
      .show()
```

**12. Rank employees by salary within their title.**
*Concept: `dense_rank`*

```sql
-- SQL
SELECT t.title, e.first_name, s.salary,
       DENSE_RANK() OVER (PARTITION BY t.title ORDER BY s.salary DESC) as rnk
FROM employees e
JOIN salaries s ON e.emp_no = s.emp_no AND s.to_date = '9999-01-01'
JOIN titles t ON e.emp_no = t.emp_no AND t.to_date = '9999-01-01';
```

```python
# PySpark
cond = [employees_df.emp_no == salaries_df.emp_no, salaries_df.to_date == '9999-01-01',
        employees_df.emp_no == titles_df.emp_no, titles_df.to_date == '9999-01-01']
joined = employees_df.join(salaries_df, "emp_no").join(titles_df, "emp_no")

window_spec = Window.partitionBy("title").orderBy(col("salary").desc())
joined.withColumn("rnk", dense_rank().over(window_spec)).show()
```

**13. Compare current salary with the previous salary (Raise amount).**
*Concept: `lag`*

```sql
-- SQL
SELECT emp_no, salary,
       LAG(salary) OVER (PARTITION BY emp_no ORDER BY from_date) as prev_salary,
       salary - LAG(salary) OVER (PARTITION BY emp_no ORDER BY from_date) as raise_amount
FROM salaries;
```

```python
# PySpark
window_spec = Window.partitionBy("emp_no").orderBy("from_date")
salaries_df.withColumn("prev_salary", lag("salary").over(window_spec)) \
           .withColumn("raise", col("salary") - col("prev_salary")) \
           .show()
```

**14. Find the date of the next job title change.**
*Concept: `lead`*

```sql
-- SQL
SELECT emp_no, title, from_date,
       LEAD(from_date) OVER (PARTITION BY emp_no ORDER BY from_date) as next_change_date
FROM titles;
```

```python
# PySpark
window_spec = Window.partitionBy("emp_no").orderBy("from_date")
titles_df.withColumn("next_change_date", lead("from_date").over(window_spec)).show()
```

**15. Calculate salary quartiles (NTILE).**
*Concept: `ntile`*

```sql
-- SQL
SELECT emp_no, salary,
       NTILE(4) OVER (ORDER BY salary DESC) as quartile
FROM salaries WHERE to_date = '9999-01-01';
```

```python
# PySpark
current_salaries = salaries_df.filter(col("to_date") == '9999-01-01')
current_salaries.withColumn("quartile", ntile(4).over(Window.orderBy(col("salary").desc()))).show()
```

**16. Find the percentage rank of a salary within the company.**
*Concept: `percent_rank`*

```sql
-- SQL
SELECT emp_no, salary,
       PERCENT_RANK() OVER (ORDER BY salary ASC) as pct_rank
FROM salaries WHERE to_date = '9999-01-01';
```

```python
# PySpark
current_salaries = salaries_df.filter(col("to_date") == '9999-01-01')
current_salaries.withColumn("pct_rank", percent_rank().over(Window.orderBy("salary"))).show()
```

**17. Count the number of employees hired in the same year as each employee.**
*Concept: `count` Window*

```sql
-- SQL
SELECT emp_no, hire_date,
       COUNT(*) OVER (PARTITION BY YEAR(hire_date)) as peers_hired_same_year
FROM employees;
```

```python
# PySpark
window_spec = Window.partitionBy(year("hire_date"))
employees_df.withColumn("peers", count("*").over(window_spec)).show()
```

**18. Find the first hire date for every department.**
*Concept: `first_value`*

```sql
-- SQL
SELECT DISTINCT d.dept_name,
       FIRST_VALUE(e.hire_date) OVER (PARTITION BY d.dept_no ORDER BY e.hire_date) as first_hire
FROM employees e
JOIN dept_emp de ON e.emp_no = de.emp_no
JOIN departments d ON de.dept_no = d.dept_no;
```

```python
# PySpark
cond = [employees_df.emp_no == dept_emp_df.emp_no, dept_emp_df.dept_no == departments_df.dept_no]
joined = employees_df.join(dept_emp_df, "emp_no").join(departments_df, cond)

window_spec = Window.partitionBy("dept_no").orderBy("hire_date")
joined.withColumn("first_hire", first("hire_date").over(window_spec)) \
      .select("dept_name", "first_hire") \
      .distinct() \
      .show()
```

**19. Find employees with duplicate titles.**
*Concept: Logic with Window*

```sql
-- SQL
WITH CountTitles AS (
    SELECT emp_no, title, COUNT(*) OVER (PARTITION BY emp_no, title) as cnt
    FROM titles
)
SELECT DISTINCT emp_no FROM CountTitles WHERE cnt > 1;
```

```python
# PySpark
window_spec = Window.partitionBy("emp_no", "title")
titles_df.withColumn("cnt", count("*").over(window_spec)) \
        .filter(col("cnt") > 1) \
        .select("emp_no") \
        .distinct() \
        .show()
```

**20. Median salary approximation.**
*Concept: Logic using `percent_rank` or `row_number`*

```sql
-- SQL
WITH Ordered AS (
    SELECT salary,
           ROW_NUMBER() OVER (ORDER BY salary) as rn,
           COUNT(*) OVER () as total
    FROM salaries WHERE to_date = '9999-01-01'
)
SELECT AVG(salary) FROM Ordered WHERE rn IN (FLOOR((total+1)/2), CEIL((total+1)/2));
```

```python
# PySpark
# Spark has a built-in percentile_approx function in aggregations
salaries_df.filter(col("to_date") == '9999-01-01') \
           .agg(expr("percentile_approx(salary, 0.5)").alias("median")) \
           .show()
```

---

### Group 3: Sliding Windows (Frames) (21-25)

**21. 3-Month Moving Average of Salary changes (Time Series).**
*Concept: `rowsBetween`*

```sql
-- SQL
SELECT 
    from_date,
    salary,
    AVG(salary) OVER (
        ORDER BY from_date 
        ROWS BETWEEN 2 PRECEDING AND CURRENT ROW
    ) as moving_avg
FROM salaries
WHERE emp_no = 10001;
```

```python
# PySpark
window_spec = Window.partitionBy("emp_no") \
                   .orderBy("from_date") \
                   .rowsBetween(-2, 0)

salaries_df.filter(col("emp_no") == 10001) \
           .withColumn("moving_avg", avg("salary").over(window_spec)) \
           .show()
```

**22. Running Total of salaries paid globally by date.**
*Concept: `rangeBetween` (Unbounded)*

```sql
-- SQL
SELECT 
    from_date,
    SUM(salary) OVER (
        ORDER BY from_date 
        RANGE BETWEEN UNBOUNDED PRECEDING AND CURRENT ROW
    ) as running_total
FROM salaries
GROUP BY from_date
ORDER BY from_date;
```

```python
# PySpark
daily_cost = salaries_df.groupBy("from_date").sum("salary").withColumnRenamed("sum(salary)", "daily_cost")
window_spec = Window.orderBy("from_date").rangeBetween(Window.unboundedPreceding, Window.currentRow)

daily_cost.withColumn("running_total", sum("daily_cost").over(window_spec)).show()
```

**23. Sum of Current Salary + Previous 2 Salaries (Centered Window).**
*Concept: `rowsBetween` (1 Preceding, 1 Following)*

```sql
-- SQL
SELECT 
    from_date,
    salary,
    SUM(salary) OVER (
        ORDER BY from_date 
        ROWS BETWEEN 1 PRECEDING AND 1 FOLLOWING
    ) as sum_neighbors
FROM salaries
WHERE emp_no = 10001;
```

```python
# PySpark
window_spec = Window.partitionBy("emp_no").orderBy("from_date").rowsBetween(-1, 1)

salaries_df.filter(col("emp_no") == 10001) \
           .withColumn("sum_neighbors", sum("salary").over(window_spec)) \
           .show()
```

**24. Department Average Salary excluding the current row.**
*Concept: Window Filter Frame*

```sql
-- SQL
SELECT 
    e.emp_no, d.dept_name, s.salary,
    AVG(s.salary) OVER (
        PARTITION BY d.dept_no 
        ORDER BY s.salary 
        ROWS BETWEEN UNBOUNDED PRECEDING AND 1 PRECEDING
    ) as avg_lower_salaries
FROM salaries s
JOIN dept_emp de ON s.emp_no = de.emp_no AND de.to_date = '9999-01-01'
JOIN departments d ON de.dept_no = d.dept_no
WHERE s.to_date = '9999-01-01';
```

```python
# PySpark
# Note: This is a specific "frame" where we look at everything before the current row
cond = [salaries_df.emp_no == dept_emp_df.emp_no, salaries_df.to_date == '9999-01-01',
        dept_emp_df.dept_no == departments_df.dept_no, dept_emp_df.to_date == '9999-01-01']

joined = salaries_df.join(dept_emp_df, "emp_no").join(departments_df, cond)
window_spec = Window.partitionBy("dept_no").orderBy("salary").rowsBetween(Window.unboundedPreceding, -1)

joined.withColumn("avg_lower", avg("salary").over(window_spec)).show()
```

**25. Rolling 12-month total hires.**
*Concept: Time-based Window (requires Date math)*

```sql
-- SQL
SELECT 
    hire_date,
    COUNT(*) OVER (
        ORDER BY hire_date
        RANGE BETWEEN INTERVAL 12 MONTH PRECEDING AND CURRENT ROW
    ) as rolling_hires
FROM employees;
```

```python
# PySpark
# Spark doesn't support SQL standard RANGE BETWEEN INTERVAL for dates in the DataFrame Window API as directly as SQL.
# We usually convert to timestamp, but here is the closest logic:
# Calculate month, and group by month for aggregation is usually better for monthly rollups.
# Alternative: Using rangeBetween with timestamps.

# Assuming timestamps for rangeBetween
emp_ts = employees_df.withColumn("ts", to_timestamp(col("hire_date")))
window_spec = Window.orderBy("ts").rangeBetween(-31536000, 0) # Approx 1 year in seconds

emp_ts.withColumn("rolling_hires", count("*").over(window_spec)).show()
```

---

### Group 4: Pivots & Complex Aggregations (26-30)

**26. Count of Employees by Gender in each Department (Pivot).**
*Concept: `pivot`*

```sql
-- SQL
SELECT * FROM (
    SELECT d.dept_name, e.gender
    FROM employees e
    JOIN dept_emp de ON e.emp_no = de.emp_no AND de.to_date = '9999-01-01'
    JOIN departments d ON de.dept_no = d.dept_no
) 
PIVOT (
    COUNT(*) FOR gender IN ('M', 'F')
);
```

```python
# PySpark
cond = [employees_df.emp_no == dept_emp_df.emp_no, dept_emp_df.to_date == '9999-01-01',
        dept_emp_df.dept_no == departments_df.dept_no]

joined = employees_df.join(dept_emp_df, "emp_no").join(departments_df, cond)
joined.groupBy("dept_name").pivot("gender", ["M", "F"]).count().show()
```

**27. Average Salary by Department and Title (Pivot).**
*Concept: `pivot` with Aggregation*

```sql
-- SQL
SELECT * FROM (
    SELECT d.dept_name, t.title, s.salary
    FROM salaries s
    JOIN titles t ON s.emp_no = t.emp_no AND t.to_date = '9999-01-01'
    JOIN dept_emp de ON s.emp_no = de.emp_no AND de.to_date = '9999-01-01'
    JOIN departments d ON de.dept_no = d.dept_no
    WHERE s.to_date = '9999-01-01'
) 
PIVOT (
    AVG(salary) FOR title IN ('Staff', 'Senior Engineer', 'Manager')
);
```

```python
# PySpark
cond = [salaries_df.emp_no == titles_df.emp_no, titles_df.to_date == '9999-01-01',
        salaries_df.emp_no == dept_emp_df.emp_no, dept_emp_df.to_date == '9999-01-01',
        dept_emp_df.dept_no == departments_df.dept_no,
        salaries_df.to_date == '9999-01-01']

joined = salaries_df.join(titles_df, "emp_no").join(dept_emp_df, "emp_no").join(departments_df, "dept_no")
joined.groupBy("dept_name") \
      .pivot("title", ["Staff", "Senior Engineer", "Manager"]) \
      .avg("salary") \
      .show()
```

**28. List all employees in a department as a comma-separated string.**
*Concept: `collect_list` / `group_concat`*

```sql
-- SQL (MySQL uses GROUP_CONCAT)
SELECT d.dept_name, GROUP_CONCAT(e.first_name) as employees
FROM departments d
JOIN dept_emp de ON d.dept_no = de.dept_no AND de.to_date = '9999-01-01'
JOIN employees e ON de.emp_no = e.emp_no
GROUP BY d.dept_name;
```

```python
# PySpark
# Spark uses collect_list, then concat_ws to join strings
cond = [departments_df.dept_no == dept_emp_df.dept_no, dept_emp_df.to_date == '9999-01-01',
        dept_emp_df.emp_no == employees_df.emp_no]

joined = departments_df.join(dept_emp_df, cond).join(employees_df, "emp_no")
result = joined.groupBy("dept_name") \
               .agg(concat_ws(", ", collect_list("first_name")).alias("employees"))
result.show(truncate=False)
```

**29. Histogram of Salary Ranges.**
*Concept: Bucketing*

```sql
-- SQL
SELECT 
    CASE 
        WHEN salary < 50000 THEN 'Low'
        WHEN salary < 80000 THEN 'Medium'
        ELSE 'High'
    END as range,
    COUNT(*)
FROM salaries
WHERE to_date = '9999-01-01'
GROUP BY range;
```

```python
# PySpark
current_salaries = salaries_df.filter(col("to_date") == '9999-01-01')
current_salaries.withColumn("range", 
                   when(col("salary") < 50000, "Low")
                   .when(col("salary") < 80000, "Medium")
                   .otherwise("High")) \
               .groupBy("range").count() \
               .show()
```

**30. Most frequent title per department (Mode).**
*Concept: Window count aliasing*

```sql
-- SQL
WITH TitleCounts AS (
    SELECT d.dept_name, t.title, COUNT(*) as cnt
    FROM dept_emp de
    JOIN departments d ON de.dept_no = d.dept_no AND de.to_date = '9999-01-01'
    JOIN titles t ON de.emp_no = t.emp_no AND t.to_date = '9999-01-01'
    GROUP BY d.dept_name, t.title
),
Ranked AS (
    SELECT dept_name, title, cnt,
           ROW_NUMBER() OVER (PARTITION BY dept_name ORDER BY cnt DESC) as rn
    FROM TitleCounts
)
SELECT * FROM Ranked WHERE rn = 1;
```

```python
# PySpark
cond = [dept_emp_df.dept_no == departments_df.dept_no, dept_emp_df.to_date == '9999-01-01',
        dept_emp_df.emp_no == titles_df.emp_no, titles_df.to_date == '9999-01-01']

counts = dept_emp_df.join(departments_df, cond).join(titles_df, "emp_no") \
                    .groupBy(departments_df.dept_name, titles_df.title).count()

window_spec = Window.partitionBy("dept_name").orderBy(col("count").desc())
counts.withColumn("rn", row_number().over(window_spec)) \
      .filter(col("rn") == 1) \
      .show()
```

---

### Group 5: Self Joins & Comparisons (31-40)

**31. Employees who earn more than their current manager.**
*Concept: Complex Join*

```sql
-- SQL
WITH CurrentMgrs AS (
    SELECT dm.dept_no, s.salary as mgr_sal
    FROM dept_manager dm
    JOIN salaries s ON dm.emp_no = s.emp_no AND s.to_date = '9999-01-01'
    WHERE dm.to_date = '9999-01-01'
)
SELECT e.emp_no, d.dept_name, s.salary
FROM employees e
JOIN salaries s ON e.emp_no = s.emp_no AND s.to_date = '9999-01-01'
JOIN dept_emp de ON e.emp_no = de.emp_no AND de.to_date = '9999-01-01'
JOIN departments d ON de.dept_no = d.dept_no
JOIN CurrentMgrs cm ON d.dept_no = cm.dept_no
WHERE s.salary > cm.mgr_sal;
```

```python
# PySpark
mgr_cond = [dept_manager_df.emp_no == salaries_df.emp_no, salaries_df.to_date == '9999-01-01',
            dept_manager_df.to_date == '9999-01-01']
managers = dept_manager_df.join(salaries_df, mgr_cond) \
                         .select(dept_manager_df.dept_no, salaries_df.salary.alias("mgr_sal"))

emp_cond = [employees_df.emp_no == salaries_df.emp_no, salaries_df.to_date == '9999-01-01',
            employees_df.emp_no == dept_emp_df.emp_no, dept_emp_df.to_date == '9999-01-01',
            dept_emp_df.dept_no == departments_df.dept_no]

emps = employees_df.join(salaries_df, "emp_no").join(dept_emp_df, "emp_no").join(departments_df, "dept_no")

emps.join(managers, "dept_no") \
    .filter(col("salary") > col("mgr_sal")) \
    .select(employees_df.emp_no, departments_df.dept_name, salaries_df.salary) \
    .show()
```

**32. Pairs of employees sharing the same birthday.**
*Concept: Self Join with inequality*

```sql
-- SQL
SELECT 
    e1.emp_no, e1.first_name, 
    e2.emp_no, e2.first_name
FROM employees e1
JOIN employees e2 
    ON e1.birth_date = e2.birth_date 
    AND e1.emp_no < e2.emp_no;
```

```python
# PySpark
employees_df.alias("e1").join(employees_df.alias("e2"), 
    (col("e1.birth_date") == col("e2.birth_date")) & (col("e1.emp_no") < col("e2.emp_no"))) \
    .select(col("e1.emp_no"), col("e1.first_name"), col("e2.emp_no"), col("e2.first_name")) \
    .show()
```

**33. Identify employees who held multiple titles.**
*Concept: Self Join Count*

```sql
-- SQL
SELECT e.emp_no, e.first_name, COUNT(t.title) as title_count
FROM employees e
JOIN titles t ON e.emp_no = t.emp_no
GROUP BY e.emp_no, e.first_name
HAVING COUNT(t.title) > 1;
```

```python
# PySpark
employees_df.join(titles_df, "emp_no") \
            .groupBy("emp_no", "first_name") \
            .agg(count("title").alias("cnt")) \
            .filter(col("cnt") > 1) \
            .show()
```

**34. Employees who have changed departments.**
*Concept: Self Join History*

```sql
-- SQL
SELECT DISTINCT e.emp_no, e.first_name
FROM employees e
JOIN dept_emp de1 ON e.emp_no = de1.emp_no
JOIN dept_emp de2 ON e.emp_no = de2.emp_no
WHERE de1.dept_no <> de2.dept_no;
```

```python
# PySpark
d1 = dept_emp_df.alias("d1")
d2 = dept_emp_df.alias("d2")

d1.join(d2, "emp_no") \
  .filter(col("d1.dept_no") != col("d2.dept_no")) \
  .select("emp_no") \
  .distinct() \
  .join(employees_df, "emp_no") \
  .show()
```

**35. Employees whose salary decreased.**
*Concept: Self Join with Date Logic*

```sql
-- SQL
SELECT DISTINCT s1.emp_no
FROM salaries s1
JOIN salaries s2 ON s1.emp_no = s2.emp_no
    AND s2.from_date = s1.to_date -- Next record starts when previous ends
WHERE s2.salary < s1.salary;
```

```python
# PySpark
# Using window function (lag) is easier, but strictly Self Join:
s1 = salaries_df.alias("s1")
s2 = salaries_df.alias("s2")

s1.join(s2, (col("s1.emp_no") == col("s2.emp_no")) & (col("s2.from_date") == col("s1.to_date"))) \
  .filter(col("s2.salary") < col("s1.salary")) \
  .select(col("s1.emp_no")) \
  .distinct() \
  .show()
```

**36. Find gaps in employment history (if any).**
*Concept: Date Logic Self Join*

```sql
-- SQL
SELECT e.emp_no, e1.to_date, e2.from_date
FROM dept_emp e1
JOIN dept_emp e2 ON e1.emp_no = e2.emp_no 
    AND e1.from_date < e2.from_date
WHERE NOT EXISTS (
    SELECT 1 FROM dept_emp e3 
    WHERE e3.emp_no = e1.emp_no 
    AND e3.from_date > e1.from_date 
    AND e3.from_date < e2.from_date
);
```

```python
# PySpark
# This is complex. A simpler approach:
# Find cases where to_date is not '9999-01-01' and no record starts the next day
d1 = dept_emp_df.filter(col("to_date") != '9999-01-01')
d2 = dept_emp_df

d1.join(d2, d1["emp_no"] == d2["emp_no"]) \
  .filter(d1["to_date"] < d2["from_date"]) \
  .filter(d1["emp_no"] == 10001) # Filter for specific emp for sanity
  .show()
```

**37. Tenure of employees in current department.**
*Concept: Date Diff*

```sql
-- SQL
SELECT e.first_name, d.dept_name, 
       DATEDIFF(CURRENT_DATE, de.from_date) as days_in_dept
FROM employees e
JOIN dept_emp de ON e.emp_no = de.emp_no AND de.to_date = '9999-01-01'
JOIN departments d ON de.dept_no = d.dept_no;
```

```python
# PySpark
# Spark's datediff takes (end, start)
cond = [employees_df.emp_no == dept_emp_df.emp_no, dept_emp_df.to_date == '9999-01-01',
        dept_emp_df.dept_no == departments_df.dept_no]

joined = employees_df.join(dept_emp_df, "emp_no").join(departments_df, "dept_no")
joined.withColumn("days", datediff(current_date(), col("from_date"))) \
     .select("first_name", "dept_name", "days") \
     .show()
```

**38. Salary increase percentage on the last raise.**
*Concept: Aggregation over dates*

```sql
-- SQL
WITH LastTwo AS (
    SELECT emp_no, salary,
           ROW_NUMBER() OVER (PARTITION BY emp_no ORDER BY from_date DESC) as rn
    FROM salaries
)
SELECT 
    cur.emp_no, 
    cur.salary, 
    prev.salary as old_salary,
    ((cur.salary - prev.salary) / prev.salary) * 100 as pct_increase
FROM LastTwo cur
JOIN LastTwo prev ON cur.emp_no = prev.emp_no AND cur.rn = 1 AND prev.rn = 2;
```

```python
# PySpark
window_spec = Window.partitionBy("emp_no").orderBy(col("from_date").desc())
ranked = salaries_df.withColumn("rn", row_number().over(window_spec))

# Self join
cur = ranked.filter(col("rn") == 1).withColumnRenamed("salary", "cur_sal")
prev = ranked.filter(col("rn") == 2).withColumnRenamed("salary", "prev_sal")

cur.join(prev, "emp_no") \
    .withColumn("pct", ((col("cur_sal") - col("prev_sal")) / col("prev_sal")) * 100) \
    .show()
```

**39. Departments with average salary above company average.**
*Concept: Scalar Subquery equivalent (Cross Join)*

```sql
-- SQL
SELECT d.dept_name, AVG(s.salary)
FROM salaries s
JOIN dept_emp de ON s.emp_no = de.emp_no AND de.to_date = '9999-01-01'
JOIN departments d ON de.dept_no = d.dept_no
WHERE s.to_date = '9999-01-01'
GROUP BY d.dept_name
HAVING AVG(s.salary) > (SELECT AVG(salary) FROM salaries WHERE to_date = '9999-01-01');
```

```python
# PySpark
# Calculate global avg
global_avg = salaries_df.filter(col("to_date") == '9999-01-01').agg(avg("salary")).collect()[0][0]

cond = [salaries_df.emp_no == dept_emp_df.emp_no, dept_emp_df.to_date == '9999-01-01',
        salaries_df.to_date == '9999-01-01', dept_emp_df.dept_no == departments_df.dept_no]

joined = salaries_df.join(dept_emp_df, "emp_no").join(departments_df, "dept_no")
joined.groupBy("dept_name") \
      .agg(avg("salary").alias("avg_sal")) \
      .filter(col("avg_sal") > global_avg) \
      .show()
```

**40. Find the longest serving manager of each department.**
*Concept: Duration Calculation & Max*

```sql
-- SQL
WITH ManagerTenure AS (
    SELECT 
        dm.dept_no, e.emp_no, e.first_name,
        DATEDIFF(dm.to_date, dm.from_date) as days
    FROM dept_manager dm
    JOIN employees e ON dm.emp_no = e.emp_no
)
SELECT * FROM (
    SELECT *,
           ROW_NUMBER() OVER (PARTITION BY dept_no ORDER BY days DESC) as rn
    FROM ManagerTenure
) t WHERE rn = 1;
```

```python
# PySpark
joined = dept_manager_df.join(employees_df, "emp_no")
joined = joined.withColumn("days", datediff(col("to_date"), col("from_date")))

window_spec = Window.partitionBy("dept_no").orderBy(col("days").desc())
joined.withColumn("rn", row_number().over(window_spec)) \
      .filter(col("rn") == 1) \
      .show()
```

---

### Group 6: Recursion & Time Series Logic (41-50)

*Note: PySpark does not support recursive CTEs natively. Solutions use Range Joins (exploding a date sequence) to achieve the same result.*

**41. Generate a report of total hires per month, filling months with 0 hires.**
*Concept: Time Series Generation (SQL Recursive vs PySpark Range Join)*

```sql
-- SQL (Recursive CTE)
WITH RECURSIVE DateSeries AS (
    SELECT MIN(hire_date) as dt FROM employees
    UNION ALL
    SELECT DATE_ADD(dt, INTERVAL 1 MONTH) FROM DateSeries WHERE dt < (SELECT MAX(hire_date) FROM employees)
)
SELECT 
    ds.dt, 
    COUNT(e.emp_no) as hires
FROM DateSeries ds
LEFT JOIN employees e ON ds.dt = DATE_FORMAT(e.hire_date, '%Y-%m-01')
GROUP BY ds.dt;
```

```python
# PySpark
# 1. Generate Months Range
min_date = employees_df.agg(min("hire_date")).collect()[0][0].replace(day=1)
max_date = employees_df.agg(max("hire_date")).collect()[0][0].replace(day=1)

# Generate sequence in Python for the DataFrame creation
import pandas as pd
date_range = pd.date_range(start=min_date, end=max_date, freq='MS')
months_df = spark.createDataFrame(date_range.to_frame(index=False), DateType()).toDF("month_start")

# 2. Prepare hires data
hires_df = employees_df.withColumn("month_start", trunc("hire_date", "month")) \
                       .groupBy("month_start") \
                       .count()

# 3. Left Join to fill zeros
months_df.join(hires_df, "month_start", "left") \
         .na.fill(0, subset=["count"]) \
         .orderBy("month_start") \
         .show()
```

**42. Calculate cumulative headcount of the company over time.**
*Concept: Running Aggregation on Time Series*

```sql
-- SQL (Recursive CTE + Window)
WITH DateSeries AS (
    SELECT MIN(hire_date) as dt FROM employees
    UNION ALL
    SELECT DATE_ADD(dt, INTERVAL 1 DAY) FROM DateSeries WHERE dt < '2000-01-01' -- Limit for demo
),
HireCounts AS (
    SELECT ds.dt, COUNT(e.emp_no) as new_hires
    FROM DateSeries ds
    LEFT JOIN employees e ON ds.dt = e.hire_date
    GROUP BY ds.dt
)
SELECT dt, SUM(new_hires) OVER (ORDER BY dt ROWS BETWEEN UNBOUNDED PRECEDING AND CURRENT ROW) as total_headcount
FROM HireCounts;
```

```python
# PySpark
# Create daily sequence between two dates
start = employees_df.agg(min("hire_date")).collect()[0][0]
end = employees_df.agg(max("hire_date")).collect()[0][0]

# Use Spark range and sequence to generate days (Spark 2.4+)
num_days = (end - start).days
days_df = spark.range(num_days).select(expr("date_add(date('{}'), cast(id as int)) as dt".format(start)))

# Join hires
hires_df = employees_df.groupBy("hire_date").count().withColumnRenamed("hire_date", "dt")

joined = days_df.join(hires_df, "dt", "left").na.fill(0)
joined.withColumn("running_total", sum("count").over(Window.orderBy("dt").rowsBetween(Window.unboundedPreceding, Window.currentRow))) \
      .show()
```

**43. Monthly Active Users (MAU) - Count of employees active in each month.**
*Concept: Expanding date ranges into monthly records*

```sql
-- SQL (Recursive CTE)
-- (This is extremely complex in pure SQL without helper tables, so we assume a standard date_series table exists)
SELECT 
    ds.month_start,
    COUNT(DISTINCT de.emp_no) as active_emps
FROM date_series ds
JOIN dept_emp de 
    ON ds.month_start BETWEEN DATE_FORMAT(de.from_date, '%Y-%m-01') AND DATE_FORMAT(de.to_date, '%Y-%m-01')
GROUP BY ds.month_start;
```

```python
# PySpark
# 1. Generate Month Series
start = dept_emp_df.agg(min("from_date")).collect()[0][0].replace(day=1)
end = dept_emp_df.agg(max("to_date")).collect()[0][0].replace(day=1)
date_list = pd.date_range(start, end, freq='MS')
months_df = spark.createDataFrame(date_list.to_frame(index=False), DateType()).toDF("month_start")

# 2. Normalize dept_emp to monthly granularity is expensive.
#    Instead, use a Range Join condition.
#    Employee is active if month_start is between from_date and to_date.
#    (Warning: This is a broadcast join if one side is small, otherwise cross join with filter)

# Efficient approach: Filter current employees only for simplicity, or expand range:
active_counts = months_df.crossJoin(dept_emp_df) \
    .filter((col("month_start") >= trunc(col("from_date"), "month")) & 
            (col("month_start") <= trunc(col("to_date"), "month"))) \
    .groupBy("month_start") \
    .agg(countDistinct("emp_no").alias("active_employees"))

active_counts.show()
```

**44. Find months where salary expenses spiked compared to previous month.**
*Concept: Time Series Comparison*

```sql
-- SQL
WITH MonthlyExpense AS (
    SELECT DATE_FORMAT(from_date, '%Y-%m-01') as mth, SUM(salary) as total_cost
    FROM salaries
    GROUP BY DATE_FORMAT(from_date, '%Y-%m-01')
)
SELECT 
    mth, total_cost,
    LAG(total_cost) OVER (ORDER BY mth) as prev_cost,
    (total_cost - LAG(total_cost) OVER (ORDER BY mth)) / LAG(total_cost) OVER (ORDER BY mth) as growth
FROM MonthlyExpense;
```

```python
# PySpark
monthly = salaries_df.groupBy(trunc("from_date", "month").alias("mth")).sum("salary").withColumnRenamed("sum(salary)", "cost")
window_spec = Window.orderBy("mth")

monthly.withColumn("prev_cost", lag("cost").over(window_spec)) \
       .withColumn("growth", (col("cost") - col("prev_cost")) / col("prev_cost")) \
       .show()
```

**45. Employee Tenure distribution (Buckets).**
*Concept: Window based on time elapsed*

```sql
-- SQL
SELECT 
    CASE 
        WHEN DATEDIFF(CURRENT_DATE, hire_date) < 365 THEN 'Junior'
        WHEN DATEDIFF(CURRENT_DATE, hire_date) < 1825 THEN 'Mid'
        ELSE 'Senior'
    END as tenure,
    COUNT(*)
FROM employees
GROUP BY tenure;
```

```python
# PySpark
employees_df.withColumn("days", datediff(current_date(), "hire_date")) \
            .withColumn("tenure",
                when(col("days") < 365, "Junior")
                .when(col("days") < 1825, "Mid")
                .otherwise("Senior")) \
            .groupBy("tenure").count() \
            .show()
```

**46. Find consecutive years where an employee got a raise.**
*Concept: Complex Logic (Self Join)*

```sql
-- SQL
SELECT DISTINCT s1.emp_no
FROM salaries s1
JOIN salaries s2 ON s1.emp_no = s2.emp_no
    AND YEAR(s1.from_date) = YEAR(s2.from_date) - 1
    AND s2.salary > s1.salary;
```

```python
# PySpark
s1 = salaries_df.withColumn("y", year("from_date"))
s2 = salaries_df.withColumn("y", year("from_date"))

s1.alias("a").join(s2.alias("b"), 
    (col("a.emp_no") == col("b.emp_no")) & 
    (col("a.y") + 1 == col("b.y")) & 
    (col("b.salary") > col("a.salary"))) \
    .select(col("a.emp_no")).distinct().show()
```

**47. Determine the hire date cohort (Year) and retention count.**
*Concept: Grouping + Window*

```sql
-- SQL
SELECT 
    YEAR(hire_date) as cohort,
    COUNT(*) OVER (PARTITION BY YEAR(hire_date) ORDER BY hire_date) as running_retention
FROM employees
ORDER BY hire_date;
```

```python
# PySpark
window_spec = Window.partitionBy(year("hire_date")).orderBy("hire_date")
employees_df.withColumn("cohort", year("hire_date")) \
            .withColumn("running_retention", count("*").over(window_spec)) \
            .show()
```

**48. Department headcount trend (Simple Time Series).**
*Concept: Generate dates and count active dept_emp records.*

```sql
-- SQL
SELECT d.dept_name, COUNT(*) 
FROM departments d
JOIN dept_emp de ON d.dept_no = de.dept_no
WHERE de.to_date = '9999-01-01'
GROUP BY d.dept_name;
```

```python
# PySpark
# Just current headcount
dept_emp_df.filter(col("to_date") == '9999-01-01') \
           .groupBy("dept_no") \
           .count() \
           .join(departments_df, "dept_no") \
           .show()
```

**49. Identify employees whose current salary is not the highest they've ever earned.**
*Concept: Window vs Current Value*

```sql
-- SQL
WITH MaxSal AS (
    SELECT emp_no, MAX(salary) as max_ever
    FROM salaries
    GROUP BY emp_no
)
SELECT e.first_name, e.last_name, s.current_salary, m.max_ever
FROM employees e
JOIN (SELECT emp_no, salary as current_salary FROM salaries WHERE to_date = '9999-01-01') s ON e.emp_no = s.emp_no
JOIN MaxSal m ON e.emp_no = m.emp_no
WHERE s.current_salary < m.max_ever;
```

```python
# PySpark
max_ever = salaries_df.groupBy("emp_no").agg(max("salary").alias("max_ever"))
current = salaries_df.filter(col("to_date") == '9999-01-01').withColumnRenamed("salary", "current")

current.join(max_ever, "emp_no") \
       .filter(col("current") < col("max_ever")) \
       .join(employees_df, "emp_no") \
       .select("first_name", "last_name", "current", "max_ever") \
       .show()
```

**50. Find the department with the highest salary variance.**
*Concept: Statistical Aggregation (VarPop/VarSamp)*

```sql
-- SQL
SELECT d.dept_name, VAR_POP(s.salary) as variance
FROM salaries s
JOIN dept_emp de ON s.emp_no = de.emp_no AND de.to_date = '9999-01-01'
JOIN departments d ON de.dept_no = d.dept_no
WHERE s.to_date = '9999-01-01'
GROUP BY d.dept_name
ORDER BY variance DESC
LIMIT 1;
```

```python
# PySpark
from pyspark.sql.functions import var_samp

cond = [salaries_df.emp_no == dept_emp_df.emp_no, dept_emp_df.to_date == '9999-01-01',
        salaries_df.to_date == '9999-01-01', dept_emp_df.dept_no == departments_df.dept_no]

joined = salaries_df.join(dept_emp_df, "emp_no").join(departments_df, "dept_no")
joined.groupBy("dept_name") \
      .agg(var_samp("salary").alias("variance")) \
      .orderBy(col("variance").desc()) \
      .limit(1) \
      .show()
```