$login = curl.exe -s -X POST -H 'Content-Type: application/json' -d '{"user_id":"flow_test"}' http://localhost:8080/api/login | ConvertFrom-Json
$token = $login.session_token

Write-Host "Token: $token"

# Wrong SQL (should be wrong)
$wrongSql = "SELECT MAX(salary) FROM Employee"
$body1 = "{`"session_token`":`"$token`",`"question_id`":`"q1`",`"user_sql`":`"$wrongSql`"}"
$result1 = curl.exe -s -X POST -H 'Content-Type: application/json' -d $body1 http://localhost:8080/api/execute | ConvertFrom-Json
Write-Host "`nAttempt 1 (Wrong SQL):"
Write-Host "  Is Correct: $($result1.is_correct)"
Write-Host "  Result: $($result1.rows | ConvertTo-Json -Compress)"

# Correct SQL (should be correct)
$correctSql = "SELECT MAX(salary) AS SecondHighestSalary FROM Employee WHERE salary < (SELECT MAX(salary) FROM Employee)"
$body2 = "{`"session_token`":`"$token`",`"question_id`":`"q1`",`"user_sql`":`"$correctSql`"}"
$result2 = curl.exe -s -X POST -H 'Content-Type: application/json' -d $body2 http://localhost:8080/api/execute | ConvertFrom-Json
Write-Host "`nAttempt 2 (Correct SQL):"
Write-Host "  Is Correct: $($result2.is_correct)"
Write-Host "  Result: $($result2.rows | ConvertTo-Json -Compress)"
