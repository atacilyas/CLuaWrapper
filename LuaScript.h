#pragma once
#include "LuaVariable.h"
#include "StackDump.h"
#include "LuaClass.h"

class LuaScript
{
public:
	LuaScript(bool loadlibs = true);
	~LuaScript();

	void LoadFile(const string path);	//open a lua file
	void operator()(string);			//run string as lua code
	LuaVariable operator[](string);		//get a lua variable

private:
	lua_State* m_pState;
	void PrintError(int error);
	bool m_AreLibsLoaded;
public:
	template<typename... Args,typename... funcs>
	void AddClass(LuaClass<funcs...>* luaclass)
	{
		//make a new class function
		lua_register(m_pState, luaclass->m_ClassName.c_str(), luaclass->newLuaClass<Args...>);
		luaL_newmetatable(m_pState, luaclass->m_ClassName.c_str());
		//add the destroy function
		lua_pushcfunction(m_pState, luaclass->destroy);	lua_setfield(m_pState, -2, "__gc");
		lua_pushvalue(m_pState, -1);					lua_setfield(m_pState, -2, "__index");
		//add the functions
		luaclass->PushAllFunctions(m_pState);
		//add the variables
		lua_pop(m_pState, 1);
	}
};