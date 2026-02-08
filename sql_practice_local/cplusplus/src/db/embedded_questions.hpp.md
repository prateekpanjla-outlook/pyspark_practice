# embedded_questions.hpp - Documentation

## Purpose

Header file that defines the data structures for storing SQL practice questions directly in compiled C++ code. This enables a zero-dependency deployment model where questions are embedded in the binary instead of being loaded from external files (JSON/Python).

## Key Data Structures

### ColumnDef
```cpp
struct ColumnDef {
    const char* name;   // Column name
    const char* type;   // SQL type (INTEGER, VARCHAR, etc.)
};
```
Defines a single column in a database table schema.

### TableDef
```cpp
struct TableDef {
    const char* name;                   // Table name
    std::vector<ColumnDef> columns;     // Column definitions
};
```
Defines a complete database table with all its columns.

### DataRow
```cpp
using DataRow = std::unordered_map<const char*, const char*>;
```
A single row of sample data represented as key-value pairs (column name -> value).

### QuestionDef
```cpp
struct QuestionDef {
    const char* id;                                    // Unique identifier (e.g., "q1")
    const char* title;                                 // Display title
    const char* slug;                                  // URL-friendly identifier
    const char* description;                           // Problem description
    const char* difficulty;                            // easy/medium/hard
    const char* category;                              // sql, python, etc.
    const char* company;                               // Associated company (e.g., "FAANG")
    const char* starter_code;                          // Initial code shown to user
    const char* solution;                              // Reference solution
    std::vector<const char*> tags;                     // Topic tags
    std::vector<const char*> hints;                    // Hints for the user
    std::vector<TableDef> tables;                      // Database schema
    std::unordered_map<const char*, std::vector<DataRow>> sample_data;  // Sample data per table
    std::vector<const char*> expected_columns;         // Expected output columns
    std::vector<DataRow> expected_rows;                // Expected output rows
};
```
Complete definition of a SQL practice question including schema, data, and expected results.

## Public API Functions

### `get_all_questions()`
```cpp
const std::vector<QuestionDef>& get_all_questions();
```
Returns a reference to all embedded questions.

### `find_question_by_slug()`
```cpp
const QuestionDef* find_question_by_slug(const std::string& slug);
```
Finds a question by its URL-friendly slug. Returns `nullptr` if not found.

### `find_question_by_id()`
```cpp
const QuestionDef* find_question_by_id(const std::string& id);
```
Finds a question by its ID (e.g., "q1"). Returns `nullptr` if not found.

### `get_question_count()`
```cpp
size_t get_question_count();
```
Returns the total number of embedded questions.

## Architecture Notes

### Zero-Dependency Design
- Questions are compiled directly into the binary
- No JSON files, Python scripts, or external data files required at runtime
- Simplifies deployment - single binary contains everything

### Memory Layout
- All strings are `const char*` pointers to string literals
- Stored in read-only memory segment
- Efficient memory usage - no dynamic allocation for question data

### Namespace
```cpp
namespace sql_practice {
namespace embedded {
    // All definitions
}
}
```
All embedded question definitions are in the `sql_practice::embedded` namespace to avoid conflicts.

## Related Files

- **embedded_questions.cpp**: Implementation with hardcoded question data
- **question_loader.cpp**: Converts embedded data to runtime Question objects
- **question_loader.hpp**: Defines the runtime Question structure

## Example Usage

```cpp
#include "db/embedded_questions.hpp"

using namespace sql_practice::embedded;

// Get all questions
const auto& questions = get_all_questions();
std::cout << "Total questions: " << get_question_count() << std::endl;

// Find specific question
const QuestionDef* q = find_question_by_slug("second-highest-salary");
if (q) {
    std::cout << "Question: " << q->title << std::endl;
    std::cout << "Difficulty: " << q->difficulty << std::endl;
}
```
