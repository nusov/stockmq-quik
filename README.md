# LuaOverMQ

```
require("LuaOverMQ")

local STOCKMQ_ZMQ_REP = 4
local STOCKMQ_ZMQ_PUB = 1

local isRunning = true

function main()
    rpc = LuaOverMQ.bind("tcp://0.0.0.0:8004", STOCKMQ_ZMQ_REP)
    pub = LuaOverMQ.bind("tcp://0.0.0.0:8005", STOCKMQ_ZMQ_PUB)

    -- Publish a message
    if pub:send("topic", "Hello World!") ~= 0 then
        print("LuaOverMQ Error: "..tostring(rpc:errno()))
    end

    -- RPC Server loop
    while isRunning do
        if rpc:process() ~= 0 then
            print("LuaOverMQ Error: "..tostring(rpc:errno()))
        end
    end
end
```
