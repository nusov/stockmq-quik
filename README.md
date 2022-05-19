# LuaOverMQ

```
require("LuaOverMQ")

local isRunning = true

function main()
	rpc = LuaOverMQ.bind("tcp://0.0.0.0:8004")
	while isRunning do
		rpc.process()
	end
end
```
