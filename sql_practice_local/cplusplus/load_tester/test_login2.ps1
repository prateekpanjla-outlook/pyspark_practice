$data = @{user_id='test_user_999'}
$json = $data | ConvertTo-Json -Compress
Write-Host "Sending JSON: $json"
$response = curl.exe -s -X POST -H 'Content-Type: application/json' -d $json http://localhost:8080/api/login
Write-Host "Response: $response"
