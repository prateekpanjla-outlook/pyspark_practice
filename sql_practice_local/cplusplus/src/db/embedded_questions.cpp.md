# embedded_questions.cpp - Documentation

## Purpose

Implementation file containing all SQL practice questions hardcoded as C++ data structures. This file contains 10 complete questions with schemas, sample data, and expected results.

## Questions Included

| ID | Title | Difficulty | Tags |
|----|-------|------------|------|
| q1 | Second Highest Salary | easy | aggregate-functions, subqueries |
| q2 | Duplicate Emails | easy | group-by, having-clause, aggregate-functions |
| q3 | Department Highest Salary | medium | window-functions, joins, group-by |
| q4 | Employees Earning More Than Their Manager | easy | joins, self-join |
| q5 | Nth Highest Salary | medium | window-functions, limit-offset, dense-rank |
| q6 | Rank Scores | medium | window-functions, dense-rank |
| q7 | Delete Duplicate Emails | medium | delete, self-join |
| q8 | Consecutive Numbers | medium | joins, self-join |
| q9 | Customers With Largest Revenue | easy | aggregate-functions, group-by, order-by |
| q10 | Actors Who Never Appeared Together | hard | cross-join, not-exists, subqueries |

## Data Structure Pattern

Each question follows this structure:

```cpp
static const QuestionDef question_N = {
    "id",                          // Unique identifier
    "Title",                       // Display name
    "url-slug",                    // URL-friendly identifier
    "Description text...",         // Problem description
    "difficulty",                  // easy/medium/hard
    "category",                    // sql, python, etc.
    "company",                     // FAANG, Amazon, etc.
    "starter code",                // Initial code template
    "reference solution",          // Correct SQL query
    {"tag1", "tag2"},             // Topic tags
    {"hint1", "hint2"},           // User hints
    {                             // Table schemas
        {
            "table_name",
            {{"col1", "TYPE"}, {"col2", "TYPE"}}
        }
    },
    {                             // Sample data
        {"table_name", {
            {{"col1", "val1"}, {"col2", "val2"}},
            {{"col1", "val3"}, {"col2", "val4"}}
        }}
    },
    {"col1", "col2"},            // Expected output columns
    {                             // Expected output rows
        {{"col1", "val1"}, {"col2", "val2"}}
    }
};
```

## Key Functions

### `get_all_questions()`
```cpp
const std::vector<QuestionDef>& get_all_questions();
```
Returns a static vector containing all 10 question definitions.

### `find_question_by_slug()`
```cpp
const QuestionDef* find_question_by_slug(const std::string& slug);
```
Linear search through questions to find matching slug. Used by the question loader to find specific questions.

### `find_question_by_id()`
```cpp
const QuestionDef* find_question_by_id(const std::string& id);
```
Linear search through questions to find matching ID.

### `get_question_count()`
```cpp
size_t get_question_count();
```
Returns the total number of questions (size of ALL_QUESTIONS vector).

## Example Question: Second Highest Salary (q1)

```cpp
static const QuestionDef question_1 = {
    "q1",
    "Second Highest Salary",
    "second-highest-salary",
    "Given an Employee table, write a SQL query to find the second highest salary.",
    "easy",
    "sql",
    "FAANG",
    "-- Write your query here\nSELECT ",
    "SELECT MAX(salary) AS SecondHighestSalary FROM Employee WHERE salary < (SELECT MAX(salary) FROM Employee)",
    {"aggregate-functions", "subqueries"},
    {
        "Use ORDER BY with DESC to sort salaries in descending order",
        "Use LIMIT and OFFSET to get the second row",
        "Or use MAX(salary) with a subquery to find salaries less than the maximum"
    },
    {
        {
            "Employee",
            {
                {"id", "INTEGER"},
                {"name", "VARCHAR"},
                {"salary", "INTEGER"},
                {"department_id", "INTEGER"}
            }
        }
    },
    {
        {"Employee", {
            {{"id", "1"}, {"name", "Alice"}, {"salary", "100000"}, {"department_id", "1"}},
            {{"id", "2"}, {"name", "Bob"}, {"salary", "90000"}, {"department_id", "2"}},
            {{"id", "3"}, {"name", "Charlie"}, {"salary", "120000"}, {"department_id", "1"}},
            {{"id", "4"}, {"name", "David"}, {"salary", "85000"}, {"department_id", "2"}}
        }}
    },
    {"SecondHighestSalary"},
    {{{"SecondHighestSalary", "100000"}}}
};
```

## Architecture Notes

### Adding New Questions

To add a new question:

1. Create a new `QuestionDef` static variable following the pattern above
2. Add it to the `ALL_QUESTIONS` vector
3. Increment the count in documentation

### Compile-Time Data

- All question data exists at compile time as string literals
- No runtime parsing or file I/O required
- Binary size increases with each question (~1-2 KB per question)

### Known Limitations

1. **Table Name Conflicts**: Multiple questions use the same table name (e.g., `Employee`) with different schemas. The last-initialized schema overwrites previous ones.

2. **Hardcoded Updates**: Adding questions requires code changes and recompilation.

## Related Files

- **embedded_questions.hpp**: Header with structure definitions
- **question_loader.cpp**: Converts QuestionDef to runtime Question objects
- **duckdb_executor.cpp**: Uses question schemas to initialize database

## See Also

- [Database Architecture](../DATABASE_ARCHITECTURE.md)
- [Question Loader](question_loader.cpp.md)
