/*
 * This file is part of the LuaOverMQ distribution (https://github.com/StockMQ/LuaOverMQ).
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
 */
#include "pch.h"

constexpr auto CODE_PAGE = 1251;
constexpr auto METATABLE = "luaL_LuaOverMQ";
constexpr auto LUAOVERMQ = "LuaOverMQ";
constexpr auto RUNTIME_ERROR = "RUNTIME_ERROR";
constexpr auto OK = "OK";
constexpr auto NOT_FOUND = "NOT_FOUND";

// String Utils
std::string wide_to_ansi(const std::wstring& wstr) {
	int count = WideCharToMultiByte(CODE_PAGE, 0, wstr.c_str(), static_cast<int>(wstr.length()), NULL, 0, NULL, NULL);
	std::string str(count, 0);
	WideCharToMultiByte(CODE_PAGE, 0, wstr.c_str(), -1, &str[0], count, NULL, NULL);
	return str;
}

std::string wide_to_utf8(const std::wstring& wstr) {
	int count = WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), static_cast<int>(wstr.length()), NULL, 0, NULL, NULL);
	std::string str(count, 0);
	WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), -1, &str[0], static_cast<int>(count), NULL, NULL);
	return str;
}

std::wstring ansi_to_wide(const std::string& str) {
	int count = MultiByteToWideChar(CODE_PAGE, 0, str.c_str(), static_cast<int>(str.length()), NULL, 0);
	std::wstring wstr(count, 0);
	MultiByteToWideChar(CODE_PAGE, 0, str.c_str(), static_cast<int>(str.length()), &wstr[0], count);
	return wstr;
}

std::wstring utf8_to_wide(const std::string& str) {
	int count = MultiByteToWideChar(CP_UTF8, 0, str.c_str(), static_cast<int>(str.length()), NULL, 0);
	std::wstring wstr(count, 0);
	MultiByteToWideChar(CP_UTF8, 0, str.c_str(), static_cast<int>(str.length()), &wstr[0], count);
	return wstr;
}

// Stack Utils
int stack_table_count(lua_State* L, int t) {
	lua_pushnil(L);
	int count = 0;
	while (lua_next(L, t) != 0) {
		lua_pop(L, 1);
		count++;
	}
	return count;
}

void stack_push(lua_State* L, msgpack::object& obj) {
	switch (obj.type) {
	case msgpack::type::STR:
		lua_pushstring(L, wide_to_ansi(utf8_to_wide(obj.as<std::string>())).c_str());
		break;
	case msgpack::type::BOOLEAN:
		lua_pushboolean(L, obj.via.boolean);
		break;
	case msgpack::type::NIL:
		lua_pushnil(L);
		break;
	case msgpack::type::POSITIVE_INTEGER:
		lua_pushnumber(L, (lua_Number)obj.via.u64);
		break;
	case msgpack::type::NEGATIVE_INTEGER:
		lua_pushnumber(L, (lua_Number)obj.via.i64);
		break;
	case msgpack::type::FLOAT32:
		lua_pushnumber(L, (lua_Number)obj.via.f64);
		break;
	case msgpack::type::FLOAT64:
		lua_pushnumber(L, (lua_Number)obj.via.f64);
		break;
	case msgpack::type::ARRAY:
		lua_newtable(L);
		for (uint32_t i = 0; i < obj.via.array.size; i++) {
			lua_pushinteger(L, static_cast<lua_Integer>(i) + 1);
			stack_push(L, obj.via.array.ptr[i]);
			lua_settable(L, -3);
		}
		break;
	case msgpack::type::MAP:
		lua_newtable(L);
		for (uint32_t i = 0; i < obj.via.map.size; i++) {
			stack_push(L, obj.via.map.ptr[i].key);
			stack_push(L, obj.via.map.ptr[i].val);
			lua_settable(L, -3);
		}
	}
}

void stack_pack(msgpack::packer<msgpack::sbuffer>& pk, lua_State* L, int i) {
	switch (lua_type(L, i)) {
	case LUA_TNIL:
		pk.pack_nil();
		break;
	case LUA_TBOOLEAN:
		pk.pack(static_cast<bool>(lua_toboolean(L, i)));
		break;
	case LUA_TNUMBER:
		if (lua_isinteger(L, i)) {
			pk.pack(lua_tointeger(L, i));
		}
		else {
			pk.pack(lua_tonumber(L, i));
		}
		break;
	case LUA_TSTRING:
		pk.pack(wide_to_utf8(ansi_to_wide(lua_tostring(L, i))));
		break;
	case LUA_TTABLE:
		pk.pack_map(stack_table_count(L, i));
		lua_pushnil(L);
		while (lua_next(L, i)) {
			stack_pack(pk, L, -2);
			stack_pack(pk, L, lua_gettop(L));
			lua_pop(L, 1);
		}
		break;
	default:
		pk.pack_nil();
		break;
	}
}

