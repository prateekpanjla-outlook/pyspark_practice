# Test the fixed schema with capitalized table names

# Login
$login = curl.exe -s -X POST -H 'Content-Type: application/json' -d '{"user_id":"test_fix"}' http://localhost:8080/api/login | ConvertFrom-Json
$token = $login.session_token

Write-Host "Token: $token"

# Test correct SQL with capitalized table name Employee
$correct_sql = "SELECT MAX(salary) AS SecondHighestSalary FROM Employee WHERE salary < (SELECT MAX(salary) FROM Employee)"
$result1 = curl.exe -s -X POST -H 'Content-Type: application/json' -d "{`"session_token`":`"$token`",`"question_id`":`"q1`",`"user_sql`":`"$correct_sql`"}" http://localhost:8080/api/execute | ConvertFrom-Json

Write-Host "`nTest 1: Correct SQL with 'Employee' table"
Write-Host "Is Correct: $($result1.is_correct)"
Write-Host "Result: $($result1.rows | ConvertTo-Json -Compress)"

# Test wrong SQL
$wrong_sql = "SELECT MAX(salary) FROM Employee"
$result2 = curl.exe -s -X POST -H 'Content-Type: application/json' -d "{`"session_token`":`"$token`",`"question_id`":`"q1`",`"user_sql`":`"$wrong_sql`"}" http://localhost:8080/api/execute | ConvertFrom-Json

Write-Host "`nTest 2: Wrong SQL (gets highest salary)"
Write-Host "Is Correct: $($result2.is_correct)"
Write-Host "Result: $($result2.rows | ConvertTo-Json -Compress)"
