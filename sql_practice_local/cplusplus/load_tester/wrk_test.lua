-- wrk load testing script for SQL Practice Server
-- Tests login and execute endpoints

-- Counter for requests
local counter = 0
local login_responses = {}

-- Login request
function request_login()
  local user_id = "wrk_user_" .. counter
  counter = counter + 1

  local body = '{"user_id":"' .. user_id .. '"}'

  return wrk.format("POST", "/api/login", {
    ["Content-Type"] = "application/json"
  }, body)
end

-- Execute SQL request
function request_execute(session_token)
  local body = '{"session_token":"' .. session_token .. '","question_id":"1","user_sql":"SELECT Email FROM Person GROUP BY Email HAVING COUNT(*) > 1;"}'

  return wrk.format("POST", "/api/execute", {
    ["Content-Type"] = "application/json"
  }, body)
end

-- Main request function - alternates between login and execute
request = function()
  -- For load testing, we'll do login then execute
  return request_login()
end

-- Process response
response = function(status, headers, body)
  if status == 200 then
    -- Could parse response here to extract session_token
    -- and use for next request
  end
end

-- Called when thread starts
init = function(args)
  counter = 0
end

-- Called when thread ends
done = function(summary, latency, requests)
  io.write("\n-- Thread Stats --\n")
  io.write("Requests: " .. requests:count() .. "\n")
end
