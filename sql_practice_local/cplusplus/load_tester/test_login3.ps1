$data = @{user_id='test_user_999'}
$json = $data | ConvertTo-Json -Compress
Write-Host "Sending JSON: $json"
# Try with quotes around the JSON
$response = curl.exe -s -X POST -H "Content-Type: application/json" -d "$json" http://localhost:8080/api/login
Write-Host "Response: $response"

# Also try single quotes
$response2 = curl.exe -s -X POST -H 'Content-Type: application/json' -d '{"user_id":"test_user_999"}' http://localhost:8080/api/login
Write-Host "Response2: $response2"
