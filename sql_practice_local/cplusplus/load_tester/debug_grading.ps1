# Debug grading issue

# 1. Login
$login = curl.exe -s -X POST -H 'Content-Type: application/json' -d '{"user_id":"debug_user"}' http://localhost:8080/api/login | ConvertFrom-Json
$token = $login.session_token
Write-Host "Token: $token"

# 2. Get question details to see expected schema
$question = curl.exe -s "http://localhost:8080/api/questions/second-highest-salary" | ConvertFrom-Json
Write-Host "`nQuestion schema:"
$question.schema.tables | Format-List

# 3. Test with Employee table (capitalized)
$correct_sql = "SELECT MAX(salary) AS SecondHighestSalary FROM Employee WHERE salary < (SELECT MAX(salary) FROM Employee)"
$body = @{
    session_token = $token
    question_id = "q1"
    user_sql = $correct_sql
} | ConvertTo-Json -Compress

Write-Host "`nSQL: $correct_sql"
$result1 = curl.exe -s -X POST -H 'Content-Type: application/json' -d $body http://localhost:8080/api/execute | ConvertFrom-Json

Write-Host "`nResult:"
Write-Host "  Success: $($result1.success)"
Write-Host "  Is Correct: $($result1.is_correct)"
Write-Host "  Error: $($result1.error)"
Write-Host "  Columns: $($result1.columns -join ', ')"
Write-Host "  Rows: $($result1.rows | ConvertTo-Json -Compress)"
Write-Host "  Expected: $($question.expected_result | ConvertTo-Json -Compress)"
