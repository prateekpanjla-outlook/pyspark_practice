/**
 * Embedded Questions - Pure C++ Implementation
 * No Python or JSON required - all questions hardcoded here
 */

#include "db/embedded_questions.hpp"

namespace sql_practice {
namespace embedded {

// =============================================================================
// QUESTION 1: Second Highest Salary
// =============================================================================

static const QuestionDef question_1 = {
    "q1",
    "Second Highest Salary",
    "second-highest-salary",
    "Given an Employee table, write a SQL query to find the second highest salary.",
    "easy",
    "sql",
    "FAANG",
    "-- Write your query here\nSELECT ",
    "SELECT MAX(salary) AS SecondHighestSalary FROM Employee WHERE salary < (SELECT MAX(salary) FROM Employee)",
    {"aggregate-functions", "subqueries"},
    {
        "Use ORDER BY with DESC to sort salaries in descending order",
        "Use LIMIT and OFFSET to get the second row",
        "Or use MAX(salary) with a subquery to find salaries less than the maximum"
    },
    {
        {
            "employees",
            {
                {"id", "INTEGER"},
                {"name", "VARCHAR"},
                {"salary", "INTEGER"},
                {"department_id", "INTEGER"}
            }
        }
    },
    {
        {"employees", {
            {{"id", "1"}, {"name", "Alice"}, {"salary", "100000"}, {"department_id", "1"}},
            {{"id", "2"}, {"name", "Bob"}, {"salary", "90000"}, {"department_id", "2"}},
            {{"id", "3"}, {"name", "Charlie"}, {"salary", "120000"}, {"department_id", "1"}},
            {{"id", "4"}, {"name", "David"}, {"salary", "85000"}, {"department_id", "2"}}
        }}
    },
    {"second_highest_salary"},
    {{{"second_highest_salary", "100000"}}}
};

// =============================================================================
// QUESTION 2: Duplicate Emails
// =============================================================================

static const QuestionDef question_2 = {
    "q2",
    "Duplicate Emails",
    "duplicate-emails",
    "Given a Person table, write a SQL query to find all duplicate emails.",
    "easy",
    "sql",
    "LinkedIn",
    "SELECT email",
    "SELECT email FROM Person GROUP BY email HAVING COUNT(*) > 1",
    {"group-by", "having-clause", "aggregate-functions"},
    {
        "Use GROUP BY email to group identical emails",
        "Use HAVING COUNT(*) > 1 to find emails that appear more than once"
    },
    {
        {
            "person",
            {
                {"id", "INTEGER"},
                {"email", "VARCHAR"}
            }
        }
    },
    {
        {"person", {
            {{"id", "1"}, {"email", "alice@example.com"}},
            {{"id", "2"}, {"email", "bob@example.com"}},
            {{"id", "3"}, {"email", "alice@example.com"}},
            {{"id", "4"}, {"email", "charlie@example.com"}},
            {{"id", "5"}, {"email", "bob@example.com"}}
        }}
    },
    {"email"},
    {
        {{"email", "alice@example.com"}},
        {{"email", "bob@example.com"}}
    }
};

// =============================================================================
// QUESTION 3: Department Highest Salary
// =============================================================================

static const QuestionDef question_3 = {
    "q3",
    "Department Highest Salary",
    "department-highest-salary",
    "Find employees who earn the highest salary in their department.",
    "medium",
    "sql",
    "Amazon",
    "SELECT ",
    "SELECT d.name as department, e.name as employee, e.salary FROM Employee e JOIN Department d ON e.department_id = d.id WHERE (e.department_id, e.salary) IN (SELECT department_id, MAX(salary) FROM Employee GROUP BY department_id)",
    {"window-functions", "joins", "group-by"},
    {
        "Use a CTE or subquery to find MAX salary per department",
        "JOIN with department table to get department names",
        "Filter employees whose salary matches their department maximum"
    },
    {
        {
            "employee",
            {
                {"id", "INTEGER"},
                {"name", "VARCHAR"},
                {"salary", "INTEGER"},
                {"department_id", "INTEGER"}
            }
        },
        {
            "department",
            {
                {"id", "INTEGER"},
                {"name", "VARCHAR"}
            }
        }
    },
    {
        {"employee", {
            {{"id", "1"}, {"name", "Alice"}, {"salary", "90000"}, {"department_id", "1"}},
            {{"id", "2"}, {"name", "Bob"}, {"salary", "85000"}, {"department_id", "2"}},
            {{"id", "3"}, {"name", "Charlie"}, {"salary", "95000"}, {"department_id", "1"}}
        }},
        {"department", {
            {{"id", "1"}, {"name", "Engineering"}},
            {{"id", "2"}, {"name", "Sales"}}
        }}
    },
    {"department", "employee", "salary"},
    {
        {{"department", "Engineering"}, {"employee", "Charlie"}, {"salary", "95000"}},
        {{"department", "Sales"}, {"employee", "Bob"}, {"salary", "85000"}}
    }
};

// =============================================================================
// QUESTION 4: Employees Earning More Than Their Manager
// =============================================================================

static const QuestionDef question_4 = {
    "q4",
    "Employees Earning More Than Their Manager",
    "employees-earning-more-than-manager",
    "Find all employees who earn more than their direct manager.",
    "easy",
    "sql",
    "Amazon",
    "SELECT ",
    "SELECT e.name FROM Employee e JOIN Employee m ON e.manager_id = m.id WHERE e.salary > m.salary",
    {"joins", "self-join"},
    {
        "Join the Employee table with itself on manager_id = id",
        "Compare salary of employee (e) with salary of manager (m)"
    },
    {
        {
            "employee",
            {
                {"id", "INTEGER"},
                {"name", "VARCHAR"},
                {"salary", "INTEGER"},
                {"manager_id", "INTEGER"}
            }
        }
    },
    {
        {"employee", {
            {{"id", "1"}, {"name", "Alice"}, {"salary", "100000"}, {"manager_id", "3"}},
            {{"id", "2"}, {"name", "Bob"}, {"salary", "90000"}, {"manager_id", "3"}},
            {{"id", "3"}, {"name", "Charlie"}, {"salary", "85000"}, {"manager_id", "4"}},
            {{"id", "4"}, {"name", "David"}, {"salary", "80000"}, {"manager_id", "NULL"}}
        }}
    },
    {"name"},
    {
        {{"name", "Alice"}}
    }
};

// =============================================================================
// QUESTION 5: Nth Highest Salary
// =============================================================================

static const QuestionDef question_5 = {
    "q5",
    "Nth Highest Salary",
    "nth-highest-salary",
    "Given an Employee table, write a SQL query to get the nth highest salary.",
    "medium",
    "sql",
    "Facebook",
    "CREATE FUNCTION getNthHighestSalary(N INT) ",
    "CREATE FUNCTION getNthHighestSalary(N INT) RETURNS INT BEGIN DECLARE M INT; SET M = N - 1; RETURN (SELECT DISTINCT salary FROM Employee ORDER BY salary DESC LIMIT 1 OFFSET M); END",
    {"window-functions", "limit-offset", "dense-rank"},
    {
        "Use DENSE_RANK() or ROW_NUMBER() window function",
        "Or use LIMIT 1 OFFSET N-1 after ordering by salary DESC",
        "Use DISTINCT to handle duplicate salaries"
    },
    {
        {
            "employee",
            {
                {"id", "INTEGER"},
                {"name", "VARCHAR"},
                {"salary", "INTEGER"}
            }
        }
    },
    {
        {"employee", {
            {{"id", "1"}, {"name", "Alice"}, {"salary", "100000"}},
            {{"id", "2"}, {"name", "Bob"}, {"salary", "100000"}},
            {{"id", "3"}, {"name", "Charlie"}, {"salary", "90000"}},
            {{"id", "4"}, {"name", "David"}, {"salary", "85000"}}
        }}
    },
    {"get_nth_highest_salary"},
    {{{"get_nth_highest_salary", "90000"}}}
};

// =============================================================================
// QUESTION 6: Rank Scores
// =============================================================================

static const QuestionDef question_6 = {
    "q6",
    "Rank Scores",
    "rank-scores",
    "Write a SQL query to rank scores. If there is a tie between two scores, both should have the same ranking.",
    "medium",
    "sql",
    "LeetCode",
    "SELECT ",
    "SELECT score, DENSE_RANK() OVER (ORDER BY score DESC) AS rank FROM Scores",
    {"window-functions", "dense-rank"},
    {
        "Use DENSE_RANK() for consecutive ranking (1,2,2,3)",
        "Use RANK() for standard competition ranking (1,2,2,4)",
        "Use ROW_NUMBER() for unique ranking (1,2,3,4)"
    },
    {
        {
            "scores",
            {
                {"id", "INTEGER"},
                {"score", "INTEGER"}
            }
        }
    },
    {
        {"scores", {
            {{"id", "1"}, {"score", "95"}},
            {{"id", "2"}, {"score", "85"}},
            {{"id", "3"}, {"score", "95"}},
            {{"id", "4"}, {"score", "75"}}
        }}
    },
    {"score", "rank"},
    {
        {{"score", "95"}, {"rank", "1"}},
        {{"score", "95"}, {"rank", "1"}},
        {{"score", "85"}, {"rank", "2"}},
        {{"score", "75"}, {"rank", "3"}}
    }
};

// =============================================================================
// QUESTION 7: Delete Duplicate Emails
// =============================================================================

static const QuestionDef question_7 = {
    "q7",
    "Delete Duplicate Emails",
    "delete-duplicate-emails",
    "Delete all duplicate emails from the Person table, keeping only the one with the smallest ID.",
    "medium",
    "sql",
    "Google",
    "DELETE p1 FROM Person p1, Person p2 ",
    "DELETE p1 FROM Person p1, Person p2 WHERE p1.email = p2.email AND p1.id > p2.id",
    {"delete", "self-join"},
    {
        "Join Person table with itself on email",
        "Delete rows where id is greater than the duplicate's id",
        "This keeps only the row with the smallest id for each email"
    },
    {
        {
            "person",
            {
                {"id", "INTEGER"},
                {"email", "VARCHAR"}
            }
        }
    },
    {
        {"person", {
            {{"id", "1"}, {"email", "alice@example.com"}},
            {{"id", "2"}, {"email", "bob@example.com"}},
            {{"id", "3"}, {"email", "alice@example.com"}}
        }}
    },
    {"id", "email"},
    {
        {{"id", "1"}, {"email", "alice@example.com"}},
        {{"id", "2"}, {"email", "bob@example.com"}}
    }
};

// =============================================================================
// QUESTION 8: Consecutive Numbers
// =============================================================================

static const QuestionDef question_8 = {
    "q8",
    "Consecutive Numbers",
    "consecutive-numbers",
    "Find all numbers that appear at least three times consecutively.",
    "medium",
    "sql",
    "Microsoft",
    "SELECT ",
    "SELECT DISTINCT l1.num FROM Logs l1 JOIN Logs l2 ON l1.id = l2.id - 1 AND l1.num = l2.num JOIN Logs l3 ON l1.id = l3.id - 2 AND l1.num = l3.num",
    {"joins", "self-join"},
    {
        "Join the Logs table with itself twice",
        "First join: current row with next row (id + 1)",
        "Second join: current row with row after next (id + 2)",
        "Check if all three have the same num value"
    },
    {
        {
            "logs",
            {
                {"id", "INTEGER"},
                {"num", "INTEGER"}
            }
        }
    },
    {
        {"logs", {
            {{"id", "1"}, {"num", "1"}},
            {{"id", "2"}, {"num", "1"}},
            {{"id", "3"}, {"num", "1"}},
            {{"id", "4"}, {"num", "2"}},
            {{"id", "5"}, {"num", "1"}},
            {{"id", "6"}, {"num", "2"}},
            {{"id", "7"}, {"num", "2"}}
        }}
    },
    {"consecutive_numbers"},
    {{{"consecutive_numbers", "1"}}}
};

// =============================================================================
// QUESTION 9: Customers With Largest Revenue
// =============================================================================

static const QuestionDef question_9 = {
    "q9",
    "Customers With Largest Revenue",
    "customers-largest-revenue",
    "Find the customer with the largest total revenue (sum of all their orders).",
    "easy",
    "sql",
    "Amazon",
    "SELECT ",
    "SELECT customer_id FROM Orders GROUP BY customer_id ORDER BY SUM(amount) DESC LIMIT 1",
    {"aggregate-functions", "group-by", "order-by"},
    {
        "Use SUM(amount) to calculate total revenue per customer",
        "Use GROUP BY customer_id to group orders by customer",
        "Use ORDER BY with DESC and LIMIT 1 to get the top customer"
    },
    {
        {
            "orders",
            {
                {"id", "INTEGER"},
                {"customer_id", "INTEGER"},
                {"amount", "INTEGER"}
            }
        }
    },
    {
        {"orders", {
            {{"id", "1"}, {"customer_id", "1"}, {"amount", "100"}},
            {{"id", "2"}, {"customer_id", "2"}, {"amount", "200"}},
            {{"id", "3"}, {"customer_id", "1"}, {"amount", "150"}},
            {{"id", "4"}, {"customer_id", "3"}, {"amount", "300"}}
        }}
    },
    {"customer_id"},
    {{{"customer_id", "1"}}}
};

// =============================================================================
// QUESTION 10: Actors Who Never Appeared Together
// =============================================================================

static const QuestionDef question_10 = {
    "q10",
    "Actors Who Never Appeared Together",
    "actors-never-together",
    "Find pairs of actors who have never appeared in a movie together.",
    "hard",
    "advanced-sql",
    "Netflix",
    "SELECT ",
    "SELECT DISTINCT a1.actor_id, a2.actor_id FROM Actor a1 CROSS JOIN Actor a2 WHERE a1.actor_id < a2.actor_id AND NOT EXISTS (SELECT 1 FROM Movie_Actor ma1 JOIN Movie_Actor ma2 ON ma1.movie_id = ma2.movie_id WHERE ma1.actor_id = a1.actor_id AND ma2.actor_id = a2.actor_id)",
    {"cross-join", "not-exists", "subqueries"},
    {
        "Use CROSS JOIN to get all possible actor pairs",
        "Use NOT EXISTS to filter out pairs who appeared together",
        "Compare actor_id to avoid duplicate pairs (a1 < a2)"
    },
    {
        {
            "actor",
            {
                {"actor_id", "INTEGER"},
                {"name", "VARCHAR"}
            }
        },
        {
            "movie_actor",
            {
                {"movie_id", "INTEGER"},
                {"actor_id", "INTEGER"}
            }
        }
    },
    {
        {"actor", {
            {{"actor_id", "1"}, {"name", "Tom Hanks"}},
            {{"actor_id", "2"}, {"name", "Meryl Streep"}},
            {{"actor_id", "3"}, {"name", "Leonardo DiCaprio"}}
        }},
        {"movie_actor", {
            {{"movie_id", "1"}, {"actor_id", "1"}},
            {{"movie_id", "1"}, {"actor_id", "2"}}
        }}
    },
    {"actor1_id", "actor2_id"},
    {
        {{"actor1_id", "1"}, {"actor2_id", "3"}},
        {{"actor1_id", "2"}, {"actor2_id", "3"}}
    }
};

// =============================================================================
// All Questions Array
// =============================================================================

static const std::vector<QuestionDef> ALL_QUESTIONS = {
    question_1,
    question_2,
    question_3,
    question_4,
    question_5,
    question_6,
    question_7,
    question_8,
    question_9,
    question_10
};

// =============================================================================
// Getter functions
// =============================================================================

const std::vector<QuestionDef>& get_all_questions() {
    return ALL_QUESTIONS;
}

const QuestionDef* find_question_by_slug(const std::string& slug) {
    for (const auto& q : ALL_QUESTIONS) {
        if (slug == q.slug) {
            return &q;
        }
    }
    return nullptr;
}

const QuestionDef* find_question_by_id(const std::string& id) {
    for (const auto& q : ALL_QUESTIONS) {
        if (id == q.id) {
            return &q;
        }
    }
    return nullptr;
}

size_t get_question_count() {
    return ALL_QUESTIONS.size();
}

} // namespace embedded
} // namespace sql_practice
