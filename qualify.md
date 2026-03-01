# QUALIFY Clause - Filter Window Functions Without Subqueries

## What is QUALIFY?

`QUALIFY` is a SQL extension (Teradata, Snowflake, BigQuery, H2) that filters results based on **window function values** without needing a subquery wrapper.

---

## The "Before QUALIFY" Problem

### ❌ This doesn't work - can't use window function in WHERE

```sql
SELECT
    employee_name,
    department,
    salary,
    RANK() OVER (PARTITION BY department ORDER BY salary DESC) as salary_rank
FROM employees
WHERE salary_rank <= 3;
-- ERROR: window function not allowed in WHERE clause
```

### ✅ Traditional Solution (verbose subquery)

```sql
SELECT *
FROM (
    SELECT
        employee_name,
        department,
        salary,
        RANK() OVER (PARTITION BY department ORDER BY salary DESC) as salary_rank
    FROM employees
) t
WHERE salary_rank <= 3;
```

### ✅ With QUALIFY - Clean Syntax

```sql
SELECT
    employee_name,
    department,
    salary,
    RANK() OVER (PARTITION BY department ORDER BY salary DESC) as salary_rank
FROM employees
QUALIFY salary_rank <= 3;
```

---

## Common Use Cases with Examples

### Example 1: Top N Per Group

```sql
-- Top 3 highest-paid employees per department
SELECT
    department,
    employee_name,
    salary,
    ROW_NUMBER() OVER (PARTITION BY department ORDER BY salary DESC) as row_num
FROM employees
QUALIFY row_num <= 3;
```

### Example 2: Remove Duplicates

```sql
-- Keep only one row per duplicate set
SELECT
    id,
    name,
    email,
    ROW_NUMBER() OVER (PARTITION BY email ORDER BY id) as dup_num
FROM customers
QUALIFY dup_num = 1;
```

### Example 3: Gaps and Islands (Consecutive Groups)

```sql
-- Find sequences of consecutive days (7+ days)
SELECT
    user_id,
    date,
    SUM(is_consecutive) OVER (
        PARTITION BY user_id, date_group
        ORDER BY date
    ) as consecutive_days
FROM (
    SELECT
        user_id,
        date,
        CASE
            WHEN date = prev_date + 1 THEN 1
            ELSE 0
        END as is_consecutive,
        SUM(CASE WHEN date = prev_date + 1 THEN 0 ELSE 1 END) OVER (
            PARTITION BY user_id ORDER BY date
        ) as date_group
    FROM (
        SELECT
            user_id,
            date,
            LAG(date) OVER (PARTITION BY user_id ORDER BY date) as prev_date
        FROM user_activity
    )
)
QUALIFY consecutive_days >= 7;
```

### Example 4: Percentile Filtering

```sql
-- Keep only top 10% by revenue
SELECT
    product,
    revenue,
    PERCENT_RANK() OVER (ORDER BY revenue DESC) as percentile
FROM sales
QUALIFY percentile <= 0.1;
```

### Example 5: Time Series - First/Last Occurrence

```sql
-- Get only the first purchase for each customer
SELECT
    customer_id,
    purchase_date,
    amount,
    ROW_NUMBER() OVER (PARTITION BY customer_id ORDER BY purchase_date) as purchase_num
FROM purchases
QUALIFY purchase_num = 1;
```

---

## QUALIFY vs WHERE vs HAVING

### SQL Clause Order

```
┌─────────────────────────────────────────────────────────────┐
│  1. FROM      → Data source                                │
│  2. WHERE     → Filter raw rows (no aggregates/windows)     │
│  3. GROUP BY  → Create groups                               │
│  4. HAVING    → Filter groups (aggregates only)             │
│  5. WINDOW    → Calculate window functions                 │
│  6. QUALIFY   → Filter by window functions ← Extended SQL!  │
│  7. SELECT    → Final columns                               │
│  8. ORDER BY  → Sort results                                │
└─────────────────────────────────────────────────────────────┘
```

---

## QUALIFY with Multiple Conditions

```sql
-- Complex filtering with window functions
SELECT
    employee_name,
    department,
    salary,
    hire_date,
    ROW_NUMBER() OVER (PARTITION BY department ORDER BY salary DESC) as salary_rank,
    ROW_NUMBER() OVER (PARTITION BY department ORDER BY hire_date) as tenure_rank
FROM employees
QUALIFY salary_rank <= 3 AND tenure_rank = 1;
-- Within each dept: top 3 salaries AND first hired person
```

---

## QUALIFY vs Subquery - Side-by-Side Comparison

### Without QUALIFY (Standard SQL)

```sql
SELECT * FROM (
    SELECT
        product,
        revenue,
        SUM(revenue) OVER (
            PARTITION BY product
            ORDER BY month
            ROWS UNBOUNDED PRECEDING
        ) as running_total
    FROM sales
) t
WHERE running_total > 10000;
```

**Issues:**
- Two levels of nesting
- Harder to read and maintain
- Need to reference the subquery as `t`

