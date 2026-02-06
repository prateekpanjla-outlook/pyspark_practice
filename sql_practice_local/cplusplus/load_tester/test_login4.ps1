# Test different ways to send JSON via curl from PowerShell

Write-Host "=== Test 1: Using single quotes in PowerShell string ==="
$json1 = '{"user_id":"test1"}'
$response1 = curl.exe -s -X POST -H "Content-Type: application/json" --data-raw $json1 http://localhost:8080/api/login
Write-Host "Response1: $response1"

Write-Host "`n=== Test 2: Using file for JSON ==="
$json2 = '{"user_id":"test2"}'
$json2 | Out-File -FilePath temp.json -Encoding utf8
$response2 = curl.exe -s -X POST -H "Content-Type: application/json" -d @temp.json http://localhost:8080/api/login
Write-Host "Response2: $response2"

Write-Host "`n=== Test 3: Using stdin ==="
$json3 = '{"user_id":"test3"}'
$response3 = $json3 | curl.exe -s -X POST -H "Content-Type: application/json" -d @- http://localhost:8080/api/login
Write-Host "Response3: $response3"

Remove-Item temp.json -ErrorAction SilentlyContinue
