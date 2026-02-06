$session = curl.exe -s -X POST -H 'Content-Type: application/json' -d '{"user_id":"fresh_test"}' http://localhost:8080/api/login | ConvertFrom-Json
$token = $session.session_token

Write-Host "Token: $token"

$sql = "SELECT MAX(salary) AS SecondHighestSalary FROM Employee WHERE salary < (SELECT MAX(salary) FROM Employee)"
$body = "{`"session_token`":`"$token`",`"question_id`":`"q1`",`"user_sql`":`"$sql`"}"

$result = curl.exe -s -X POST -H 'Content-Type: application/json' -d $body http://localhost:8080/api/execute | ConvertFrom-Json

Write-Host "`nResult:"
Write-Host "  Success: $($result.success)"
Write-Host "  Is Correct: $($result.is_correct)"
Write-Host "  Error: $($result.error)"
if ($result.rows) {
    Write-Host "  Rows: $($result.rows | ConvertTo-Json)"
}
