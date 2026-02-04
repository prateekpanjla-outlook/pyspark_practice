

Here is the complete, regenerated list of **50 problems and solutions**, fully updated to match your specific schema structure (e.g., `employee` table uses `id`, `salary` table uses `amount`, etc.).

**Prerequisites (PySpark)**
```python
from pyspark.sql import Window
from pyspark.sql.functions import *
# Assuming DataFrames are named: employee_df, department_df, department_employee_df,
# department_manager_df, salary_df, title_df
```

---

### Group 1: Joins, Filtering & Basic Aggregates (1-10)

**1. List all current employees with their department names.**
*Concept: Inner Join, Current Date Filter*

```sql
-- SQL
SELECT e.id, e.first_name, e.last_name, d.dept_name
FROM employee e
JOIN department_employee de ON e.id = de.employee_id AND de.to_date = '9999-01-01'
JOIN department d ON de.department_id = d.id;
```

```python
# PySpark
cond = [employee_df.id == department_employee_df.employee_id,
        department_employee_df.to_date == '9999-01-01',
        department_employee_df.department_id == department_df.id]

employee_df.join(department_employee_df, "id") \
           .join(department_df, cond) \
           .select(employee_df.id, employee_df.first_name,
                   employee_df.last_name, department_df.dept_name) \
           .show()
```

**2. Find the current head of each department.**
*Concept: Inner Join, Manager Table*

```sql
-- SQL
SELECT d.dept_name, e.first_name, e.last_name
FROM department_manager dm
JOIN employee e ON dm.employee_id = e.id
JOIN department d ON dm.department_id = d.id
WHERE dm.to_date = '9999-01-01';
```

```python
employee_df = spark.read.jdbc(url="jdbc:postgresql://localhost:5432/employees",table="employees.employee",
                         properties={"user": "vagrant", "password": "vagrant","driver": "org.postgresql.Driver"})


department_df = spark.read.jdbc(url="jdbc:postgresql://localhost:5432/employees",table="employees.department",
                         properties={"user": "vagrant", "password": "vagrant","driver": "org.postgresql.Driver"})


department_manager_df = spark.read.jdbc(url="jdbc:postgresql://localhost:5432/employees",table="employees.department_manager",
                         properties={"user": "vagrant", "password": "vagrant","driver": "org.postgresql.Driver"})




# Create aliases for the tables
dm = department_manager_df.alias("dm")
e = employee_df.alias("e")
d = department_df.alias("d")

dm.filter(col("to_date") == '9999-01-01') \
  .join(e, dm.employee_id == e.id) \
  .join(d, dm.department_id == d.id) \
  .select(d.dept_name, e.first_name, e.last_name) \
  .show()

**3. List employees who are currently "Senior Engineers".**
*Concept: Multi-table Join*

```sql
-- SQL
SELECT e.id, e.first_name
FROM employee e
JOIN title t ON e.id = t.employee_id
WHERE t.title = 'Senior Engineer' AND t.to_date = '9999-01-01';
```

```python
# PySpark
employee_df.join(title_df, employee_df.id == title_df.employee_id) \
           .filter((col("title") == "Senior Engineer") & (col("to_date") == '9999-01-01')) \
           .select(employee_df.id, employee_df.first_name) \
           .show()
```

**4. Calculate the average salary per department.**
*Concept: Aggregation*

```sql
-- SQL
SELECT d.dept_name, AVG(s.amount) as avg_salary
FROM salary s
JOIN department_employee de ON s.employee_id = de.employee_id AND de.to_date = '9999-01-01' AND s.to_date = '9999-01-01'
JOIN department d ON de.department_id = d.id
GROUP BY d.dept_name;
```

```python
# PySpark
cond = [salary_df.employee_id == department_employee_df.employee_id,
        salary_df.to_date == '9999-01-01',
        department_employee_df.to_date == '9999-01-01',
        department_employee_df.department_id == department_df.id]

joined = salary_df.join(department_employee_df, cond).join(department_df, "department_id")
joined.groupBy("dept_name").avg("amount").show()
```

**5. Find departments with more than 10,000 current employees.**
*Concept: Having Clause*

```sql
-- SQL
SELECT d.dept_name, COUNT(*) as cnt
FROM department_employee de
JOIN department d ON de.department_id = d.id
WHERE de.to_date = '9999-01-01'
GROUP BY d.dept_name
HAVING COUNT(*) > 10000;
```

```python
# PySpark
department_employee_df.filter(col("to_date") == '9999-01-01') \
                      .join(department_df, department_employee_df.department_id == department_df.id) \
                      .groupBy("dept_name") \
                      .agg(count("*").alias("cnt")) \
                      .filter(col("cnt") > 10000) \
                      .show()
