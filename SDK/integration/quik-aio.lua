--[[
 * This file is part of the StockMQ distribution (https://github.com/StockMQ).
 * Copyright (c) 2022 Alexander Nusov
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 3.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
]]

-- Configure LUA_CPATH
package.cpath = package.cpath .. ';' 
    .. getScriptPath() .. "\\lib\\?.dll"

-- Load C++ extension
require("LuaOverMQ")

-- Global constants
STOCKMQ_ZMQ_REP = 4
STOCKMQ_ZMQ_PUB = 1

STOCKMQ_RPC_TIMEOUT = 10
STOCKMQ_RPC_URI = "tcp://0.0.0.0:8004"
STOCKMQ_PUB_URI = "tcp://0.0.0.0:8005"

-- Global variables
STOCKMQ_RUN = false
STOCKMQ_PUB = nil

-- Used to test serialization/deserialization
function stockmq_test(...)
    return table.unpack({...})
end

-- REPL helper function
function stockmq_repl(s)
    return assert(load(s))()
end

-- Publish messages if STOCKMQ_PUB is defined
function stockmq_publish(topic, message)
    if STOCKMQ_PUB ~= nil then 
        STOCKMQ_PUB:send(topic, message)
    end
end

-- Set global variable which is used by main() function
function OnInit(script_path)
    STOCKMQ_PUB = LuaOverMQ.bind(STOCKMQ_PUB_URI, STOCKMQ_ZMQ_PUB)
    STOCKMQ_RUN = true
end

-- Callback called when the script is stopped
function OnStop(signal)
    STOCKMQ_RUN = false
    return STOCKMQ_RPC_TIMEOUT
end

-- Main function
function main()
    local rpc = LuaOverMQ.bind(STOCKMQ_RPC_URI, STOCKMQ_ZMQ_REP)
    while STOCKMQ_RUN do
        if rpc:process() ~= 0 then
            message("LuaOverMQ Error: code " .. tostring(rpc:errno()), 1)
        end
    end
end

-- Callbacks
function OnFirm(msg)
    stockmq_publish("OnFirm", {ts=LuaOverMQ.time(), msg=msg})
end

function OnAllTrade(msg)
    stockmq_publish("OnAllTrade", {ts=LuaOverMQ.time(), msg=msg})
end

function OnTrade(msg)
    stockmq_publish("OnTrade", {ts=LuaOverMQ.time(), msg=msg})
end

function OnOrder(msg)
    stockmq_publish("OnOrder", {ts=LuaOverMQ.time(), msg=msg})
end

function OnAccountBalance(msg)
    stockmq_publish("OnAccountBalance", {ts=LuaOverMQ.time(), msg=msg})
end

function OnFuturesLimitChange(msg)
    stockmq_publish("OnFuturesLimitChange", {ts=LuaOverMQ.time(), msg=msg})
end

function OnFuturesLimitDelete(msg)
    stockmq_publish("OnFuturesLimitDelete", {ts=LuaOverMQ.time(), msg=msg})
end

function OnFuturesClientHolding(msg)
    stockmq_publish("OnFuturesClientHolding", {ts=LuaOverMQ.time(), msg=msg})
end

function OnMoneyLimit(msg)
    stockmq_publish("OnMoneyLimit", {ts=LuaOverMQ.time(), msg=msg})
end

function OnMoneyLimitDelete(msg)
    stockmq_publish("OnMoneyLimitDelete", {ts=LuaOverMQ.time(), msg=msg})
end

function OnDepoLimit(msg)
    stockmq_publish("OnDepoLimit", {ts=LuaOverMQ.time(), msg=msg})
end

function OnDepoLimitDelete(msg)
    stockmq_publish("OnDepoLimitDelete", {ts=LuaOverMQ.time(), msg=msg})
end

function OnAccountPosition(msg)
    stockmq_publish("OnAccountPosition", {ts=LuaOverMQ.time(), msg=msg})
end

function OnNegDeal(msg)
    stockmq_publish("OnNegDeal", {ts=LuaOverMQ.time(), msg=msg})
end

function OnNegTrade(msg)
    stockmq_publish("OnNegTrade", {ts=LuaOverMQ.time(), msg=msg})
end

function OnStopOrder(msg)
    stockmq_publish("OnStopOrder", {ts=LuaOverMQ.time(), msg=msg})
end

function OnTransReply(msg)
    stockmq_publish("OnTransReply", {ts=LuaOverMQ.time(), msg=msg})
end

function OnParam(msg1, msg2)
    stockmq_publish("OnParam", {ts=LuaOverMQ.time(), msg={class_code=msg1, sec_code=msg2}})
end

function OnQuote(msg1, msg2)
    stockmq_publish("OnQuote", {ts=LuaOverMQ.time(), msg={class_code=msg1, sec_code=msg2}})
end

function OnDisconnected()
    stockmq_publish("OnDisconnected", {ts=LuaOverMQ.time(), msg=nil})
end

function OnConnected(msg)
    stockmq_publish("OnConnected", {ts=LuaOverMQ.time(), msg=msg})
end

function OnCleanUp()
    stockmq_publish("OnCleanUp", {ts=LuaOverMQ.time(), msg=nil})
end
