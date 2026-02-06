# SQL Practice Server - Load Testing Script (PowerShell)
# Usage: .\load_test.ps1 -ServerUrl "http://localhost:8080" -Users 1000

param(
    [string]$ServerUrl = "http://localhost:8082",
    [int]$Users = 1000
)

# Test cases with wrong and correct SQL solutions
# Table names: Employee, Person, Logs (capitalized to match schema)
$TestCases = @(
    @{
        question_id = "q1"
        question_title = "Second Highest Salary"
        wrong_sql = "SELECT MAX(salary) FROM Employee"
        correct_sql = "SELECT MAX(salary) AS SecondHighestSalary FROM Employee WHERE salary < (SELECT MAX(salary) FROM Employee)"
    },
    @{
        question_id = "q2"
        question_title = "Duplicate Emails"
        wrong_sql = "SELECT email FROM Person"
        correct_sql = "SELECT email FROM Person GROUP BY email HAVING COUNT(*) > 1"
    },
    @{
        question_id = "q8"
        question_title = "Consecutive Numbers"
        wrong_sql = "SELECT DISTINCT num FROM Logs"
        correct_sql = "SELECT DISTINCT l1.num AS consecutive_numbers FROM Logs l1 JOIN Logs l2 ON l1.id = l2.id - 1 AND l1.num = l2.num JOIN Logs l3 ON l1.id = l3.id - 2 AND l1.num = l3.num"
    }
)

# Synchronized hashtable for thread-safe statistics
$Stats = [System.Collections.Hashtable]::Synchronized(@{
    total_attempts = 0
    successful_attempts = 0
    failed_attempts = 0
    correct_answers = 0
    wrong_answers = 0
    total_response_time = 0
    min_response_time = [long]::MaxValue
    max_response_time = 0
})

# Session tokens (thread-safe list)
$SessionTokens = [System.Collections.ArrayList]::Synchronized([System.Collections.ArrayList]::new())

function Invoke-HttpPost {
    param(
        [string]$Url,
        [hashtable]$Data
    )

    try {
        $jsonData = $Data | ConvertTo-Json -Compress
        $tempFile = [System.IO.Path]::GetTempFileName()

        try {
            [System.IO.File]::WriteAllText($tempFile, $jsonData)
            $responseJson = & 'curl.exe' -s -X POST -H 'Content-Type: application/json' -d "@$tempFile" $Url 2>&1

            if ($LASTEXITCODE -ne 0) {
                return @{ success = $false; error = "curl exit code: $LASTEXITCODE" }
            }

            $response = $responseJson | ConvertFrom-Json
            return @{ success = $true; data = $response }
        } finally {
            Remove-Item $tempFile -ErrorAction SilentlyContinue
        }
    } catch {
        return @{ success = $false; error = $_.Exception.Message }
    }
}

function Create-Sessions {
    param([int]$Count)

    Write-Host "Creating $Count user sessions..."

    $loginUrl = "$ServerUrl/api/login"

    for ($i = 0; $i -lt $Count; $i++) {
        $userId = "load_test_user_$i"

        $result = Invoke-HttpPost -Url $loginUrl -Data @{user_id = $userId}

        if ($result.success -and $result.data.session_token) {
            [void]$SessionTokens.Add($result.data.session_token)

            if (($i + 1) % 100 -eq 0) {
                Write-Host "  Created $($i + 1) sessions..."
            }
        } else {
            Write-Host "Failed to create session for user $i"
            Write-Host "  Success: $($result.success)"
            Write-Host "  Error: $($result.error)"
            if ($result.data) {
                Write-Host "  Data: $($result.data | ConvertTo-Json -Compress)"
            }
            return $false
        }
    }

    Write-Host "All $Count sessions created successfully" -ForegroundColor Green
    return $true
}