### With QUALIFY (Extended SQL)

```sql
SELECT
    product,
    revenue,
    SUM(revenue) OVER (
        PARTITION BY product
        ORDER BY month
        ROWS UNBOUNDED PRECEDING
    ) as running_total
FROM sales
QUALIFY running_total > 10000;
```

**Benefits:**
- Single level
- Clearer intent
- Easier to maintain

---

## QUALIFY Pattern Summary

| Pattern | Description | Example |
|---------|-------------|---------|
| `QUALIFY row_num = 1` | First row per group | Get most recent item |
| `QUALIFY row_num <= N` | Top N rows per group | Top 3 per department |
| `QUALIFY rank = 1` | One row per rank value | Remove ties, keep one |
| `QUALIFY percentile <= 0.1` | Top 10% | Highest percentile |
| `QUALIFY row_num = (SELECT MAX(row_num) FROM ...)` | Last row per group | Most recent item |

---

## Database Support

| Database | QUALIFY Support |
|----------|-----------------|
| **Snowflake** | ✅ Yes |
| **Google BigQuery** | ✅ Yes |
| **Teradata** | ✅ Yes (originator) |
| **H2 Database** | ✅ Yes |
| **Apache Druid** | ✅ Yes |
| **PostgreSQL** | ❌ No (use subquery or DISTINCT ON) |
| **SQL Server** | ❌ No (use subquery or CROSS APPLY) |
| **Oracle** | ❌ No (use subquery) |
| **MySQL** | ❌ No (use subquery) |
| **DuckDB** | ❌ No (use subquery) |

---

## Alternatives for Databases Without QUALIFY

### PostgreSQL - DISTINCT ON

```sql
-- First row per group (simpler alternative to ROW_NUMBER)
SELECT DISTINCT ON (department)
    employee_name,
    department,
    salary
FROM employees
ORDER BY department, salary DESC;
```

### PostgreSQL - LATERAL JOIN

```sql
SELECT t.*
FROM employees e,
LATERAL (
    SELECT
        e.*,
        ROW_NUMBER() OVER (PARTITION BY department ORDER BY salary DESC) as rn
) t
WHERE t.rn <= 3;
```

### SQL Server - CROSS APPLY

```sql
SELECT t.*
FROM employees e
CROSS APPLY (
    SELECT
        e.*,
        ROW_NUMBER() OVER (PARTITION BY department ORDER BY salary DESC) as rn
) t
WHERE t.rn <= 3;
```

### Standard SQL - Subquery (Works Everywhere)

```sql
SELECT *
FROM (
    SELECT
        employee_name,
        department,
        salary,
        ROW_NUMBER() OVER (PARTITION BY department ORDER BY salary DESC) as rn
    FROM employees
) t
WHERE t.rn <= 3;
```

---

## Quick Pattern Reference

| Goal | QUALIFY Pattern (full query) |
|------|------------------------------|
| First row per group | `QUALIFY row_num = 1` |
| Top N rows per group | `QUALIFY row_num <= N` |
| One per rank (remove ties) | `QUALIFY rank = 1` |
| Top percent | `QUALIFY percentile <= 0.1` |
| Exclude first N | `QUALIFY row_num > N` |

> **Note:** These are just the QUALIFY clauses. Full query includes SELECT, FROM, and the window function definition.

---

## Key Takeaway

```
╔═══════════════════════════════════════════════════════════════╗
║  QUALIFY eliminates the "wrap in subquery" pattern            ║
║                                                               ║
║  Instead of: SELECT * FROM (SELECT ..., ROW_NUMBER() ...) t  ║
║               WHERE row_num <= 3                               ║
║                                                               ║
║  Just write:  SELECT ..., ROW_NUMBER() ...                   ║
║               QUALIFY row_num <= 3                             ║
║                                                               ║
║  QUALIFY = "After computing window functions, filter"       ║
╚═══════════════════════════════════════════════════════════════╝
```

---

## Real-World Examples

### Example: Employee Rankings

```sql
-- Identify highest-paid employees in each department
SELECT
    department,
    employee_name,
    salary,
    RANK() OVER (PARTITION BY department ORDER BY salary DESC) as salary_rank
FROM employees
QUALIFY salary_rank = 1;
-- Only the highest-paid person in each department
```

### Example: Time Series Analysis

```sql
-- Find days where stock hit an all-time high
SELECT
    date,
    stock_price,
    MAX(stock_price) OVER (ORDER BY date ROWS UNBOUNDED PRECEDING) as max_to_date
FROM stock_prices
QUALIFY stock_price = max_to_date;
-- Days when stock reached a new peak
```

### Example: Session Analysis

```sql
-- Get the last page view in each session
SELECT
    session_id,
    page_url,
    timestamp,
    ROW_NUMBER() OVER (PARTITION BY session_id ORDER BY timestamp DESC) as reverse_order
FROM page_views
QUALIFY reverse_order = 1;
```

---

*QUALIFY clause is a powerful extension that simplifies window function filtering. Check your database documentation for support.*
