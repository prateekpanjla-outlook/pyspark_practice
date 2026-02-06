#ifndef QUESTION_LOADER_HPP
#define QUESTION_LOADER_HPP

#include "sql_executor.hpp"
#include <string>
#include <vector>
#include <unordered_map>
#include <optional>

namespace sql_practice {

/**
 * @brief Question definition
 */
struct Question {
    std::string id;
    std::string title;
    std::string slug;
    std::string description;
    std::string question_difficulty;  // easy, medium, hard
    std::string category;    // sql, python, etc.
    std::string company;     // FAANG, etc.
    QuestionSchema schema;
    QueryResult expected_output;
    std::string starter_code;
    std::vector<std::string> hints;
    std::string solution;    // Optional
    std::vector<std::string> tags;
};

/**
 * @brief Loads questions from embedded data
 *
 * Questions are compiled into the binary for zero-dependency deployment
 */
class QuestionLoader {
private:
    std::unordered_map<std::string, Question> questions_by_slug;
    std::unordered_map<std::string, Question> questions_by_id;

public:
    QuestionLoader() = default;

    /**
     * @brief Load all embedded questions
     * Called at startup from compiled-in data
     */
    void load_embedded_questions();

    /**
     * @brief Get question by slug
     */
    std::optional<Question> get_question_by_slug(const std::string& slug) const;

    /**
     * @brief Get question by ID
     */
    std::optional<Question> get_question_by_id(const std::string& id) const;

    /**
     * @brief List all questions with optional filtering
     */
    std::vector<Question> list_questions(
        const std::string& question_difficulty = "",
        const std::string& category = "",
        const std::string& tag = "",
        int skip = 0,
        int limit = 100
    ) const;

    /**
     * @brief Get all unique tags
     */
    std::vector<std::string> get_all_tags() const;

    /**
     * @brief Get total question count
     */
    size_t get_count() const {
        return questions_by_slug.size();
    }
};

} // namespace sql_practice

#endif // QUESTION_LOADER_HPP
