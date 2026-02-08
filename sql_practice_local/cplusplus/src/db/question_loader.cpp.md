# question_loader.cpp - Documentation

## Purpose

Converts embedded question data (from `embedded_questions.cpp`) into runtime `Question` objects that the rest of the application uses. Handles type conversions between compile-time `QuestionDef` structures and runtime `Question` structures.

## Key Class: QuestionLoader

### Responsibility
- Loads embedded questions at startup
- Stores questions in hash maps for fast lookup by ID or slug
- Provides filtered listing and query methods

### Data Members
```cpp
class QuestionLoader {
private:
    std::unordered_map<std::string, Question> questions_by_slug;  // Indexed by slug
    std::unordered_map<std::string, Question> questions_by_id;    // Indexed by ID
};
```

## Public Methods

### `load_embedded_questions()`
```cpp
void QuestionLoader::load_embedded_questions();
```
Loads all questions from embedded data into the hash maps. Called once at server startup.

**Conversion Process:**
1. Iterates through all embedded questions
2. Converts `const char*` strings to `std::string`
3. Converts `vector<const char*>` to `vector<string>`
4. Converts embedded data row format to runtime format
5. Stores in both `questions_by_id` and `questions_by_slug` maps

### `get_question_by_slug()`
```cpp
std::optional<Question> get_question_by_slug(const std::string& slug) const;
```
Retrieves a question by its URL-friendly slug. Returns `std::nullopt` if not found.

**Example:**
```cpp
auto q = loader.get_question_by_slug("second-highest-salary");
if (q) {
    std::cout << q->title << std::endl;
}
```

### `get_question_by_id()`
```cpp
std::optional<Question> get_question_by_id(const std::string& id) const;
```
Retrieves a question by its ID (e.g., "q1"). Returns `std::nullopt` if not found.

### `list_questions()`
```cpp
std::vector<Question> list_questions(
    const std::string& difficulty = "",
    const std::string& category = "",
    const std::string& tag = "",
    int skip = 0,
    int limit = 100
) const;
```
Lists questions with optional filtering and pagination.

**Filters:**
- `difficulty`: "easy", "medium", or "hard"
- `category`: "sql", "python", etc.
- `tag`: Specific tag (e.g., "window-functions")
- `skip`: Number of results to skip (pagination)
- `limit`: Maximum results to return

**Sorting:** Results are sorted by difficulty (ascending), then by title (alphabetically).

**Example:**
```cpp
// Get all medium difficulty SQL questions
auto questions = loader.list_questions("medium", "sql", "", 0, 10);
```

### `get_all_tags()`
```cpp
std::vector<std::string> get_all_tags() const;
```
Returns a sorted, unique list of all tags across all questions.

### `get_count()`
```cpp
size_t get_count() const;
```
Returns the total number of loaded questions.

## Data Conversion Details

### Schema Conversion
```cpp
// From: embedded::TableDef
struct TableDef {
    const char* name;
    std::vector<ColumnDef> columns;  // ColumnDef has const char*
};

// To: QuestionSchema::Table
struct Table {
    std::string name;
    std::vector<Column> columns;  // Column has std::string
};
```

### Sample Data Conversion
```cpp
// From: embedded::DataRow = unordered_map<const char*, const char*>
// To: unordered_map<std::string, std::string>
```

Each row's key-value pairs are converted from C-style strings to C++ strings.

## Architecture Notes

### Dual Indexing
Questions are stored in TWO hash maps:
1. `questions_by_id`: Fast lookup by question ID (q1, q2, etc.)
2. `questions_by_slug`: Fast lookup by URL slug (second-highest-salary, etc.)

This enables quick access regardless of which identifier is available.

### Null Safety
```cpp
q.id = eq.id ? eq.id : "";
q.title = eq.title ? eq.title : "";
```
All string fields are checked for null before assignment, defaulting to empty strings.

### Expected Output Success Flag
```cpp
q.expected_output.success = true;
```
The expected output is pre-marked as valid for comparison purposes since it comes from trusted embedded data.

## Dependencies

| Header | Purpose |
|--------|---------|
| `include/question_loader.hpp` | Main class definition |
| `db/embedded_questions.hpp` | Embedded question data |
| `<algorithm>` | std::sort, std::find |
| `<optional>` | std::optional |
| `<unordered_set>` | Tag deduplication |

## Related Files

- **question_loader.hpp**: Header with Question structure definitions
- **embedded_questions.cpp**: Source of embedded question data
- **embedded_questions.hpp**: Embedded data structure definitions
- **duckdb_executor.cpp**: Uses QuestionLoader to initialize schemas

## Usage Flow

```
Server Startup
       |
       v
QuestionLoader::load_embedded_questions()
       |
       v
SQLExecutor::initialize_all_schemas(&loader)
       |
       v
HTTP Requests -> get_question_by_slug() -> execute query
```
