# LuaOverMQ

LuaOverMQ is high-performance RPC library that uses MsgPack and ZeroMQ.

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

# Before you begin building the application, you must have the following prerequisites installed on your system

* [Visual Studio 2022 (C++ Desktop Development)](https://visualstudio.microsoft.com/downloads/)
* [vcpkg](https://vcpkg.io/en/getting-started.html)

# Install dependencies

```
vcpkg.exe install msgpack cppzmq --triplet=x64-windows-static
vcpkg.exe integrate install
```

# Build Lua 5.4.1 (Preferred)
```
git clone -b v5.4.1 https://github.com/lua/lua
cd lua
del *.o *.lib *.exe
cl /MD /O2 /c /DLUA_BUILD_AS_DLL *.c
ren lua.obj lua.o
ren onelua.obj onelua.o
link /DLL /IMPLIB:luadyn.lib /OUT:lua54.dll *.obj
link /OUT:lua.exe lua.o luadyn.lib
```

# Build Lua 5.3.5 
```
git clone -b v5.3.5 https://github.com/lua/lua
cd lua
del *.o *.lib *.exe
cl /MD /O2 /c /DLUA_BUILD_AS_DLL *.c
ren lua.obj lua.o
ren luac.obj luac.o
link /DLL /IMPLIB:luadyn.lib /OUT:lua53.dll *.obj
link /OUT:lua.exe lua.o luadyn.lib
```

# Build the DLL
```
msbuild -p:Configuration=Release
```

