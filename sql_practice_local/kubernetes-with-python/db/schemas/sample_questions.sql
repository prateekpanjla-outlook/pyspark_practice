-- Sample SQL Practice Questions
-- Run this after the main database initialization

-- Question 1: Easy - Second Highest Salary (Classic FAANG question)
INSERT INTO questions (title, slug, description, difficulty, category, company, schema_data, expected_output, starter_code, hints)
VALUES (
    'Second Highest Salary',
    'second-highest-salary',
    'Given an Employee table, write a SQL query to find the second highest salary.',
    'easy',
    'sql',
    'FAANG',
    jsonb_build_object(
        'tables', jsonb_build_array(
            jsonb_build_object(
                'name', 'employees',
                'columns', jsonb_build_array(
                    jsonb_build_object('name', 'id', 'type', 'INTEGER'),
                    jsonb_build_object('name', 'name', 'type', 'VARCHAR'),
                    jsonb_build_object('name', 'salary', 'type', 'INTEGER'),
                    jsonb_build_object('name', 'department_id', 'type', 'INTEGER')
                )
            )
        ),
        'sample_data', jsonb_build_object(
            'employees', jsonb_build_array(
                jsonb_build_object('id', 1, 'name', 'Alice', 'salary', 100000, 'department_id', 1),
                jsonb_build_object('id', 2, 'name', 'Bob', 'salary', 90000, 'department_id', 2),
                jsonb_build_object('id', 3, 'name', 'Charlie', 'salary', 120000, 'department_id', 1),
                jsonb_build_object('id', 4, 'name', 'David', 'salary', 85000, 'department_id', 2)
            )
        )
    ),
    jsonb_build_object(
        'columns', jsonb_build_array('second_highest_salary'),
        'rows', jsonb_build_array(
            jsonb_build_object('second_highest_salary', 100000)
        )
    ),
    '-- Write your query here\nSELECT ',
    jsonb_build_array(
        'Use ORDER BY with DESC to sort salaries in descending order',
        'Use LIMIT and OFFSET to get the second row',
        'Or use MAX(salary) with a subquery to find salaries less than the maximum'
    )
);

-- Question 2: Easy - Duplicate Emails
INSERT INTO questions (title, slug, description, difficulty, category, company, schema_data, expected_output, starter_code, hints)
VALUES (
    'Duplicate Emails',
    'duplicate-emails',
    'Given a Person table, write a SQL query to find all duplicate emails.',
    'easy',
    'sql',
    'LinkedIn',
    jsonb_build_object(
        'tables', jsonb_build_array(
            jsonb_build_object(
                'name', 'person',
                'columns', jsonb_build_array(
                    jsonb_build_object('name', 'id', 'type', 'INTEGER'),
                    jsonb_build_object('name', 'email', 'type', 'VARCHAR')
                )
            )
        ),
        'sample_data', jsonb_build_object(
            'person', jsonb_build_array(
                jsonb_build_object('id', 1, 'email', 'alice@example.com'),
                jsonb_build_object('id', 2, 'email', 'bob@example.com'),
                jsonb_build_object('id', 3, 'email', 'alice@example.com'),
                jsonb_build_object('id', 4, 'email', 'charlie@example.com'),
                jsonb_build_object('id', 5, 'email', 'bob@example.com')
            )
        )
    ),
    jsonb_build_object(
        'columns', jsonb_build_array('email'),
        'rows', jsonb_build_array(
            jsonb_build_object('email', 'alice@example.com'),
            jsonb_build_object('email', 'bob@example.com')
        )
    ),
    'SELECT email',
    jsonb_build_array(
        'Use GROUP BY email to group identical emails',
        'Use HAVING COUNT(*) > 1 to find emails that appear more than once'
    )
);

-- Question 3: Medium - Department Highest Salary
INSERT INTO questions (title, slug, description, difficulty, category, company, schema_data, expected_output, starter_code, hints)
VALUES (
    'Department Highest Salary',
    'department-highest-salary',
    'Find employees who earn the highest salary in their department.',
    'medium',
    'sql',
    'Amazon',
    jsonb_build_object(
        'tables', jsonb_build_array(
            jsonb_build_object(
                'name', 'employee',
                'columns', jsonb_build_array(
                    jsonb_build_object('name', 'id', 'type', 'INTEGER'),
                    jsonb_build_object('name', 'name', 'type', 'VARCHAR'),
                    jsonb_build_object('name', 'salary', 'type', 'INTEGER'),
                    jsonb_build_object('name', 'department_id', 'type', 'INTEGER')
                )
            ),
            jsonb_build_object(
                'name', 'department',
                'columns', jsonb_build_array(
                    jsonb_build_object('name', 'id', 'type', 'INTEGER'),
                    jsonb_build_object('name', 'name', 'type', 'VARCHAR')
                )
            )
        ),
        'sample_data', jsonb_build_object(
            'employee', jsonb_build_array(
                jsonb_build_object('id', 1, 'name', 'Alice', 'salary', 90000, 'department_id', 1),
                jsonb_build_object('id', 2, 'name', 'Bob', 'salary', 85000, 'department_id', 2),
                jsonb_build_object('id', 3, 'name', 'Charlie', 'salary', 95000, 'department_id', 1)
            ),
            'department', jsonb_build_array(
                jsonb_build_object('id', 1, 'name', 'Engineering'),
                jsonb_build_object('id', 2, 'name', 'Sales')
            )
        )
    ),
    jsonb_build_object(
        'columns', jsonb_build_array('department', 'employee', 'salary'),
        'rows', jsonb_build_array(
            jsonb_build_object('department', 'Engineering', 'employee', 'Charlie', 'salary', 95000),
            jsonb_build_object('department', 'Sales', 'employee', 'Bob', 'salary', 85000)
        )
    ),
    'SELECT ',
    jsonb_build_array(
        'Use a CTE or subquery to find MAX salary per department',
        'JOIN with department table to get department names',
        'Filter employees whose salary matches their department maximum'
    )
);

-- Add tags for these questions
INSERT INTO question_tags (question_id, tag)
SELECT id, 'window-functions'
FROM questions
WHERE slug IN ('department-highest-salary');

INSERT INTO question_tags (question_id, tag)
SELECT id, 'joins'
FROM questions
WHERE slug IN ('department-highest-salary');

INSERT INTO question_tags (question_id, tag)
SELECT id, 'aggregate-functions'
FROM questions
WHERE slug IN ('second-highest-salary', 'duplicate-emails');

INSERT INTO question_tags (question_id, tag)
SELECT id, 'group-by'
FROM questions
WHERE slug IN ('duplicate-emails');

INSERT INTO question_tags (question_id, tag)
SELECT id, 'having-clause'
FROM questions
WHERE slug IN ('duplicate-emails');
