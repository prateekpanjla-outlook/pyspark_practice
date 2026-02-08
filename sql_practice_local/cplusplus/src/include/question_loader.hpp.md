# question_loader.hpp - Documentation

## Purpose

Header file defining the `Question` structure and `QuestionLoader` class for managing SQL practice questions. Provides the runtime question data model used throughout the application.

## Key Structures

### Question
```cpp
struct Question {
    std::string id;                        // Unique identifier (e.g., "q1")
    std::string title;                     // Display title
    std::string slug;                      // URL-friendly identifier
    std::string description;               // Problem description
    std::string question_difficulty;       // easy/medium/hard
    std::string category;                  // sql, python, etc.
    std::string company;                   // Associated company (FAANG, etc.)
    QuestionSchema schema;                 // Database schema
    QueryResult expected_output;           // Expected query results
    std::string starter_code;              // Initial code template
    std::vector<std::string> hints;        // User hints
    std::string solution;                  // Reference solution
    std::vector<std::string> tags;         // Topic tags
};
```
Complete runtime representation of a SQL practice question.

## Key Class: QuestionLoader

```cpp
class QuestionLoader {
private:
    std::unordered_map<std::string, Question> questions_by_slug;  // Slug index
    std::unordered_map<std::string, Question> questions_by_id;    // ID index

public:
    QuestionLoader() = default;

    void load_embedded_questions();

    std::optional<Question> get_question_by_slug(const std::string& slug) const;
    std::optional<Question> get_question_by_id(const std::string& id) const;

    std::vector<Question> list_questions(
        const std::string& question_difficulty = "",
        const std::string& category = "",
        const std::string& tag = "",
        int skip = 0,
        int limit = 100
    ) const;

    std::vector<std::string> get_all_tags() const;

    size_t get_count() const;
};
```

## Public Methods

### `load_embedded_questions()`
```cpp
void load_embedded_questions();
```
Loads all questions from embedded C++ data into the hash maps. Called once at server startup.

### `get_question_by_slug()`
```cpp
std::optional<Question> get_question_by_slug(const std::string& slug) const;
```
Retrieves a question by URL-friendly slug. Returns `std::nullopt` if not found.

**Example:**
```cpp
auto q = loader.get_question_by_slug("second-highest-salary");
if (q) {
    std::cout << q->title << std::endl;  // "Second Highest Salary"
}
```

### `get_question_by_id()`
```cpp
std::optional<Question> get_question_by_id(const std::string& id) const;
```
Retrieves a question by ID (e.g., "q1"). Returns `std::nullopt` if not found.

### `list_questions()`
```cpp
std::vector<Question> list_questions(
    const std::string& question_difficulty = "",
    const std::string& category = "",
    const std::string& tag = "",
    int skip = 0,
    int limit = 100
) const;
```
Lists questions with optional filtering and pagination.

**Parameters:**
- `question_difficulty`: Filter by "easy", "medium", or "hard"
- `category`: Filter by category (e.g., "sql")
- `tag`: Filter by specific tag (e.g., "window-functions")
- `skip`: Number of results to skip (for pagination)
- `limit`: Maximum results to return

**Returns:** Sorted vector of questions (by difficulty, then title)

### `get_all_tags()`
```cpp
std::vector<std::string> get_all_tags() const;
```
Returns a sorted list of all unique tags across all questions.

### `get_count()`
```cpp
size_t get_count() const;
```
Returns the total number of loaded questions.

## Architecture Notes

### Dual Indexing
Questions are stored in two hash maps for O(1) lookup:
- `questions_by_id`: Lookup by question ID
- `questions_by_slug`: Lookup by URL slug

### Default Constructor
```cpp
QuestionLoader() = default;
```
Compiler-generated default constructor. `load_embedded_questions()` must be called explicitly.

### std::optional Return Type
Getter methods return `std::optional<Question>` to gracefully handle missing questions:
```cpp
auto q = loader.get_question_by_slug("non-existent");
if (!q) {
    // Handle not found
}
```

## Dependencies

| Header | Purpose |
|--------|---------|
| `sql_executor.hpp` | QuestionSchema, QueryResult definitions |
| `<string>` | std::string |
| `<vector>` | std::vector |
| `<unordered_map>` | std::unordered_map |
| `<optional>` | std::optional |

## Related Files

- **question_loader.cpp**: Implementation
- **embedded_questions.hpp**: Compile-time question definitions
- **embedded_questions.cpp**: Hardcoded question data

## Usage Example

```cpp
#include "include/question_loader.hpp"

sql_practice::QuestionLoader loader;
loader.load_embedded_questions();

// Get a specific question
auto q = loader.get_question_by_slug("duplicate-emails");
if (q) {
    std::cout << "Title: " << q->title << std::endl;
    std::cout << "Difficulty: " << q->question_difficulty << std::endl;

    // Access schema
    for (const auto& table : q->schema.tables) {
        std::cout << "Table: " << table.name << std::endl;
    }
}

// List all easy questions
auto easy_questions = loader.list_questions("easy", "sql");
```
