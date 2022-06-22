# LuaOverMQ

```
require("LuaOverMQ")

local STOCKMQ_ZMQ_REP = 4
local STOCKMQ_ZMQ_PUB = 1

local isRunning = true

function main()
    rpc = LuaOverMQ.bind("tcp://0.0.0.0:8004", STOCKMQ_ZMQ_REP)
    pub = LuaOverMQ.bind("tcp://0.0.0.0:8005", STOCKMQ_ZMQ_PUB)

    pub:send("hello", "world")

    while isRunning do
        if rpc:process() ~= 0 then
            print("StockMQ Error: "..tostring(rpc:errno()))
        end
    end
end
```