function Execute-Test {
    param(
        [string]$SessionToken,
        [string]$QuestionId,
        [string]$Sql
    )

    $executeUrl = "$ServerUrl/api/execute"

    $data = @{
        session_token = $SessionToken
        question_id = $QuestionId
        user_sql = $Sql
    }

    $stopwatch = [System.Diagnostics.Stopwatch]::StartNew()
    $result = Invoke-HttpPost -Url $executeUrl -Data $data
    $stopwatch.Stop()

    $responseTime = $stopwatch.ElapsedMilliseconds

    if ($result.success) {
        $isCorrect = $result.data.is_correct
        return @{
            success = $true
            is_correct = $isCorrect
            response_time_ms = $responseTime
        }
    } else {
        return @{
            success = $false
            is_correct = $false
            response_time_ms = $responseTime
            error = $result.error
        }
    }
}

function Record-Result {
    param([hashtable]$Result)

    $Stats.total_attempts++

    if ($Result.success) {
        $Stats.successful_attempts++
        $Stats.total_response_time += $Result.response_time_ms

        if ($Result.response_time_ms -lt $Stats.min_response_time) {
            $Stats.min_response_time = $Result.response_time_ms
        }

        if ($Result.response_time_ms -gt $Stats.max_response_time) {
            $Stats.max_response_time = $Result.response_time_ms
        }
    } else {
        $Stats.failed_attempts++
    }

    if ($Result.is_correct) {
        $Stats.correct_answers++
    } else {
        $Stats.wrong_answers++
    }
}

function Print-Results {
    Write-Host ""
    Write-Host "========================================================" -ForegroundColor Cyan
    Write-Host "           LOAD TEST RESULTS                              " -ForegroundColor Cyan
    Write-Host "--------------------------------------------------------" -ForegroundColor Cyan
    Write-Host ("Total Attempts:     {0,40}" -f $Stats.total_attempts)
    Write-Host ("Successful:         {0,40}" -f $Stats.successful_attempts)
    Write-Host ("Failed:             {0,40}" -f $Stats.failed_attempts)
    Write-Host ("Correct Answers:    {0,40}" -f $Stats.correct_answers)
    Write-Host ("Wrong Answers:      {0,40}" -f $Stats.wrong_answers)
    Write-Host "--------------------------------------------------------" -ForegroundColor Cyan

    if ($Stats.successful_attempts -gt 0) {
        $avg = $Stats.total_response_time / $Stats.successful_attempts
        Write-Host ("Avg Response Time: {0,39} ms" -f [int]$avg)
        Write-Host ("Min Response Time: {0,39} ms" -f $Stats.min_response_time)
        Write-Host ("Max Response Time: {0,39} ms" -f $Stats.max_response_time)
    }

    Write-Host "========================================================" -ForegroundColor Cyan
    Write-Host ""
}

# Main execution
Write-Host ""
Write-Host "========================================================" -ForegroundColor Cyan
Write-Host "     SQL Practice Server - Load Testing Tool            " -ForegroundColor Cyan
Write-Host "========================================================" -ForegroundColor Cyan
Write-Host ""
Write-Host "Configuration:"
Write-Host "  Server URL: $ServerUrl"
Write-Host "  Concurrent Users: $Users"
Write-Host "  Attempts per User: 2 (wrong answer, then correct)"
Write-Host "  Total Requests: $($Users * 2)"
Write-Host ""

# Create sessions
if (-not (Create-Sessions -Count $Users)) {
    Write-Host "Failed to create sessions. Exiting." -ForegroundColor Red
    exit 1
}

# Run load test
Write-Host ""
Write-Host "Starting load test with $Users concurrent users..." -ForegroundColor Cyan
Write-Host "Server: $ServerUrl"
Write-Host "Each user will run 2 attempts (wrong answer, then correct answer)" -ForegroundColor Gray
Write-Host ""

$stopwatch = [System.Diagnostics.Stopwatch]::StartNew()

# Use ForEach-Object -Parallel for PowerShell 7+
# For older PowerShell, use sequential execution with limited parallelization
$maxConcurrent = 500  # Increased to handle high concurrent user loads
$currentIndex = 0

# Create runspace pool
$runspacePool = [RunspaceFactory]::CreateRunspacePool(1, $maxConcurrent)
$runspacePool.Open()

$jobs = @()

