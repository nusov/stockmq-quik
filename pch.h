// pch.h: This is a precompiled header file.
// Files listed below are compiled only once, improving build performance for future builds.
// This also affects IntelliSense performance, including code completion and many code browsing features.
// However, files listed here are ALL re-compiled if any one of them is updated between builds.
// Do not add files here that you will be updating frequently as this negates the performance advantage.

#ifndef PCH_H
#define PCH_H

// Windows
#include "framework.h"

// STL
#include <string>

// MsgPack
#include <msgpack.hpp>

// ZeroMQ
#include <zmq.hpp>

// Lua
#define LUA_LIB
#define LUA_BUILD_AS_DLL

extern "C" {
#include <lauxlib.h>
#include <lualib.h>
}


#endif //PCH_H
