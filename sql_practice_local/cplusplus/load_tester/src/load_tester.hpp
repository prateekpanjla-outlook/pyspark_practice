#ifndef LOAD_TESTER_HPP
#define LOAD_TESTER_HPP

#include <string>
#include <vector>
#include <random>
#include <chrono>
#include <atomic>
#include <mutex>

namespace load_test {

/**
 * @brief Test case with incorrect and correct SQL solutions
 */
struct TestCase {
    std::string question_id;
    std::string question_title;
    std::string wrong_sql;      // Incorrect solution (first attempt)
    std::string correct_sql;    // Correct solution (second attempt)

    TestCase(const std::string& id, const std::string& title,
             const std::string& wrong, const std::string& correct)
        : question_id(id), question_title(title),
          wrong_sql(wrong), correct_sql(correct) {}
};

/**
 * @brief Result of a single test execution
 */
struct TestResult {
    bool success;
    int64_t response_time_ms;
    std::string error_message;
    bool is_correct;

    TestResult() : success(false), response_time_ms(0), is_correct(false) {}
};

/**
 * @brief Statistics collected during load test
 */
struct LoadTestStats {
    std::atomic<int> total_attempts{0};
    std::atomic<int> successful_attempts{0};
    std::atomic<int> failed_attempts{0};
    std::atomic<int> correct_answers{0};
    std::atomic<int> wrong_answers{0};
    std::atomic<int64_t> total_response_time{0};
    std::atomic<int64_t> min_response_time{INT64_MAX};
    std::atomic<int64_t> max_response_time{0};

    void record_attempt(const TestResult& result) {
        total_attempts++;
        if (result.success) {
            successful_attempts++;
            total_response_time += result.response_time_ms;

            // Update min/max
            int64_t current = result.response_time_ms;
            int64_t old_min = min_response_time.load();
            while (current < old_min && !min_response_time.compare_exchange_weak(old_min, current)) {}

            int64_t old_max = max_response_time.load();
            while (current > old_max && !max_response_time.compare_exchange_weak(old_max, current)) {}
        } else {
            failed_attempts++;
        }

        if (result.is_correct) {
            correct_answers++;
        } else {
            wrong_answers++;
        }
    }

    void print() const {
        printf("\n");
        printf("╔════════════════════════════════════════════════════════╗\n");
        printf("║           LOAD TEST RESULTS                              ║\n");
        printf("╠════════════════════════════════════════════════════════╣\n");
        printf("║ Total Attempts:     %40d ║\n", total_attempts.load());
        printf("║ Successful:         %40d ║\n", successful_attempts.load());
        printf("║ Failed:             %40d ║\n", failed_attempts.load());
        printf("║ Correct Answers:    %40d ║\n", correct_answers.load());
        printf("║ Wrong Answers:      %40d ║\n", wrong_answers.load());
        printf("╠════════════════════════════════════════════════════════╣\n");

        if (successful_attempts.load() > 0) {
            int64_t avg = total_response_time.load() / successful_attempts.load();
            printf("║ Avg Response Time: %39lld ms ║\n", avg);
            printf("║ Min Response Time: %39lld ms ║\n", min_response_time.load());
            printf("║ Max Response Time: %39lld ms ║\n", max_response_time.load());
        }

        printf("╚════════════════════════════════════════════════════════╝\n\n");
    }
};

/**
 * @brief Load tester for SQL Practice Server
 */
class LoadTester {
private:
    std::string server_url;
    int num_users;
    std::vector<TestCase> test_cases;
    LoadTestStats stats;
    std::vector<std::string> session_tokens;
    std::mutex tokens_mutex;

    std::random_device rd;
    std::mt19937 gen;
    std::uniform_int_distribution<> dist;

public:
    LoadTester(const std::string& url, int users);
    ~LoadTester() = default;

    // Initialize test cases
    void init_test_cases();

    // Create sessions for all users
    bool create_sessions();

    // Run load test
    void run();

    // Get results
    const LoadTestStats& get_stats() const { return stats; }

private:
    // Worker thread for each user
    void worker_thread(int user_id);

    // Execute a single test
    TestResult execute_test(const std::string& session_token,
                           const std::string& question_id,
                           const std::string& sql);

    // HTTP helper functions
    std::string http_post(const std::string& url, const std::string& json_data);
    std::string escape_json(const std::string& str);
};

} // namespace load_test

#endif // LOAD_TESTER_HPP
