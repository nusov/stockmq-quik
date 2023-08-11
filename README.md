# High-Performance RPC for Quik

This project contains Lua server for QUIK and client examples in Python 3.10+ and Go 1.18+.

# Lua Integration

Before you begin please download latest version of [LuaOverMQ](https://github.com/nusov/LuaOverMQ/releases) DLL and place it to lib folder.

There are two versions of the integration. Use quik-aio.py if you want to build your own bindings from scratch.

quik.lua includes several helpful functions such as unique id generator based on the snowflake algorithm, transaction and order management functions.

# Python SDK

```
import asyncio
import time

from stockmq.api import Quik, TimeInForce, Side, QuikTable

api = Quik("tcp://10.211.55.3:8004")

client = "CLIENT"
board = "TQBR"
ticker = "SBER"

async def main():
    # Create transaction to BUY and wait for completion
    tx = await api.create_order(client, board, ticker, TimeInForce.DAY, Side.BUY, 1600.0, 1)
    print(tx)
    print(tx.updated_ts - tx.created_ts)

    # Create transaction to cancel the order
    tx = await api.cancel_order(client, board, ticker, tx.order_id, timeout=4.0)
    print(tx)
    print(tx.updated_ts - tx.created_ts)

if __name__ == '__main__':
    asyncio.run(main())
```

# Python REPL

Using the repl.py you can connect to the running instance of LuaVM.

```
(.venv) alex@express python % python repl.py tcp://10.211.55.3:8004
Type exit() to terminate the session
>> return 1
1
```
