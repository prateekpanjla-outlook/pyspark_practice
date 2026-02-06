$rawResponse = curl.exe -s -X POST -H 'Content-Type: application/json' -d '{"user_id":"raw_test"}' http://localhost:8080/api/login

Write-Host "Raw response:"
Write-Host $rawResponse
Write-Host ""

# Try to parse
try {
    $parsed = $rawResponse | ConvertFrom-Json
    Write-Host "Parsed successfully"
    Write-Host "Success: $($parsed.success)"
    Write-Host "Token: $($parsed.session_token)"
} catch {
    Write-Host "Parse failed: $_"
}

# Try manual extraction
if ($rawResponse -match '"session_token":"([^"]+)"') {
    Write-Host "Manual token extraction: $($Matches[1])"
}
