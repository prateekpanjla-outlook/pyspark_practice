-- wrk load testing script for SQL Practice Server
-- Tests full flow: Login -> Execute SQL

local math = math
local string = string
local counter = 0
local session_tokens = {}
local max_cached_tokens = 1000

-- Initialize per thread
init = function(args)
  counter = 0
  math.randomseed(os.time())
end

-- Generate login request
function login_request()
  local user_id = "wrk_user_" .. counter
  counter = counter + 1

  local body = '{"user_id":"' .. user_id .. '"}'

  return wrk.format("POST", "/api/login", {
    ["Content-Type"] = "application/json"
  }, body)
end

-- Generate execute SQL request
function execute_request(session_token)
  -- Correct answer for question 1 (Duplicate Emails)
  local sql = "SELECT Email FROM Person GROUP BY Email HAVING COUNT(*) > 1;"

  local body = '{"session_token":"' .. session_token .. '","question_id":"1","user_sql":"' .. sql .. '"}'

  return wrk.format("POST", "/api/execute", {
    ["Content-Type"] = "application/json"
  }, body)
end

-- Main request function
request = function()
  -- 30% do login, 70% do execute (simulates multiple queries per session)
  if math.random(100) <= 30 or #session_tokens == 0 then
    return login_request()
  else
    -- Pick a random cached session token
    local idx = math.random(#session_tokens)
    return execute_request(session_tokens[idx])
  end
end

-- Process response to extract session token
response = function(status, headers, body)
  if status == 200 and body then
    -- Try to extract session_token from login response
    local token = body:match('"session_token":"([^"]+)"')
    if token and #session_tokens < max_cached_tokens then
      session_tokens[#session_tokens + 1] = token
    end
  end
end

-- Done function for stats
done = function(summary, latency, requests)
  io.write("\n====================\n")
  io.write("Thread Statistics:\n")
  io.write("  Requests: " .. tostring(requests) .. "\n")
  io.write("  Session tokens cached: " .. #session_tokens .. "\n")
  io.write("====================\n")
end
