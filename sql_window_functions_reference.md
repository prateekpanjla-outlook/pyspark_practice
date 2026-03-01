# SQL Window Functions - Complete Reference

Based on SQL:2003 Standard (and later additions)

---

## Table of Contents

1. [Ranking Functions](#1-ranking-functions)
2. [Offset Functions](#2-offset-functions-laglead)
3. [Value Functions](#3-value-functions-firstlastnth)
4. [Aggregate Functions](#4-aggregate-functions-as-window-functions)
5. [String Aggregate Functions](#5-string-aggregate-functions)
6. [Statistical Functions](#6-statistical-functions)
7. [Frame Support Reference](#frame-support-reference)
8. [Database-Specific Extensions](#database-specific-extensions)
9. [Example Usage](#example-usage)

---

## 1. Ranking Functions

| Function | Description | Frame Allowed |
|----------|-------------|---------------|
| **ROW_NUMBER()** | Unique sequential integer (1, 2, 3...) | ❌ No |
| **RANK()** | Rank with gaps for ties (1, 2, 2, 4...) | ❌ No |
| **DENSE_RANK()** | Rank without gaps (1, 2, 2, 3...) | ❌ No |
| **NTILE(n)** | Distribute rows into n buckets | ❌ No |
| **PERCENT_RANK()** | Relative rank (0 to 1) | ❌ No |
| **CUME_DIST()** | Cumulative distribution (0 to 1) | ❌ No |

### Key Characteristics

- **No frame specification allowed** - operate on entire partition
- **ORDER BY is required** (except in some databases for specific use cases)
- **PARTITION BY is optional** - without it, operates on entire result set

### Ranking Example

```sql
SELECT
    employee_name,
    department,
    salary,
    ROW_NUMBER() OVER (PARTITION BY department ORDER BY salary DESC) as row_num,
    RANK() OVER (PARTITION BY department ORDER BY salary DESC) as rank,
    DENSE_RANK() OVER (PARTITION BY department ORDER BY salary DESC) as dense_rank,
    NTILE(4) OVER (PARTITION BY department ORDER BY salary DESC) as quartile
FROM employees;
```

---

## 2. Offset Functions (LAG/LEAD)

| Function | Description | Frame Allowed |
|----------|-------------|---------------|
| **LAG(expr, n, default)** | Value from n rows **before** current | ❌ No |
| **LEAD(expr, n, default)** | Value from n rows **after** current | ❌ No |

### Parameters

- **expr**: The column/expression to retrieve
- **n** (optional): Number of rows to look back/forward (default: 1)
- **default** (optional): Value to return if no row found (default: NULL)

### Key Characteristics

- **No frame specification allowed**
- **ORDER BY is required** - defines the "direction" for offset
- Useful for:
  - Year-over-year comparisons
  - Comparing current row to previous/next
  - Calculating differences between consecutive rows

### Offset Example

```sql
SELECT
    date,
    revenue,
    LAG(revenue) OVER (ORDER BY date) as prev_revenue,
    LEAD(revenue) OVER (ORDER BY date) as next_revenue,
    revenue - LAG(revenue) OVER (ORDER BY date) as daily_change,
    LAG(revenue, 7, 0) OVER (ORDER BY date) as revenue_week_ago
FROM sales;
```

---

## 3. Value Functions (First/Last/Nth)

| Function | Description | Frame Allowed |
|----------|-------------|---------------|
| **FIRST_VALUE(expr)** | First value in window frame | ✅ Yes |
| **LAST_VALUE(expr)** | Last value in window frame | ✅ Yes |
| **NTH_VALUE(expr, n)** | Nth value in window frame | ✅ Yes |

### Key Characteristics

- **Frame specification determines results** - be careful with default frame!
- **ORDER BY is typically required**
- Default frame is `RANGE BETWEEN UNBOUNDED PRECEDING AND CURRENT ROW` which may not give expected results for `LAST_VALUE`

### Value Functions Example

```sql
SELECT
    date,
    revenue,
    FIRST_VALUE(revenue) OVER (
        ORDER BY date
        ROWS BETWEEN UNBOUNDED PRECEDING AND UNBOUNDED FOLLOWING
    ) as first_revenue,
    LAST_VALUE(revenue) OVER (
        ORDER BY date
        ROWS BETWEEN UNBOUNDED PRECEDING AND UNBOUNDED FOLLOWING
    ) as last_revenue,
    NTH_VALUE(revenue, 3) OVER (
        ORDER BY date
        ROWS BETWEEN UNBOUNDED PRECEDING AND CURRENT ROW
    ) as third_revenue
FROM sales;
```

---

## 4. Aggregate Functions (as Window Functions)

All standard aggregate functions can be used as window functions.

| Function | Description | Frame Allowed |
|----------|-------------|---------------|
| **COUNT(\*)** or **COUNT(expr)** | Count of rows | ✅ Yes |
| **SUM(expr)** | Sum of values | ✅ Yes |
| **AVG(expr)** | Average of values | ✅ Yes |
| **MIN(expr)** | Minimum value | ✅ Yes |
| **MAX(expr)** | Maximum value | ✅ Yes |
| **STDDEV(expr)** | Standard deviation (sample) | ✅ Yes |
| **STDDEV_POP(expr)** | Standard deviation (population) | ✅ Yes |
| **VARIANCE(expr)** | Variance (sample) | ✅ Yes |
| **VAR_POP(expr)** | Variance (population) | ✅ Yes |
| **CORR(expr1, expr2)** | Correlation coefficient | ✅ Yes |
| **COVAR(expr1, expr2)** | Covariance | ✅ Yes |

### Common Use Cases

- **Running totals**: `ROWS BETWEEN UNBOUNDED PRECEDING AND CURRENT ROW`
- **Moving averages**: `ROWS BETWEEN 2 PRECEDING AND CURRENT ROW`
- **Cumulative metrics**: Same as running totals

### Aggregate Example

```sql
SELECT
    date,
    revenue,
    SUM(revenue) OVER (
        ORDER BY date
        ROWS BETWEEN UNBOUNDED PRECEDING AND CURRENT ROW
    ) as running_total,
    AVG(revenue) OVER (
        ORDER BY date
        ROWS BETWEEN 4 PRECEDING AND CURRENT ROW
    ) as moving_avg_5day,
    MAX(revenue) OVER (
        PARTITION BY EXTRACT(YEAR FROM date)
        ORDER BY date
        ROWS BETWEEN UNBOUNDED PRECEDING AND CURRENT ROW
    ) as ytd_max
FROM sales;
```

---

## 5. String Aggregate Functions

| Function | Description | Frame Allowed |
|----------|-------------|---------------|
| **STRING_AGG(expr, delimiter)** | Concatenates values with delimiter | ✅ Yes |
| **LISTAGG(expr, delimiter)** | Same as STRING_AGG (Oracle/SQL Server) | ✅ Yes |

### String Aggregate Example

```sql
-- PostgreSQL / SQL Server
SELECT
    department,
    employee_name,
    STRING_AGG(employee_name, ', ') OVER (
        PARTITION BY department
        ORDER BY hire_date
        ROWS BETWEEN UNBOUNDED PRECEDING AND CURRENT ROW
    ) as all_employees_hired
FROM employees;

-- Oracle
SELECT
    department,
    LISTAGG(employee_name, ', ') WITHIN GROUP (ORDER BY hire_date)
        OVER (PARTITION BY department) as employees_in_order
FROM employees;
```

---

## 6. Statistical Functions

| Function | Description | Frame Allowed |
|----------|-------------|---------------|
| **PERCENTILE_CONT(p)** | Continuous percentile (interpolated) | ✅ Yes |
| **PERCENTILE_DISC(p)** | Discrete percentile (actual value) | ✅ Yes |
| **MODE()** | Most frequent value | ✅ Yes |

### Statistical Example

```sql
SELECT
    department,
    salary,
    PERCENTILE_CONT(0.5) WITHIN GROUP (ORDER BY salary) OVER (
        PARTITION BY department
    ) as median_salary,
    PERCENTILE_DISC(0.95) WITHIN GROUP (ORDER BY salary) OVER (
        PARTITION BY department
    ) as percentile_95_salary
FROM employees;
```

---

## Frame Support Reference

```
┌─────────────────────────────────────────────────────────────┐
│  NO Frame Allowed (operate on full partition)              │
├─────────────────────────────────────────────────────────────┤
│  ROW_NUMBER, RANK, DENSE_RANK, NTILE                       │
│  PERCENT_RANK, CUME_DIST                                    │
│  LAG, LEAD                                                  │
└─────────────────────────────────────────────────────────────┘

┌─────────────────────────────────────────────────────────────┐
│  Frame Allowed (can use ROWS/RANGE/GROUPS)                  │
├─────────────────────────────────────────────────────────────┤
│  All aggregate functions (SUM, AVG, COUNT, MIN, MAX...)    │
│  FIRST_VALUE, LAST_VALUE, NTH_VALUE                         │
│  PERCENTILE_CONT, PERCENTILE_DISC                           │
│  STRING_AGG, LISTAGG                                       │
└─────────────────────────────────────────────────────────────┘
```

### Frame Clause Syntax

```sql
{ ROWS | RANGE | GROUPS } BETWEEN
    { frame_start | frame_between }
    AND frame_end

frame_start ::= UNBOUNDED PRECEDING | numeric_value PRECEDING | CURRENT ROW
frame_end ::= UNBOUNDED FOLLOWING | numeric_value FOLLOWING | CURRENT ROW
```

### Common Frame Patterns

| Pattern | Meaning | Use For |
|---------|---------|---------|
| `ROWS UNBOUNDED PRECEDING` | From start to current row | Running totals |
| `ROWS BETWEEN 2 PRECEDING AND CURRENT ROW` | Current + 2 before | 3-day moving average |
| `ROWS BETWEEN 1 PRECEDING AND 1 FOLLOWING` | 1 before, current, 1 after | Centered moving average |
| `ROWS UNBOUNDED PRECEDING AND UNBOUNDED FOLLOWING` | Entire partition | First/last value in partition |

---

## Database-Specific Extensions

| Database | Extended Functions |
|----------|-------------------|
| **PostgreSQL** | No major extensions (standard compliant) |
| **SQL Server** | `COALESCE` windowed, `CURRENT_ROW`, `OFFSET` clauses |
| **Oracle** | `LISTAGG`, `KEEP` (DENSE_RANK FIRST/LAST) |
| **DuckDB** | `ARG_MAX`, `ARG_MIN`, `MODE`, `HISTOGRAM` |
| **MySQL 8.0+** | Full standard support |

---

## Complete Example Usage

```sql
-- Comprehensive window function example
WITH sales_data AS (
    SELECT
        DATE_TRUNC('month', sale_date) as month,
        department,
        employee_id,
        employee_name,
        amount as revenue
    FROM sales
)
SELECT
    month,
    department,
    employee_name,
    revenue,

    -- Ranking functions
    ROW_NUMBER() OVER (PARTITION BY department ORDER BY revenue DESC) as dept_rank,
    RANK() OVER (ORDER BY revenue DESC) as global_rank,
    NTILE(4) OVER (ORDER BY revenue DESC) as revenue_quartile,

    -- Offset functions
    LAG(revenue) OVER (PARTITION BY employee_id ORDER BY month) as prev_month_revenue,
    revenue - LAG(revenue) OVER (PARTITION BY employee_id ORDER BY month) as month_over_month,

    -- Aggregate window functions
    SUM(revenue) OVER (PARTITION BY department ORDER BY month ROWS UNBOUNDED PRECEDING) as dept_running_total,
    AVG(revenue) OVER (ORDER BY month ROWS BETWEEN 2 PRECEDING AND CURRENT ROW) as three_month_avg,

    -- Value functions
    FIRST_VALUE(revenue) OVER (PARTITION BY department ORDER BY month ROWS BETWEEN UNBOUNDED PRECEDING AND UNBOUNDED FOLLOWING) as dept_first_month,

    -- Statistical
    PERCENT_RANK() OVER (PARTITION BY department ORDER BY revenue) as percent_rank_in_dept

FROM sales_data
ORDER BY department, month;
```

---

## Quick Syntax Reference

```sql
<window_function> OVER (
    [ PARTITION BY expression [, ...] ]
    [ ORDER BY expression [ ASC | DESC ] [, ...] ]
    [ frame_clause ]
)

-- Frame clause options:
ROWS BETWEEN { UNBOUNDED PRECEDING | n PRECEDING | CURRENT ROW | n FOLLOWING | UNBOUNDED FOLLOWING }
    AND
    { UNBOUNDED PRECEDING | n PRECEDING | CURRENT ROW | n FOLLOWING | UNBOUNDED FOLLOWING }
```

---

## Performance Considerations

1. **ORDER BY is required for most window functions** - causes sorting
2. **Different ORDER BY keys in query vs window function** = multiple sorts
3. **Indexes on ORDER BY columns** can avoid sort operations
4. **PARTITION BY** creates independent windows - more memory usage
5. **Large frames** (especially `UNBOUNDED`) require more memory

---

## Tips and Gotchas

| Issue | Solution |
|-------|----------|
| LAST_VALUE returns unexpected result | Use `ROWS BETWEEN UNBOUNDED PRECEDING AND UNBOUNDED FOLLOWING` |
| NTILE requires ORDER BY | Always specify ORDER BY (mandatory for NTILE) |
| Peers split across NTILE buckets | By design - NTILE splits ties, unlike RANK |
| Two different ORDER BY keys = two sorts | Align keys or add index if performance critical |
| Memory issues with large windows | Consider PARTITION BY to break into smaller windows |

---

*SQL:2003 standard introduced window functions. Check your database documentation for specific implementation details.*
