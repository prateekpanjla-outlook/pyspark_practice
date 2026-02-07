#ifndef LOAD_TESTER_HPP
#define LOAD_TESTER_HPP

#include <string>
#include <vector>
#include <random>
#include <chrono>
#include <atomic>
#include <mutex>
#include <algorithm>
#include <cstdint>
#include <climits>
#include <curl/curl.h>

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
    bool timeout;  // True if request timed out
    long long response_time_ms;
    std::string error_message;
    bool is_correct;

    TestResult() : success(false), timeout(false), response_time_ms(0), is_correct(false) {}
};

/**
 * @brief Statistics collected during load test
 */
struct LoadTestStats {
    std::atomic<int> total_attempts{0};
    std::atomic<int> successful_attempts{0};
    std::atomic<int> failed_attempts{0};
    std::atomic<int> timeouts{0};  // Requests that timed out
    std::atomic<int> other_failures{0};  // Other failures (not timeouts)
    std::atomic<int> correct_answers{0};
    std::atomic<int> wrong_answers{0};
    std::atomic<long long> total_response_time{0};
    std::atomic<long long> min_response_time{LLONG_MAX};
    std::atomic<long long> max_response_time{0};

    // Thread-safe storage for response times (for percentile calculation)
    std::vector<long long> response_times;
    std::mutex response_times_mutex;

    void record_attempt(const TestResult& result) {
        total_attempts++;
        if (result.success) {
            successful_attempts++;
            total_response_time += result.response_time_ms;

            // Store response time for percentile calculation
            {
                std::lock_guard<std::mutex> lock(response_times_mutex);
                response_times.push_back(result.response_time_ms);
            }

            // Update min/max
            long long current = result.response_time_ms;
            long long old_min = min_response_time.load();
            while (current < old_min && !min_response_time.compare_exchange_weak(old_min, current)) {}

            long long old_max = max_response_time.load();
            while (current > old_max && !max_response_time.compare_exchange_weak(old_max, current)) {}
        } else {
            failed_attempts++;
            if (result.timeout) {
                timeouts++;
            } else {
                other_failures++;
            }
        }

        if (result.is_correct) {
            correct_answers++;
        } else {
            wrong_answers++;
        }
    }

    // Calculate percentile
    long long get_percentile(int percentile) const {
        if (response_times.empty()) return 0;

        std::vector<long long> sorted_times = response_times;
        std::sort(sorted_times.begin(), sorted_times.end());

        size_t index = (sorted_times.size() * percentile) / 100;
        if (index >= sorted_times.size()) index = sorted_times.size() - 1;

        return sorted_times[index];
    }

    void print() const {
        printf("\n");
        printf("╔════════════════════════════════════════════════════════╗\n");
        printf("║           LOAD TEST RESULTS                              ║\n");
        printf("╠════════════════════════════════════════════════════════╣\n");
        printf("║ Total Attempts:     %40d ║\n", total_attempts.load());
        printf("║ Successful:         %40d ║\n", successful_attempts.load());
        printf("║ Failed:             %40d ║\n", failed_attempts.load());
        printf("║   - Timeouts:       %40d ║\n", timeouts.load());
        printf("║   - Other Errors:   %40d ║\n", other_failures.load());
        printf("║ Correct Answers:    %40d ║\n", correct_answers.load());
        printf("║ Wrong Answers:      %40d ║\n", wrong_answers.load());
        printf("╠════════════════════════════════════════════════════════╣\n");

        if (successful_attempts.load() > 0) {
            long long avg = total_response_time.load() / successful_attempts.load();
            printf("║ Avg Response Time: %39lld ms ║\n", avg);
            printf("║ Min Response Time: %39lld ms ║\n", min_response_time.load());
            printf("║ Max Response Time: %39lld ms ║\n", max_response_time.load());

            long long p95 = get_percentile(95);
            printf("║ 95th Percentile:   %39lld ms ║\n", p95);
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
    LoadTester(const std::string& url, int users)
        : server_url(url), num_users(users), gen(rd()), dist(0, 2) {
        curl_global_init(CURL_GLOBAL_ALL);
    }

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