```

**6. Find employees hired in the year 1995.**
*Concept: Date Function*

```sql
-- SQL
SELECT * FROM employee WHERE YEAR(hire_date) = 1995;
```

```python
# PySpark
employee_df.filter(year("hire_date") == 1995).show()
```

**7. Count how many distinct job titles exist.**
*Concept: Distinct Count*

```sql
-- SQL
SELECT COUNT(DISTINCT title) FROM title;
```

```python
# PySpark
title_df.select(countDistinct("title")).show()
```

**8. List employees who have never held a manager title.**
*Concept: Left Join (Anti-Join)*

```sql
-- SQL
SELECT e.first_name, e.last_name
FROM employee e
WHERE NOT EXISTS (
    SELECT 1 FROM title t WHERE t.employee_id = e.id AND t.title LIKE '%Manager%'
);
```

```python
# PySpark
mgrs = title_df.filter(col("title").contains("Manager")).select("employee_id").distinct()
employee_df.join(mgrs, employee_df.id == mgrs.employee_id, "left_anti") \
           .select("first_name", "last_name") \
           .show()
```

**9. Show the salary history for employee ID 10001.**
*Concept: Simple Filter*

```sql
-- SQL
SELECT * FROM salary WHERE employee_id = 10001 ORDER BY from_date;
```

```python
# PySpark
salary_df.filter(col("employee_id") == 10001).orderBy("from_date").show()
```

**10. Find the maximum salary ever paid.**
*Concept: Max*

```sql
-- SQL
SELECT MAX(amount) FROM salary;
```

```python
# PySpark
salary_df.agg(max("amount")).show()
```

---

### Group 2: Window Functions - Ranking & Offsets (11-20)

**11. Find the top 3 earners in each department.**
*Concept: `row_number`*

```sql
-- SQL
WITH Ranked AS (
    SELECT e.id, d.dept_name, s.amount,
           ROW_NUMBER() OVER (PARTITION BY d.dept_name ORDER BY s.amount DESC) as rn
    FROM salary s
    JOIN department_employee de ON s.employee_id = de.employee_id AND de.to_date = '9999-01-01'
    JOIN department d ON de.department_id = d.id
    JOIN employee e ON s.employee_id = e.id
    WHERE s.to_date = '9999-01-01'
)
SELECT * FROM Ranked WHERE rn <= 3;
```

```python
# PySpark
cond = [salary_df.employee_id == department_employee_df.employee_id,
        salary_df.to_date == '9999-01-01',
        department_employee_df.to_date == '9999-01-01',
        department_employee_df.department_id == department_df.id]

joined = salary_df.join(department_employee_df, cond) \
                  .join(department_df, "department_id") \
                  .join(employee_df, salary_df.employee_id == employee_df.id)

window_spec = Window.partitionBy("dept_name").orderBy(col("amount").desc())
joined.withColumn("rn", row_number().over(window_spec)) \
      .filter(col("rn") <= 3) \
      .show()
```

**12. Rank employees by salary within their title.**
*Concept: `dense_rank`*

```sql
-- SQL
SELECT t.title, e.first_name, s.amount,
       DENSE_RANK() OVER (PARTITION BY t.title ORDER BY s.amount DESC) as rnk
FROM employee e
JOIN salary s ON e.id = s.employee_id AND s.to_date = '9999-01-01'
JOIN title t ON e.id = t.employee_id AND t.to_date = '9999-01-01';
```

```python
# PySpark
cond = [employee_df.id == salary_df.employee_id, salary_df.to_date == '9999-01-01',
        employee_df.id == title_df.employee_id, title_df.to_date == '9999-01-01']
joined = employee_df.join(salary_df, employee_df.id == salary_df.employee_id) \
                   .join(title_df, employee_df.id == title_df.employee_id)

window_spec = Window.partitionBy("title").orderBy(col("amount").desc())
joined.withColumn("rnk", dense_rank().over(window_spec)).show()
```

**13. Compare current salary with the previous salary (Raise amount).**
*Concept: `lag`*

```sql
-- SQL
SELECT employee_id, amount,
       LAG(amount) OVER (PARTITION BY employee_id ORDER BY from_date) as prev_amount,
       amount - LAG(amount) OVER (PARTITION BY employee_id ORDER BY from_date) as raise_amount
FROM salary;
```

```python
# PySpark
window_spec = Window.partitionBy("employee_id").orderBy("from_date")
salary_df.withColumn("prev_amount", lag("amount").over(window_spec)) \
        .withColumn("raise", col("amount") - col("prev_amount")) \
        .show()
```

**14. Find the date of the next job title change.**
*Concept: `lead`*

```sql
-- SQL
SELECT employee_id, title, from_date,
       LEAD(from_date) OVER (PARTITION BY employee_id ORDER BY from_date) as next_change_date