// LuaOverMQ
struct LuaOverMQ {
	zmq::socket_t* zmq_skt;
	zmq::context_t* zmq_ctx;
	int zmq_err;
};


inline LuaOverMQ* luaovermq_check(lua_State* L, int n) {
	return *(LuaOverMQ**)luaL_checkudata(L, n, METATABLE);
}

static int luaovermq_bind(lua_State* L) {
	std::string bind_address = luaL_checkstring(L, 1);
	LuaOverMQ** udata = (LuaOverMQ**)lua_newuserdata(L, sizeof(LuaOverMQ*));
	*udata = new LuaOverMQ();

	(*udata)->zmq_ctx = new zmq::context_t(1);
	(*udata)->zmq_skt = new zmq::socket_t(*(*udata)->zmq_ctx, ZMQ_REP);
	(*udata)->zmq_skt->bind(bind_address);

	luaL_getmetatable(L, METATABLE);
	lua_setmetatable(L, -2);
	return 1;
}

static int luaovermq_process(lua_State* L) {
	LuaOverMQ* s = luaovermq_check(L, 1);
	zmq::socket_t* zmq_skt = s->zmq_skt;

	if (zmq_skt) {
		try {
			zmq::message_t msg;

			if (zmq_skt->recv(msg, zmq::recv_flags::none)) {
				auto handle = msgpack::unpack(static_cast<const char*>(msg.data()), msg.size());
				std::string status = OK;

				msgpack::sbuffer buffer;
				msgpack::packer<msgpack::sbuffer> pk(buffer);

				std::string funcname;
				handle.get().via.array.ptr[0].convert(funcname);

				int level = lua_gettop(L);
				lua_getglobal(L, funcname.c_str());
				if (!lua_isnil(L, -1)) {
					for (uint32_t i = 1; i < handle.get().via.array.size; i++) {
						stack_push(L, handle.get().via.array.ptr[i]);
					}

					int top_prev = lua_gettop(L);
					if (lua_pcall(L, handle.get().via.array.size - 1, LUA_MULTRET, 0) != 0) {
						status = RUNTIME_ERROR;
						pk.pack(lua_tostring(L, -1));
						lua_pop(L, -1);
					}
					else {
						int results = lua_gettop(L) - level;

						pk.pack_array(results);

						for (int i = results; i > 0; i--) {
							stack_pack(pk, L, lua_gettop(L) - i + 1);
						}

						// cleanup stack
						for (int i = 0; i < results; i++) {
							lua_pop(L, 1);
						}
					}
				}
				else {
					status = NOT_FOUND;
					lua_pop(L, -1);
					pk.pack(funcname);
				}
				zmq_skt->send(zmq::message_t(status.data(), status.size()), zmq::send_flags::sndmore);
				zmq_skt->send(zmq::message_t(buffer.data(), buffer.size()), zmq::send_flags::none);
			}
		}
		catch (zmq::error_t ex) {
			s->zmq_err = ex.num();
		}
	}
	return 1;
}


static int luaovermq_errno(lua_State* L) {
	LuaOverMQ* s = luaovermq_check(L, 1);
	lua_pushinteger(L, static_cast<lua_Integer>(s->zmq_err));
	return 1;
}

static int luaovermq_destructor(lua_State* L) {
	LuaOverMQ* s = luaovermq_check(L, 1);
	if (s->zmq_skt) {
		s->zmq_skt->close();
		delete s->zmq_skt;
	}
	if (s->zmq_ctx) {
		s->zmq_ctx->close();
		delete s->zmq_ctx;
	}
	delete s;
	return 0;
}

static luaL_Reg funcs[] = {
	{ "bind", luaovermq_bind },
	{ "process", luaovermq_process },
	{ "errno", luaovermq_errno },
	{ "__gc", luaovermq_destructor },
	{ NULL, NULL }
};

extern "C" LUALIB_API int luaopen_LuaOverMQ(lua_State * L) {
	luaL_checkversion(L);
	luaL_newmetatable(L, METATABLE);
	luaL_setfuncs(L, funcs, 0);
	lua_pushvalue(L, -1);
	lua_setfield(L, -1, "__index");
	lua_setglobal(L, "LuaOverMQ");
	return 1;
}