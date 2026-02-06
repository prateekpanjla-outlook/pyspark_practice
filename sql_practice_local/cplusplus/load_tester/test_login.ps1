$response = Invoke-RestMethod -Uri 'http://localhost:8080/api/login' -Method Post -Body '{"user_id":"test_user_123"}' -ContentType 'application/json'
Write-Host "Response type:" $response.GetType()
Write-Host "Session token:" $response.session_token