FROM title;
```

```python
# PySpark
window_spec = Window.partitionBy("employee_id").orderBy("from_date")
title_df.withColumn("next_change_date", lead("from_date").over(window_spec)).show()
```

**15. Calculate salary quartiles (NTILE).**
*Concept: `ntile`*

```sql
-- SQL
SELECT employee_id, amount,
       NTILE(4) OVER (ORDER BY amount DESC) as quartile
FROM salary WHERE to_date = '9999-01-01';
```

```python
# PySpark
current_salary = salary_df.filter(col("to_date") == '9999-01-01')
current_salary.withColumn("quartile", ntile(4).over(Window.orderBy(col("amount").desc()))).show()
```

**16. Find the percentage rank of a salary within the company.**
*Concept: `percent_rank`*

```sql
-- SQL
SELECT employee_id, amount,
       PERCENT_RANK() OVER (ORDER BY amount ASC) as pct_rank
FROM salary WHERE to_date = '9999-01-01';
```

```python
# PySpark
current_salary = salary_df.filter(col("to_date") == '9999-01-01')
current_salary.withColumn("pct_rank", percent_rank().over(Window.orderBy("amount"))).show()
```

**17. Count the number of employees hired in the same year as each employee.**
*Concept: `count` Window*

```sql
-- SQL
SELECT id, hire_date,
       COUNT(*) OVER (PARTITION BY YEAR(hire_date)) as peers_hired_same_year
FROM employee;
```

```python
# PySpark
window_spec = Window.partitionBy(year("hire_date"))
employee_df.withColumn("peers", count("*").over(window_spec)).show()
```

**18. Find the first hire date for every department.**
*Concept: `first_value`*

```sql
-- SQL
SELECT DISTINCT d.dept_name,
       FIRST_VALUE(e.hire_date) OVER (PARTITION BY d.id ORDER BY e.hire_date) as first_hire
FROM employee e
JOIN department_employee de ON e.id = de.employee_id
JOIN department d ON de.department_id = d.id;
```

```python
# PySpark
cond = [employee_df.id == department_employee_df.employee_id,
        department_employee_df.department_id == department_df.id]
joined = employee_df.join(department_employee_df, employee_df.id == department_employee_df.employee_id) \
                   .join(department_df, department_employee_df.department_id == department_df.id)

window_spec = Window.partitionBy("department_id").orderBy("hire_date")
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
    SELECT employee_id, title, COUNT(*) OVER (PARTITION BY employee_id, title) as cnt
    FROM title
)
SELECT DISTINCT employee_id FROM CountTitles WHERE cnt > 1;
```

```python
# PySpark
window_spec = Window.partitionBy("employee_id", "title")
title_df.withColumn("cnt", count("*").over(window_spec)) \
       .filter(col("cnt") > 1) \
       .select("employee_id") \
       .distinct() \
       .show()
```

**20. Median salary approximation.**
*Concept: Logic using `percent_rank` or `row_number`*

```sql
-- SQL
WITH Ordered AS (
    SELECT amount,
           ROW_NUMBER() OVER (ORDER BY amount) as rn,
           COUNT(*) OVER () as total
    FROM salary WHERE to_date = '9999-01-01'
)
SELECT AVG(amount) FROM Ordered WHERE rn IN (FLOOR((total+1)/2), CEIL((total+1)/2));
```

```python
# PySpark
salary_df.filter(col("to_date") == '9999-01-01') \
        .agg(expr("percentile_approx(amount, 0.5)").alias("median")) \
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
    amount,
    AVG(amount) OVER (
        ORDER BY from_date
        ROWS BETWEEN 2 PRECEDING AND CURRENT ROW
    ) as moving_avg
FROM salary
WHERE employee_id = 10001;
```

```python
# PySpark
window_spec = Window.partitionBy("employee_id") \
                   .orderBy("from_date") \
                   .rowsBetween(-2, 0)

salary_df.filter(col("employee_id") == 10001) \
        .withColumn("moving_avg", avg("amount").over(window_spec)) \
        .show()
```

**22. Running Total of salaries paid globally by date.**
*Concept: `rangeBetween` (Unbounded)*

```sql
-- SQL
SELECT
    from_date,
    SUM(amount) OVER (
        ORDER BY from_date
        RANGE BETWEEN UNBOUNDED PRECEDING AND CURRENT ROW
    ) as running_total
FROM salary
GROUP BY from_date
ORDER BY from_date;
```

```python
# PySpark
daily_cost = salary_df.groupBy("from_date").sum("amount").withColumnRenamed("sum(amount)", "daily_cost")
window_spec = Window.orderBy("from_date").rangeBetween(Window.unboundedPreceding, Window.currentRow)

