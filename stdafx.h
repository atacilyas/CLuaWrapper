#pragma once

#include <iostream>
#include <string>
using namespace std;

extern "C" {
# include "lua.h"
# include "lauxlib.h"
# include "lualib.h"
}
#define LUA_USE_APICHECK

#ifdef _WIN32

#include <Windows.h>
#endif // _WIN32
#include <assert.h>

#include "StackDump.h"
