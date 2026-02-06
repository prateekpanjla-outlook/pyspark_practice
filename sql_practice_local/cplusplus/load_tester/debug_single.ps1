# Debug individual query
$login = curl.exe -s -X POST -H 'Content-Type: application/json' -d '{"user_id":"debug_test"}' http://localhost:8080/api/login
Write-Host "Login response: $login"

$session = $login | ConvertFrom-Json
$token = $session.session_token
Write-Host "Token: $token"

# Test correct SQL
$sql = "SELECT MAX(salary) AS SecondHighestSalary FROM Employee WHERE salary < (SELECT MAX(salary) FROM Employee)"
$body = "{`"session_token`":`"$token`",`"question_id`":`"q1`",`"user_sql`":`"$sql`"}" | ConvertTo-Json -Compress

$result = curl.exe -s -X POST -H 'Content-Type: application/json' -d $body http://localhost:8080/api/execute | ConvertFrom-Json

Write-Host "`n=== Execute Result ==="
Write-Host "Success: $($result.success)"
Write-Host "Is Correct: $($result.is_correct)"
Write-Host "Error: $($result.error)"
Write-Host "Columns: $($result.columns -join ', ')"
Write-Host "Rows:"
$result.rows | ForEach-Object { Write-Host "  $($_ | ConvertTo-Json -Compress)" }