daily_cost.withColumn("running_total", sum("daily_cost").over(window_spec)).show()
```

**23. Sum of Current Salary + Previous 2 Salaries (Centered Window).**
*Concept: `rowsBetween` (1 Preceding, 1 Following)*

```sql
-- SQL
SELECT
    from_date,
    amount,
    SUM(amount) OVER (
        ORDER BY from_date
        ROWS BETWEEN 1 PRECEDING AND 1 FOLLOWING
    ) as sum_neighbors
FROM salary
WHERE employee_id = 10001;
```

```python
# PySpark
window_spec = Window.partitionBy("employee_id").orderBy("from_date").rowsBetween(-1, 1)
salary_df.filter(col("employee_id") == 10001) \
        .withColumn("sum_neighbors", sum("amount").over(window_spec)) \
        .show()
```

**24. Department Average Salary excluding the current row.**
*Concept: Window Filter Frame*

```sql
-- SQL
SELECT
    e.id, d.dept_name, s.amount,
    AVG(s.amount) OVER (
        PARTITION BY d.id
        ORDER BY s.amount
        ROWS BETWEEN UNBOUNDED PRECEDING AND 1 PRECEDING
    ) as avg_lower_amounts
FROM salary s
JOIN department_employee de ON s.employee_id = de.employee_id AND de.to_date = '9999-01-01'
JOIN department d ON de.department_id = d.id
WHERE s.to_date = '9999-01-01';
```

```python
# PySpark
cond = [salary_df.employee_id == department_employee_df.employee_id, salary_df.to_date == '9999-01-01',
        department_employee_df.department_id == department_df.id, department_employee_df.to_date == '9999-01-01']

joined = salary_df.join(department_employee_df, cond).join(department_df, "department_id")
window_spec = Window.partitionBy("department_id").orderBy("amount").rowsBetween(Window.unboundedPreceding, -1)

joined.withColumn("avg_lower", avg("amount").over(window_spec)).show()
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
FROM employee;
```

```python
# PySpark
# Using rangeBetween with timestamps
emp_ts = employee_df.withColumn("ts", to_timestamp(col("hire_date")))
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
    FROM employee e
    JOIN department_employee de ON e.id = de.employee_id AND de.to_date = '9999-01-01'
    JOIN department d ON de.department_id = d.id
)
PIVOT (
    COUNT(*) FOR gender IN ('M', 'F')
);
```

```python
# PySpark
cond = [employee_df.id == department_employee_df.employee_id, department_employee_df.to_date == '9999-01-01',
        department_employee_df.department_id == department_df.id]

joined = employee_df.join(department_employee_df, employee_df.id == department_employee_df.employee_id) \
                   .join(department_df, department_employee_df.department_id == department_df.id)
joined.groupBy("dept_name").pivot("gender", ["M", "F"]).count().show()
```

**27. Average Salary by Department and Title (Pivot).**
*Concept: `pivot` with Aggregation*

```sql
-- SQL
SELECT * FROM (
    SELECT d.dept_name, t.title, s.amount
    FROM salary s
    JOIN title t ON s.employee_id = t.employee_id AND t.to_date = '9999-01-01'
    JOIN department_employee de ON s.employee_id = de.employee_id AND de.to_date = '9999-01-01'
    JOIN department d ON de.department_id = d.id
    WHERE s.to_date = '9999-01-01'
)
PIVOT (
    AVG(amount) FOR title IN ('Staff', 'Senior Engineer', 'Manager')
);
```

```python
# PySpark
cond = [salary_df.employee_id == title_df.employee_id, title_df.to_date == '9999-01-01',
        salary_df.employee_id == department_employee_df.employee_id, department_employee_df.to_date == '9999-01-01',
        department_employee_df.department_id == department_df.id,
        salary_df.to_date == '9999-01-01']

joined = salary_df.join(title_df, salary_df.employee_id == title_df.employee_id) \
                 .join(department_employee_df, salary_df.employee_id == department_employee_df.employee_id) \
                 .join(department_df, "department_id")
joined.groupBy("dept_name") \
      .pivot("title", ["Staff", "Senior Engineer", "Manager"]) \
      .avg("amount") \
      .show()
```

**28. List all employees in a department as a comma-separated string.**
*Concept: `collect_list` / `group_concat`*

```sql
-- SQL
SELECT d.dept_name, STRING_AGG(e.first_name, ', ') as employees
FROM department d
JOIN department_employee de ON d.id = de.department_id AND de.to_date = '9999-01-01'
JOIN employee e ON de.employee_id = e.id
GROUP BY d.dept_name;
```

```python
# PySpark
cond = [department_df.id == department_employee_df.department_id, department_employee_df.to_date == '9999-01-01',
        department_employee_df.employee_id == employee_df.id]

joined = department_df.join(department_employee_df, cond).join(employee_df, department_employee_df.employee_id == employee_df.id)
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
        WHEN amount < 50000 THEN 'Low'
        WHEN amount < 80000 THEN 'Medium'
        ELSE 'High'
    END as range,
    COUNT(*)
