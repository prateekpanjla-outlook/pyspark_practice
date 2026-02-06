#include "include/question_loader.hpp"
#include "db/embedded_questions.hpp"
#include <algorithm>
#include <optional>
#include <unordered_set>

namespace sql_practice {

void QuestionLoader::load_embedded_questions() {
    // Load from embedded C++ data
    auto all_questions = embedded::get_all_questions();

    for (const auto& eq : all_questions) {
        Question q;

        q.id = eq.id ? eq.id : "";
        q.title = eq.title ? eq.title : "";
        q.slug = eq.slug ? eq.slug : "";
        q.description = eq.description ? eq.description : "";
        q.question_difficulty = eq.difficulty ? eq.difficulty : "medium";
        q.category = eq.category ? eq.category : "sql";
        q.company = eq.company ? eq.company : "";
        q.starter_code = eq.starter_code ? eq.starter_code : "";
        q.solution = eq.solution ? eq.solution : "";

        // Convert tags
        for (const auto& tag : eq.tags) {
            q.tags.push_back(tag);
        }

        // Convert hints
        for (const auto& hint : eq.hints) {
            q.hints.push_back(hint);
        }

        // Convert schema
        for (const auto& table : eq.tables) {
            QuestionSchema::Table schema_table;
            schema_table.name = table.name;

            for (const auto& col : table.columns) {
                QuestionSchema::Column schema_col;
                schema_col.name = col.name;
                schema_col.type = col.type;
                schema_table.columns.push_back(schema_col);
            }

            q.schema.tables.push_back(schema_table);

            // Convert sample data
            for (const auto& data_row : eq.sample_data) {
                std::vector<std::unordered_map<std::string, std::string>> rows;
                for (const auto& row : data_row.second) {
                    std::unordered_map<std::string, std::string> std_row;
                    for (const auto& [key, value] : row) {
                        std_row[key] = value;
                    }
                    rows.push_back(std_row);
                }
                q.schema.sample_data[data_row.first] = rows;
            }
        }

        // Convert expected output
        for (const auto& col : eq.expected_columns) {
            q.expected_output.columns.push_back(col);
        }
        for (const auto& row : eq.expected_rows) {
            std::unordered_map<std::string, std::string> std_row;
            for (const auto& [key, value] : row) {
                std_row[key] = value;
            }
            q.expected_output.rows.push_back(std_row);
        }
        // Mark expected output as valid for comparison
        q.expected_output.success = true;

        // Store in maps
        if (!q.id.empty()) {
            questions_by_id[q.id] = q;
        }
        if (!q.slug.empty()) {
            questions_by_slug[q.slug] = q;
        }
    }
}

std::optional<Question> QuestionLoader::get_question_by_slug(const std::string& slug) const {
    auto it = questions_by_slug.find(slug);
    if (it != questions_by_slug.end()) {
        return it->second;
    }
    return std::nullopt;
}

std::optional<Question> QuestionLoader::get_question_by_id(const std::string& id) const {
    auto it = questions_by_id.find(id);
    if (it != questions_by_id.end()) {
        return it->second;
    }
    return std::nullopt;
}

std::vector<Question> QuestionLoader::list_questions(
    const std::string& difficulty,
    const std::string& category,
    const std::string& tag,
    int skip,
    int limit
) const {
    std::vector<Question> result;

    for (const auto& [slug, question] : questions_by_slug) {
        // Apply filters
        if (!difficulty.empty() && question.question_difficulty != difficulty) {
            continue;
        }
        if (!category.empty() && question.category != category) {
            continue;
        }
        if (!tag.empty()) {
            auto it = std::find(question.tags.begin(), question.tags.end(), tag);
            if (it == question.tags.end()) {
                continue;
            }
        }

        result.push_back(question);
    }

    // Sort by difficulty then title
    std::sort(result.begin(), result.end(), [](const Question& a, const Question& b) {
        if (a.question_difficulty != b.question_difficulty) {
            return a.question_difficulty < b.question_difficulty;
        }
        return a.title < b.title;
    });

    // Apply pagination
    int start = std::min(skip, static_cast<int>(result.size()));
    int end = std::min(skip + limit, static_cast<int>(result.size()));

    if (start < static_cast<int>(result.size())) {
        return std::vector<Question>(result.begin() + start, result.begin() + end);
    }

    return {};
}

std::vector<std::string> QuestionLoader::get_all_tags() const {
    std::vector<std::string> tags;
    std::unordered_set<std::string> seen;

    for (const auto& [slug, question] : questions_by_slug) {
        for (const auto& tag : question.tags) {
            if (seen.find(tag) == seen.end()) {
                seen.insert(tag);
                tags.push_back(tag);
            }
        }
    }

    std::sort(tags.begin(), tags.end());
    return tags;
}

} // namespace sql_practice
