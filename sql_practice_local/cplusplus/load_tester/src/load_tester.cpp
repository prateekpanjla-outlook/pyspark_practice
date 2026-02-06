#include "load_tester.hpp"
#include <curl/curl.h>
#include <sstream>
#include <iostream>
#include <thread>
#include <cstring>

namespace load_test {

// Callback for curl to write response data
static size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp) {
    ((std::string*)userp)->append((char*)contents, size * nmemb);
    return size * nmemb;
}

LoadTester::LoadTester(const std::string& url, int users)
    : server_url(url), num_users(users), gen(rd()), dist(0, 2) {
    curl_global_init(CURL_GLOBAL_ALL);
}

void LoadTester::init_test_cases() {
    // Test cases with wrong and correct SQL solutions
    // Note: Table names are capitalized (Employee, Person, Logs) to match expected solutions
    test_cases = {
        TestCase("q1", "Second Highest Salary",
            "SELECT MAX(salary) FROM Employee",  // Wrong - gets highest
            "SELECT MAX(salary) AS SecondHighestSalary FROM Employee WHERE salary < (SELECT MAX(salary) FROM Employee)"),

        TestCase("q2", "Duplicate Emails",
            "SELECT email FROM Person",  // Wrong - returns all emails
            "SELECT email FROM Person GROUP BY email HAVING COUNT(*) > 1"),

        TestCase("q8", "Consecutive Numbers",
            "SELECT DISTINCT num FROM Logs",  // Wrong - just distinct numbers
            "SELECT DISTINCT l1.num AS consecutive_numbers FROM Logs l1 "
            "JOIN Logs l2 ON l1.id = l2.id - 1 AND l1.num = l2.num "
            "JOIN Logs l3 ON l1.id = l3.id - 2 AND l1.num = l3.num")
    };

    // For now, just test with 3 questions that we know work
    // We can add more after verifying these work correctly
}

bool LoadTester::create_sessions() {
    printf("Creating %d user sessions...\n", num_users);

    std::string login_url = server_url + "/api/login";

    for (int i = 0; i < num_users; i++) {
        std::string user_id = "load_test_user_" + std::to_string(i);
        std::string json_data = "{\"user_id\":\"" + user_id + "\"}";

        std::string response = http_post(login_url, json_data);

        // Parse session token from response
        // Response format: {"session_token":"sess_...","user_id":"..."}
        size_t token_pos = response.find("\"session_token\":\"");
        if (token_pos != std::string::npos) {
            token_pos += 17; // Skip "session_token":"
            size_t token_end = response.find("\"", token_pos);
            if (token_end != std::string::npos) {
                std::string token = response.substr(token_pos, token_end - token_pos);
                session_tokens.push_back(token);
                if ((i + 1) % 100 == 0) {
                    printf("  Created %d sessions...\n", i + 1);
                }
                continue;
            }
        }

        fprintf(stderr, "Failed to create session for user %d\n", i);
        return false;
    }

    printf("✅ All %d sessions created successfully\n", num_users);
    return true;
}

void LoadTester::run() {
    printf("\n🚀 Starting load test with %d concurrent users...\n", num_users);
    printf("Server: %s\n", server_url.c_str());
    printf("Each user will run 2 attempts (wrong answer, then correct answer)\n\n");

    auto start_time = std::chrono::steady_clock::now();

    // Launch worker threads
    std::vector<std::thread> threads;
    for (int i = 0; i < num_users; i++) {
        threads.emplace_back([this, i]() { this->worker_thread(i); });
    }

    // Wait for all threads to complete
    for (auto& t : threads) {
        t.join();
    }

    auto end_time = std::chrono::steady_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);

    printf("\n⏱️  Total test duration: %lld ms\n", elapsed.count());
    printf("📊 Throughput: %.2f requests/second\n\n",
           (double)(num_users * 2) / (elapsed.count() / 1000.0));

    stats.print();
}

void LoadTester::worker_thread(int user_id) {
    // Get this user's session token
    std::string token;
    {
        std::lock_guard<std::mutex> lock(tokens_mutex);
        if (user_id < (int)session_tokens.size()) {
            token = session_tokens[user_id];
        }
    }

    if (token.empty()) {
        fprintf(stderr, "User %d: No session token available\n", user_id);
        return;
    }

    // Pick a random question
    int question_idx = dist(gen);
    const auto& test_case = test_cases[question_idx];

    // First attempt: Wrong answer
    TestResult result1 = execute_test(token, test_case.question_id, test_case.wrong_sql);
    stats.record_attempt(result1);

    // Small delay between attempts
    std::this_thread::sleep_for(std::chrono::milliseconds(10));

    // Second attempt: Correct answer
    TestResult result2 = execute_test(token, test_case.question_id, test_case.correct_sql);
    stats.record_attempt(result2);

    if (user_id < 5 || user_id % 100 == 0) {
        printf("User %d [%s]: Attempt 1: %s (%lld ms), Attempt 2: %s (%lld ms)\n",
               user_id, test_case.question_title.c_str(),
               result1.is_correct ? "✅" : "❌", result1.response_time_ms,
               result2.is_correct ? "✅" : "❌", result2.response_time_ms);
    }
}

TestResult LoadTester::execute_test(const std::string& session_token,
                                    const std::string& question_id,
                                    const std::string& sql) {
    TestResult result;
    auto start_time = std::chrono::steady_clock::now();

    std::string execute_url = server_url + "/api/execute";

    // Build JSON payload
    std::stringstream json;
    json << "{"
         << "\"session_token\":\"" << escape_json(session_token) << "\","
         << "\"question_id\":\"" << escape_json(question_id) << "\","
         << "\"user_sql\":\"" << escape_json(sql) << "\""
         << "}";

    std::string response = http_post(execute_url, json.str());

    auto end_time = std::chrono::steady_clock::now();
    result.response_time_ms =
        std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time).count();

    // Parse response
    // Expected: {"success":true,"is_correct":true/false,...}
    if (response.find("\"success\":true") != std::string::npos ||
        response.find("\"is_correct\"") != std::string::npos) {
        result.success = true;
        result.is_correct = (response.find("\"is_correct\":true") != std::string::npos);
    } else {
        result.success = false;
        result.error_message = response;
    }

    return result;
}

std::string LoadTester::http_post(const std::string& url, const std::string& json_data) {
    CURL* curl = curl_easy_init();
    if (!curl) {
        return "{\"error\":\"Failed to initialize CURL\"}";
    }

    std::string response;

    struct curl_slist* headers = nullptr;
    headers = curl_slist_append(headers, "Content-Type: application/json");

    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, json_data.c_str());
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 30L);

    CURLcode res = curl_easy_perform(curl);

    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);

    if (res != CURLE_OK) {
        return "{\"error\":\"" + std::string(curl_easy_strerror(res)) + "\"}";
    }

    return response;
}

std::string LoadTester::escape_json(const std::string& str) {
    std::string escaped;
    for (char c : str) {
        switch (c) {
            case '"':  escaped += "\\\""; break;
            case '\\': escaped += "\\\\"; break;
            case '\n': escaped += "\\n"; break;
            case '\r': escaped += "\\r"; break;
            case '\t': escaped += "\\t"; break;
            default:   escaped += c; break;
        }
    }
    return escaped;
}

} // namespace load_test