FROM salary
WHERE to_date = '9999-01-01'
GROUP BY range;
```

```python
# PySpark
current_salary = salary_df.filter(col("to_date") == '9999-01-01')
current_salary.withColumn("range",
                   when(col("amount") < 50000, "Low")
                   .when(col("amount") < 80000, "Medium")
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
    FROM department_employee de
    JOIN department d ON de.department_id = d.id AND de.to_date = '9999-01-01'
    JOIN title t ON de.employee_id = t.employee_id AND t.to_date = '9999-01-01'
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
cond = [department_employee_df.department_id == department_df.id, department_employee_df.to_date == '9999-01-01',
        department_employee_df.employee_id == title_df.employee_id, title_df.to_date == '9999-01-01']

counts = department_employee_df.join(department_df, cond) \
                              .join(title_df, department_employee_df.employee_id == title_df.employee_id) \
                              .groupBy(department_df.dept_name, title_df.title).count()

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
    SELECT dm.department_id, s.amount as mgr_amount
    FROM department_manager dm
    JOIN salary s ON dm.employee_id = s.employee_id AND s.to_date = '9999-01-01'
    WHERE dm.to_date = '9999-01-01'
)
SELECT e.id, d.dept_name, s.amount
FROM employee e
JOIN salary s ON e.id = s.employee_id AND s.to_date = '9999-01-01'
JOIN department_employee de ON e.id = de.employee_id AND de.to_date = '9999-01-01'
JOIN department d ON de.department_id = d.id
JOIN CurrentMgrs cm ON d.id = cm.department_id
WHERE s.amount > cm.mgr_amount;
```

```python
# PySpark
mgr_cond = [department_manager_df.employee_id == salary_df.employee_id, salary_df.to_date == '9999-01-01',
            department_manager_df.to_date == '9999-01-01']
managers = department_manager_df.join(salary_df, mgr_cond) \
                               .select(department_manager_df.department_id, salary_df.amount.alias("mgr_amount"))

emp_cond = [employee_df.id == salary_df.employee_id, salary_df.to_date == '9999-01-01',
            employee_df.id == department_employee_df.employee_id, department_employee_df.to_date == '9999-01-01',
            department_employee_df.department_id == department_df.id]

emps = employee_df.join(salary_df, employee_df.id == salary_df.employee_id) \
                  .join(department_employee_df, employee_df.id == department_employee_df.employee_id) \
                  .join(department_df, department_employee_df.department_id == department_df.id)

emps.join(managers, emps.department_id == managers.department_id) \
    .filter(emps.amount > managers.mgr_amount) \
    .select(employee_df.id, department_df.dept_name, emps.amount) \
    .show()
```

**32. Pairs of employees sharing the same birthday.**
*Concept: Self Join with inequality*

```sql
-- SQL
SELECT
    e1.id, e1.first_name,
    e2.id, e2.first_name
FROM employee e1
JOIN employee e2
    ON e1.birth_date = e2.birth_date
    AND e1.id < e2.id;
```

```python
# PySpark
employee_df.alias("e1").join(employee_df.alias("e2"),
    (col("e1.birth_date") == col("e2.birth_date")) & (col("e1.id") < col("e2.id"))) \
    .select(col("e1.id"), col("e1.first_name"), col("e2.id"), col("e2.first_name")) \
    .show()
```

**33. Identify employees who held multiple titles.**
*Concept: Self Join Count*

```sql
-- SQL
SELECT e.id, e.first_name, COUNT(t.title) as title_count
FROM employee e
JOIN title t ON e.id = t.employee_id
GROUP BY e.id, e.first_name
HAVING COUNT(t.title) > 1;
```

```python
# PySpark
employee_df.join(title_df, employee_df.id == title_df.employee_id) \
           .groupBy("id", "first_name") \
           .agg(count("title").alias("cnt")) \
           .filter(col("cnt") > 1) \
           .show()
```

**34. Employees who have changed departments.**
*Concept: Self Join History*

```sql
-- SQL
SELECT DISTINCT e.id, e.first_name
FROM employee e
JOIN department_employee de1 ON e.id = de1.employee_id
JOIN department_employee de2 ON e.id = de2.employee_id
WHERE de1.department_id <> de2.department_id;
```

```python
# PySpark
d1 = department_employee_df.alias("d1")
d2 = department_employee_df.alias("d2")

d1.join(d2, d1.employee_id == d2.employee_id) \
  .filter(col("d1.department_id") != col("d2.department_id")) \
  .select(col("d1.employee_id")) \
  .distinct() \
  .join(employee_df, col("d1.employee_id") == employee_df.id) \
  .show()
