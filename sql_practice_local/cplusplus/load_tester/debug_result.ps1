# Debug why grading is failing

# 1. Login and get token
$login = curl.exe -s -X POST -H 'Content-Type: application/json' -d '{"user_id":"debug_test"}' http://localhost:8080/api/login | ConvertFrom-Json
$token = $login.session_token

Write-Host "=== Testing Second Highest Salary ==="
Write-Host "Token: $token`n"

# 2. Get question details to see expected result
$question = curl.exe -s "http://localhost:8080/api/questions/second-highest-salary" | ConvertFrom-Json

Write-Host "Expected answer:"
Write-Host "  Columns: $($question.expected_result[0].PSObject.Properties.Name -join ', ')"
Write-Host "  Values: $($question.expected_result[0].PSObject.Properties.Value -join ', ')"
Write-Host ""

# 3. Execute correct SQL
$sql = "SELECT MAX(salary) AS SecondHighestSalary FROM Employee WHERE salary < (SELECT MAX(salary) FROM Employee)"
Write-Host "SQL: $sql`n"

$body = @{
    session_token = $token
    question_id = "q1"
    user_sql = $sql
} | ConvertTo-Json -Compress

$result = curl.exe -s -X POST -H 'Content-Type: application/json' -d $body http://localhost:8080/api/execute | ConvertFrom-Json

Write-Host "Actual result:"
Write-Host "  Is Correct: $($result.is_correct)"
Write-Host "  Columns: $($result.columns -join ', ')"
Write-Host "  Rows:"
$result.rows | ForEach-Object {
    $row = $_
    $values = $result.columns | ForEach-Object { $row[$_] }
    Write-Host "    [$($values -join ', ')]"
}

Write-Host "`n=== Expected vs Actual ==="
$expected = $question.expected_result[0]
$actual = $result.rows[0]

Write-Host "Expected column name: $($expected.PSObject.Properties.Name)"
Write-Host "Actual column names: $($result.columns -join ', ')"

if ($result.columns.Count -eq 1) {
    $colName = $result.columns[0]
    $expectedVal = $expected.$colName
    $actualVal = $actual.$colName
    Write-Host "`nColumn '$colName':"
    Write-Host "  Expected: '$expectedVal'"
    Write-Host "  Actual: '$actualVal'"
    Write-Host "  Match: $($expectedVal -eq $actualVal)"
}