for ($i = 0; $i -lt $Users; $i++) {
    $token = $SessionTokens[$i]
    $random = New-Object Random
    $questionIdx = $random.Next(0, $TestCases.Count)
    $testCase = $TestCases[$questionIdx]

    # Create a scriptblock with the test logic
    $scriptBlock = {
        param($ServerUrl, $SessionToken, $QuestionId, $WrongSql, $CorrectSql, $TestId)

        function Invoke-HttpPost {
            param(
                [string]$Url,
                [hashtable]$Data
            )

            try {
                $jsonData = $Data | ConvertTo-Json -Compress
                $tempFile = [System.IO.Path]::GetTempFileName()

                try {
                    [System.IO.File]::WriteAllText($tempFile, $jsonData)
                    $responseJson = & 'curl.exe' -s -X POST -H 'Content-Type: application/json' -d "@$tempFile" $Url 2>&1

                    if ($LASTEXITCODE -ne 0) {
                        return @{ success = $false; error = "curl exit code: $LASTEXITCODE" }
                    }

                    $response = $responseJson | ConvertFrom-Json
                    return @{ success = $true; data = $response }
                } finally {
                    Remove-Item $tempFile -ErrorAction SilentlyContinue
                }
            } catch {
                return @{ success = $false; error = $_.Exception.Message }
            }
        }

        function Execute-Test {
            param(
                [string]$SessionToken,
                [string]$QuestionId,
                [string]$Sql
            )

            $executeUrl = "$ServerUrl/api/execute"

            $data = @{
                session_token = $SessionToken
                question_id = $QuestionId
                user_sql = $Sql
            }

            $stopwatch = [System.Diagnostics.Stopwatch]::StartNew()
            $result = Invoke-HttpPost -Url $executeUrl -Data $data
            $stopwatch.Stop()

            $responseTime = $stopwatch.ElapsedMilliseconds

            if ($result.success) {
                $isCorrect = $result.data.is_correct
                return @{
                    success = $true
                    is_correct = $isCorrect
                    response_time_ms = $responseTime
                }
            } else {
                return @{
                    success = $false
                    is_correct = $false
                    response_time_ms = $responseTime
                    error = $result.error
                }
            }
        }

        $result1 = Execute-Test -SessionToken $SessionToken -QuestionId $QuestionId -Sql $WrongSql
        Start-Sleep -Milliseconds 10
        $result2 = Execute-Test -SessionToken $SessionToken -QuestionId $QuestionId -Sql $CorrectSql

        return @{
            result1 = $result1
            result2 = $result2
            test_id = $TestId
        }
    }

    $powershell = [powershell]::Create()
    $powershell.RunspacePool = $runspacePool
    [void]$powershell.AddScript($scriptBlock).AddArgument($ServerUrl).AddArgument($token).AddArgument($testCase.question_id).AddArgument($testCase.wrong_sql).AddArgument($testCase.correct_sql).AddArgument($i)

    $jobs += @{
        PowerShell = $powershell
        AsyncResult = $powershell.BeginInvoke()
        UserId = $i
        TestCaseTitle = $testCase.question_title
    }
}

# Wait for all jobs to complete and collect results
foreach ($job in $jobs) {
    $result = $job.PowerShell.EndInvoke($job.AsyncResult)
    $result1 = $result.result1
    $result2 = $result.result2

    Record-Result -Result $result1
    Record-Result -Result $result2

    if ($job.UserId -lt 5 -or $job.UserId % 100 -eq 0) {
        $status1 = if ($result1.is_correct) { "[OK]" } else { "[X]" }
        $status2 = if ($result2.is_correct) { "[OK]" } else { "[X]" }
        Write-Host "User $($job.UserId) [$($job.TestCaseTitle)]: Attempt 1: $status1 ($($result1.response_time_ms) ms), Attempt 2: $status2 ($($result2.response_time_ms) ms)"
    }

    $job.PowerShell.Dispose()
}

$runspacePool.Close()
$runspacePool.Dispose()

$stopwatch.Stop()

Write-Host ""
Write-Host ("Total test duration: {0} ms" -f $stopwatch.ElapsedMilliseconds) -ForegroundColor Green
Write-Host ("Throughput: {0:F2} requests/second" -f ($Users * 2) / ($stopwatch.ElapsedMilliseconds / 1000.0)) -ForegroundColor Green
Write-Host ""

Print-Results