```

**35. Employees whose salary decreased.**
*Concept: Self Join with Date Logic*

```sql
-- SQL
SELECT DISTINCT s1.employee_id
FROM salary s1
JOIN salary s2 ON s1.employee_id = s2.employee_id
    AND s2.from_date = s1.to_date -- Next record starts when previous ends
WHERE s2.amount < s1.amount;
```

```python
# PySpark
s1 = salary_df.alias("s1")
s2 = salary_df.alias("s2")

s1.join(s2, (col("s1.employee_id") == col("s2.employee_id")) & (col("s2.from_date") == col("s1.to_date"))) \
  .filter(col("s2.amount") < col("s1.amount")) \
  .select(col("s1.employee_id")) \
  .distinct() \
  .show()
```

**36. Find gaps in employment history (if any).**
*Concept: Date Logic Self Join*

```sql
-- SQL
SELECT e.id, e1.to_date, e2.from_date
FROM department_employee e1
JOIN department_employee e2 ON e1.employee_id = e2.employee_id
    AND e1.from_date < e2.from_date
WHERE NOT EXISTS (
    SELECT 1 FROM department_employee e3
    WHERE e3.employee_id = e1.employee_id
    AND e3.from_date > e1.from_date
    AND e3.from_date < e2.from_date
);
```

```python
# PySpark
d1 = department_employee_df.alias("d1")
d2 = department_employee_df.alias("d2")

d1.join(d2, col("d1.employee_id") == col("d2.employee_id")) \
  .filter(col("d1.to_date") < col("d2.from_date")) \
  .filter(col("d1.employee_id") == 10001)
  .show()
```

**37. Tenure of employees in current department.**
*Concept: Date Diff*

```sql
-- SQL
SELECT e.first_name, d.dept_name,
       DATEDIFF(CURRENT_DATE, de.from_date) as days_in_dept
FROM employee e
JOIN department_employee de ON e.id = de.employee_id AND de.to_date = '9999-01-01'
JOIN department d ON de.department_id = d.id;
```

```python
# PySpark
cond = [employee_df.id == department_employee_df.employee_id, department_employee_df.to_date == '9999-01-01',
        department_employee_df.department_id == department_df.id]

joined = employee_df.join(department_employee_df, employee_df.id == department_employee_df.employee_id) \
                   .join(department_df, "department_id")
joined.withColumn("days", datediff(current_date(), col("from_date"))) \
     .select("first_name", "dept_name", "days") \
     .show()
```

**38. Salary increase percentage on the last raise.**
*Concept: Aggregation over dates*

```sql
-- SQL
WITH LastTwo AS (
    SELECT employee_id, amount,
           ROW_NUMBER() OVER (PARTITION BY employee_id ORDER BY from_date DESC) as rn
    FROM salary
)
SELECT
    cur.employee_id,
    cur.amount,
    prev.amount as old_amount,
    ((cur.amount - prev.amount) / prev.amount) * 100 as pct_increase
FROM LastTwo cur
JOIN LastTwo prev ON cur.employee_id = prev.employee_id AND cur.rn = 1 AND prev.rn = 2;
```

```python
# PySpark
window_spec = Window.partitionBy("employee_id").orderBy(col("from_date").desc())
ranked = salary_df.withColumn("rn", row_number().over(window_spec))

cur = ranked.filter(col("rn") == 1).withColumnRenamed("amount", "cur_amount")
prev = ranked.filter(col("rn") == 2).withColumnRenamed("amount", "prev_amount")

cur.join(prev, "employee_id") \
    .withColumn("pct", ((col("cur_amount") - col("prev_amount")) / col("prev_amount")) * 100) \
    .show()
```

**39. Departments with average salary above company average.**
*Concept: Scalar Subquery equivalent (Cross Join)*

```sql
-- SQL
SELECT d.dept_name, AVG(s.amount)
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

cond = [salary_df.employee_id == department_employee_df.employee_id, department_employee_df.to_date == '9999-01-01',
        salary_df.to_date == '9999-01-01', department_employee_df.department_id == department_df.id]

joined = salary_df.join(department_employee_df, salary_df.employee_id == department_employee_df.employee_id) \
                  .join(department_df, "department_id")
joined.groupBy("dept_name") \
      .agg(avg("amount").alias("avg_amount")) \
      .filter(col("avg_amount") > global_avg) \
      .show()
```

**40. Find the longest serving manager of each department.**
*Concept: Duration Calculation & Max*

```sql
-- SQL
WITH ManagerTenure AS (
    SELECT
        dm.department_id, e.employee_id, e.first_name,
        DATEDIFF(dm.to_date, dm.from_date) as days
    FROM department_manager dm
    JOIN employee e ON dm.employee_id = e.id
)
SELECT * FROM (
    SELECT *,
           ROW_NUMBER() OVER (PARTITION BY department_id ORDER BY days DESC) as rn
    FROM ManagerTenure
) t WHERE rn = 1;
```

```python
# PySpark
joined = department_manager_df.join(employee_df, department_manager_df.employee_id == employee_df.id)
joined = joined.withColumn("days", datediff(col("to_date"), col("from_date")))

