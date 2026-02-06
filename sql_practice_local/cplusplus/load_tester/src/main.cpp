#include "load_tester.hpp"
#include <iostream>
#include <cstdlib>

int main(int argc, char* argv[]) {
    // Default values
    std::string server_url = "http://localhost:8080";
    int num_users = 1000;

    // Parse command line arguments
    if (argc > 1) {
        server_url = argv[1];
    }
    if (argc > 2) {
        num_users = std::atoi(argv[2]);
        if (num_users <= 0) {
            std::cerr << "Number of users must be positive\n";
            return 1;
        }
    }

    std::cout << "\n";
    std::cout << "╔════════════════════════════════════════════════════════╗\n";
    std::cout << "║     SQL Practice Server - Load Testing Tool            ║\n";
    std::cout << "╚════════════════════════════════════════════════════════╝\n";
    std::cout << "\n";
    std::cout << "Configuration:\n";
    std::cout << "  Server URL: " << server_url << "\n";
    std::cout << "  Concurrent Users: " << num_users << "\n";
    std::cout << "  Attempts per User: 2 (wrong answer, then correct)\n";
    std::cout << "  Total Requests: " << (num_users * 2) << "\n";
    std::cout << "\n";

    // Create load tester
    load_test::LoadTester tester(server_url, num_users);

    // Initialize test cases
    tester.init_test_cases();

    // Create sessions for all users
    if (!tester.create_sessions()) {
        std::cerr << "\n❌ Failed to create sessions. Exiting.\n";
        return 1;
    }

    // Run load test
    tester.run();

    return 0;
}
