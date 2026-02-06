/**
 * Embedded Questions Header
 * Pure C++ - No Python or JSON dependencies
 */

#ifndef EMBEDDED_QUESTIONS_HPP
#define EMBEDDED_QUESTIONS_HPP

#include <vector>
#include <unordered_map>
#include <string>
#include <cstddef>

namespace sql_practice {
namespace embedded {

// Forward declarations
struct ColumnDef;
struct TableDef;
struct QuestionDef;

// Column definition
struct ColumnDef {
    const char* name;
    const char* type;
};

// Table definition
struct TableDef {
    const char* name;
    std::vector<ColumnDef> columns;
};

// Sample data row
using DataRow = std::unordered_map<const char*, const char*>;

// Question definition
struct QuestionDef {
    const char* id;
    const char* title;
    const char* slug;
    const char* description;
    const char* difficulty;
    const char* category;
    const char* company;
    const char* starter_code;
    const char* solution;
    std::vector<const char*> tags;
    std::vector<const char*> hints;
    std::vector<TableDef> tables;
    std::unordered_map<const char*, std::vector<DataRow>> sample_data;
    std::vector<const char*> expected_columns;
    std::vector<DataRow> expected_rows;
};

/**
 * @brief Get all embedded questions
 */
const std::vector<QuestionDef>& get_all_questions();

/**
 * @brief Find question by slug
 */
const QuestionDef* find_question_by_slug(const std::string& slug);

/**
 * @brief Find question by ID
 */
const QuestionDef* find_question_by_id(const std::string& id);

/**
 * @brief Get question count
 */
size_t get_question_count();

} // namespace embedded
} // namespace sql_practice

#endif // EMBEDDED_QUESTIONS_HPP