window_spec = Window.partitionBy("department_id").orderBy(col("days").desc())
joined.withColumn("rn", row_number().over(window_spec)) \
      .filter(col("rn") == 1) \
      .show()
```

---

### Group 6: Recursion & Time Series Logic (41-50)

*Note: PySpark does not support recursive CTEs. Solutions use Range Joins (exploding a date sequence) to achieve the same result.*

**41. Generate a report of total hires per month, filling months with 0 hires.**
*Concept: Time Series Generation (SQL Recursive vs PySpark Range Join)*

```sql
-- SQL (Recursive CTE)
WITH RECURSIVE DateSeries AS (
    SELECT MIN(hire_date) as dt FROM employee
    UNION ALL
    SELECT DATE_ADD(dt, INTERVAL 1 MONTH) FROM DateSeries WHERE dt < (SELECT MAX(hire_date) FROM employee)
)
SELECT
    ds.dt,
    COUNT(e.id) as hires
FROM DateSeries ds
LEFT JOIN employee e ON ds.dt = DATE_TRUNC('month', e.hire_date)
GROUP BY ds.dt;
```

```python
# PySpark
import pandas as pd

# 1. Generate Month Series
min_date = employee_df.agg(min("hire_date")).collect()[0][0].replace(day=1)
max_date = employee_df.agg(max("hire_date")).collect()[0][0].replace(day=1)
date_range = pd.date_range(start=min_date, end=max_date, freq='MS')
months_df = spark.createDataFrame(date_range.to_frame(index=False), DateType()).toDF("month_start")

# 2. Prepare hires data
hires_df = employee_df.withColumn("month_start", trunc("hire_date", "month")) \
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
    SELECT MIN(hire_date) as dt FROM employee
    UNION ALL
    SELECT DATE_ADD(dt, INTERVAL 1 DAY) FROM DateSeries WHERE dt < '2000-01-01' -- Limit for demo
),
HireCounts AS (
    SELECT ds.dt, COUNT(e.id) as new_hires
    FROM DateSeries ds
    LEFT JOIN employee e ON ds.dt = e.hire_date
    GROUP BY ds.dt
)
SELECT dt, SUM(new_hires) OVER (ORDER BY dt ROWS BETWEEN UNBOUNDED PRECEDING AND CURRENT ROW) as total_headcount
FROM HireCounts;
```

```python
# PySpark
# Create daily sequence
start = employee_df.agg(min("hire_date")).collect()[0][0]
end = employee_df.agg(max("hire_date")).collect()[0][0]
num_days = (end - start).days
days_df = spark.range(num_days).select(expr("date_add(date('{}'), cast(id as int)) as dt".format(start)))

hires_df = employee_df.groupBy("hire_date").count().withColumnRenamed("hire_date", "dt")
joined = days_df.join(hires_df, "dt", "left").na.fill(0)

joined.withColumn("running_total", sum("count").over(Window.orderBy("dt").rowsBetween(Window.unboundedPreceding, Window.currentRow))) \
      .show()
