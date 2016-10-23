#include "stdafx.h"
#include "LuaScript.h"

LuaScript::LuaScript(bool arelibsloaded)
	:m_AreLibsLoaded(arelibsloaded)
{
	m_pState = luaL_newstate();	//opens Lua
	if (m_AreLibsLoaded)
	{
		luaL_openlibs(m_pState);	//opens the standard libraries
	}
}

LuaScript::~LuaScript()
{
	lua_close(m_pState);//close lua
}

void LuaScript::LoadFile(const string path)
{
	//load lua file
	int error = luaL_loadfile(m_pState, path.c_str()) || lua_pcall(m_pState, 0, 0, 0);
	PrintError(error);
}

void LuaScript::PrintError(int error)
{
	if (error) 
	{
		string message = lua_tostring(m_pState, -1);
#ifdef _WIN32

		MessageBox(0, message.c_str(), "LuaVariable", MB_OK);
		exit(-1);
#else
		fprintf(stderr, "%s\n", message.c_str());
#endif
		lua_pop(m_pState, 1); // pop error message from the stack
	}
}

LuaVariable LuaScript::operator[](string name)
{
	//Get lua variable by name
	lua_getglobal(m_pState, name.c_str());//Get global lua variable on stack
	LuaVariable result(m_pState,name);	//make lua variable
	result.GetFromStack();
	return result;
}

void LuaScript::operator()(string script)
{
	luaL_dostring(m_pState, script.c_str());//run string as lua code
}