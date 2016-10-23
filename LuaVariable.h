#pragma once
#include "LuaFunction.h"

class LuaVariable
{
public:
	LuaVariable(lua_State* state,string name);
	~LuaVariable();

	//variable type
	enum Type
	{
		NONE,
		BOOL,
		INT,
		DOUBLE,
		STRING,
		TABLE,
		FUNCTION
	};
	Type m_Type;

	//set variables
	void operator=(const bool& other)
	{
		m_Type = BOOL;
		m_BoolValue = other;
		ToLua();
	}
	void operator=(const int& other)
	{
		m_Type = INT;
		m_IntValue = other;
		ToLua();
	}
	void operator=(const double& other)
	{
		m_Type = DOUBLE;
		m_NumberValue = other;
		ToLua();
	}
	void operator=(const char* other)
	{
		m_StringValue = other;
		m_Type = STRING;
		ToLua();
	}
	void operator=(const string& other)
	{
		m_StringValue = other;
		m_Type = STRING;
		ToLua();
	}

	//get variables
	operator bool()
	{
		//to bool if bool not set yet
		if (m_Type != BOOL)
		{
			ToValue(BOOL);
		}
		return m_BoolValue;
	}
	operator int()
	{
		//to int if int is not set yet
		if (m_Type != INT)
		{
			ToValue(INT);
		}
		return m_IntValue;
	}
	operator double()
	{
		//to double if double is not set yet
		if (m_Type != DOUBLE)
		{
			ToValue(DOUBLE);
		}
		return m_NumberValue;
	}
	operator string()
	{
		if (m_Type != STRING)//convert value to string if its not a string yet
		{
			ToValue(STRING);
		}
		return m_StringValue;//return string value
	}

	void GetFromStack();

	//get table fields
	LuaVariable operator[](int);
	LuaVariable operator[](const char*);
	LuaVariable operator[](string);

	//set a c function to lua
	template<typename A, typename ...args>
	void operator=(A(*other)(args...))
	{
		FunWrapper<A, args...>* wrap = new FunWrapper<A, args...>();
		wrap->function = other;
		lua_pushlightuserdata(m_pState, (FunWrapper<A, args...>*)wrap);
		lua_pushcclosure(m_pState, FunWrapper<A, args...>::WrapCFunc, 1);
		lua_setglobal(m_pState, m_Name.c_str());
	}

	//call lua function
	template<typename A,typename... T>
	vector<LuaVariable> operator()(A Head,T... args)
	{
		m_ArgumentAmount++;
		PushArgument(m_pState, Head);
		return operator()(args...);
	}
	vector<LuaVariable> operator()();
private:
	lua_State* m_pState;

	//print lua errors
	void PrintError(int error);
	//convert type to expected type
	void ToValue(Type type);
	//push value to lua
	void ToLua();

	//put on stack
	int		m_ArgumentAmount;
	string	m_Name;
	string	m_TableName;

	//the different value types
	string	m_StringValue;
	int		m_IntValue;
	double	m_NumberValue;
	bool	m_BoolValue;
};