```

**43. Monthly Active Users (MAU) - Count of employees active in each month.**
*Concept: Expanding date ranges into monthly records*

```sql
-- SQL
-- (Assumes date_series table exists or logic from #41 is used)
SELECT
    ds.month_start,
    COUNT(DISTINCT de.employee_id) as active_emps
FROM date_series ds
JOIN department_employee de
    ON ds.month_start >= DATE_TRUNC('month', de.from_date)
    AND ds.month_start <= DATE_TRUNC('month', de.to_date)
GROUP BY ds.month_start;
```

```python
# PySpark
# Generate Month Series (similar to Q41)
min_date = department_employee_df.agg(min("from_date")).collect()[0][0].replace(day=1)
max_date = department_employee_df.agg(max("to_date")).collect()[0][0].replace(day=1)
date_list = pd.date_range(min_date, max_date, freq='MS')
months_df = spark.createDataFrame(date_list.to_frame(index=False), DateType()).toDF("month_start")

# Range Join: Active if month_start is between from_date and to_date
active_counts = months_df.crossJoin(department_employee_df) \
    .filter((col("month_start") >= trunc(col("from_date"), "month")) &
            (col("month_start") <= trunc(col("to_date"), "month"))) \
    .groupBy("month_start") \
    .agg(countDistinct("employee_id").alias("active_employees"))

active_counts.show()
```

**44. Find months where salary expenses spiked compared to previous month.**
*Concept: Time Series Comparison*

```sql
-- SQL
WITH MonthlyExpense AS (
    SELECT DATE_TRUNC('month', from_date) as mth, SUM(amount) as total_cost
    FROM salary
    GROUP BY DATE_TRUNC('month', from_date)
)
SELECT
    mth, total_cost,
    LAG(total_cost) OVER (ORDER BY mth) as prev_cost,
    (total_cost - LAG(total_cost) OVER (ORDER BY mth)) / LAG(total_cost) OVER (ORDER BY mth) as growth
FROM MonthlyExpense;
```

```python
# PySpark
monthly = salary_df.groupBy(trunc("from_date", "month").alias("mth")).sum("amount").withColumnRenamed("sum(amount)", "cost")
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
FROM employee
GROUP BY tenure;
```

```python
# PySpark
employee_df.withColumn("days", datediff(current_date(), "hire_date")) \
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
SELECT DISTINCT s1.employee_id
FROM salary s1
JOIN salary s2 ON s1.employee_id = s2.employee_id
    AND YEAR(s1.from_date) + 1 = YEAR(s2.from_date)
    AND s2.amount > s1.amount;
```

```python
# PySpark
s1 = salary_df.withColumn("y", year("from_date"))
s2 = salary_df.withColumn("y", year("from_date"))

s1.alias("a").join(s2.alias("b"),
    (col("a.employee_id") == col("b.employee_id")) &
    (col("a.y") + 1 == col("b.y")) &
    (col("b.amount") > col("a.amount"))) \
    .select(col("a.employee_id")).distinct().show()
```

**47. Determine the hire date cohort (Year) and retention count.**
*Concept: Grouping + Window*

```sql
-- SQL
SELECT
    YEAR(hire_date) as cohort,
    COUNT(*) OVER (PARTITION BY YEAR(hire_date) ORDER BY hire_date) as running_retention
FROM employee
ORDER BY hire_date;
```

```python
# PySpark
window_spec = Window.partitionBy(year("hire_date")).orderBy("hire_date")
employee_df.withColumn("cohort", year("hire_date")) \
            .withColumn("running_retention", count("*").over(window_spec)) \
            .show()
```

**48. Department headcount trend (Simple Time Series).**
*Concept: Generate dates and count active dept_emp records.*

```sql
-- SQL
SELECT d.dept_name, COUNT(*)
FROM department d
JOIN department_employee de ON d.id = de.department_id
WHERE de.to_date = '9999-01-01'
GROUP BY d.dept_name;
```

```python
# PySpark
# Just current headcount
department_employee_df.filter(col("to_date") == '9999-01-01') \
                      .groupBy("department_id") \
                      .count() \
                      .join(department_df, department_employee_df.department_id == department_df.id) \
                      .show()
```

**49. Identify employees whose current salary is not the highest they've ever earned.**
*Concept: Window vs Current Value*

```sql
-- SQL
WITH MaxSal AS (
    SELECT employee_id, MAX(amount) as max_ever
    FROM salary
    GROUP BY employee_id
)
SELECT e.first_name, e.last_name, s.current_amount, m.max_ever
FROM employee e
JOIN (SELECT employee_id, amount as current_amount FROM salary WHERE to_date = '9999-01-01') s ON e.id = s.employee_id
JOIN MaxSal m ON e.id = m.employee_id
WHERE s.current_amount < m.max_ever;
```

```python
# PySpark
max_ever = salary_df.groupBy("employee_id").agg(max("amount").alias("max_ever"))
current = salary_df.filter(col("to_date") == '9999-01-01').withColumnRenamed("amount", "current")

current.join(max_ever, "employee_id") \
       .filter(col("current") < col("max_ever")) \
       .join(employee_df, current.employee_id == employee_df.id) \
       .select("first_name", "last_name", "current", "max_ever") \
       .show()
```

**50. Find the department with the highest salary variance.**
*Concept: Statistical Aggregation (VarPop/VarSamp)*

```sql
-- SQL
SELECT d.dept_name, VAR_POP(s.amount) as variance
FROM salary s
JOIN department_employee de ON s.employee_id = de.employee_id AND de.to_date = '9999-01-01'
JOIN department d ON de.department_id = d.id
WHERE s.to_date = '9999-01-01'
GROUP BY d.dept_name
ORDER BY variance DESC
LIMIT 1;
```

```python
# PySpark
from pyspark.sql.functions import var_samp

cond = [salary_df.employee_id == department_employee_df.employee_id, department_employee_df.to_date == '9999-01-01',
        salary_df.to_date == '9999-01-01', department_employee_df.department_id == department_df.id]

joined = salary_df.join(department_employee_df, cond).join(department_df, "department_id")
joined.groupBy("dept_name") \
      .agg(var_samp("amount").alias("variance")) \
      .orderBy(col("variance").desc()) \
      .limit(1) \
      .show()
```
