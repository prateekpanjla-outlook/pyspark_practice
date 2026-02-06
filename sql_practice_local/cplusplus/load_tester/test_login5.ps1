# Test using --data-raw

Write-Host "=== Test 1: Using --data-raw ==="
$json1 = '{"user_id":"test1"}'
$response1 = curl.exe -s -X POST -H "Content-Type: application/json" --data-raw $json1 http://localhost:8080/api/login
Write-Host "Response1: $response1"

Write-Host "`n=== Test 2: Direct inline JSON ==="
$response2 = curl.exe -s -X POST -H "Content-Type: application/json" -d '{\"user_id\":\"test2\"}' http://localhost:8080/api/login
Write-Host "Response2: $response2"

Write-Host "`n=== Test 3: Via temp file ==="
$json3 = '{"user_id":"test3"}'
$tempFile = [System.IO.Path]::GetTempFileName()
[System.IO.File]::WriteAllText($tempFile, $json3)
$response3 = & 'curl.exe' -s -X POST -H 'Content-Type: application/json' -d "@$tempFile" http://localhost:8080/api/login
Write-Host "Response3: $response3"
Remove-Item $tempFile
