#include "stdafx.h"
#include "LuaVariable.h"

LuaVariable::LuaVariable(lua_State* state, string name)
	:m_Name(name)
	,m_Type(Type::NONE)
	,m_ArgumentAmount(0)
	,m_pState(state)
{
}

LuaVariable::~LuaVariable()
{
}

//get value from the lua stack
void LuaVariable::GetFromStack()
{
	//find the variable type 
	int type = lua_type(m_pState, -1);
	if (lua_gettop(m_pState) > 0)
	{
		//convert lua type to c++ type
		switch (type)
		{
		case LUA_TSTRING:
			(*this) = (string)lua_tostring(m_pState, -1);
			break;
		case LUA_TNUMBER:
		{
			double t = (double)lua_tonumber(m_pState, -1);
			double part = t;
			if (modf(part, &part) == 0)
			{
				(*this) = (int)t;
			}
			else
			{
				(*this) = (double)t;
			}
		}
		break;
		case LUA_TBOOLEAN:
			(*this) = lua_toboolean(m_pState, -1) > 0;
			break;
		case LUA_TTABLE:
		{
			//set table name
			m_TableName = m_Name;
		}
		break;
		case LUA_TFUNCTION:
		{
		}
		break;
		default:
			break;
		}
		//remove the pulled value from stack
		lua_remove(m_pState, -1);
	}
}

//get table field
LuaVariable LuaVariable::operator[](const char* value)
{
	return (*this)[(string)value];
}
LuaVariable LuaVariable::operator[](string name)
{
	m_TableName = m_TableName + "." + name;
	//lua_getglobal(m_pState, m_TableName.c_str());
	std::string mystr = m_TableName.substr(0, m_TableName.find(".", 0));
	lua_getglobal(m_pState, mystr.c_str());
	string currentname = m_TableName.substr(m_TableName.find(".", 0) + 1);

	if (m_TableName.find(".", 0) != std::string::npos)
	{
		while (currentname.find(".", 0) != std::string::npos)
		{
			string temp = currentname.substr(0, currentname.find(".", 0));
			lua_pushstring(m_pState, temp.c_str());
			lua_gettable(m_pState, -2);
			currentname = currentname.substr(currentname.find(".", 0) + 1);
		}

		lua_pushstring(m_pState, name.c_str());
		lua_gettable(m_pState, -2);
	}

	LuaVariable result(m_pState, name);
	result.GetFromStack();
	result.m_TableName = m_TableName;
	return result;
}
LuaVariable LuaVariable::operator[](int id)
{
	lua_pushinteger(m_pState, id);
	if (lua_gettop(m_pState) > 2)
	{
		lua_gettable(m_pState, -2);
	}
	LuaVariable result(m_pState, to_string(id));
	GetFromStack();
	return result;
}

//call lua function
vector<LuaVariable> LuaVariable::operator()()
{
	vector<LuaVariable> values;
	lua_pushglobaltable(m_pState);
	lua_getfield(m_pState, -1, m_Name.c_str());
	for (size_t i = m_ArgumentAmount; i > 0; i--)
	{
		int id = i + (m_ArgumentAmount - i);
		id += 2;
		lua_pushvalue(m_pState, -id);
	}
	if (lua_isfunction(m_pState, -m_ArgumentAmount - 1))
	{
		int top = lua_gettop(m_pState);
		int v = lua_pcall(m_pState, m_ArgumentAmount, LUA_MULTRET, 0);
		if (v != LUA_OK)
		{
			PrintError(v);
		}

		int nresults = lua_gettop(m_pState) - top + m_ArgumentAmount + 1;
		for (int i = 0; i < nresults; i++)
		{
			LuaVariable result(m_pState, to_string(i));
			result.GetFromStack();
			values.push_back(result);
		}
		for (int i = 0; i < (m_ArgumentAmount); i++)
		{
			lua_pop(m_pState, 1);
		}
		lua_pop(m_pState, 1);

		m_ArgumentAmount = 0;
		reverse(values.begin(), values.end());
	}
	else
	{
		cout << "not a function" << endl;
	}

	return values;
}

//print lua errors
void LuaVariable::PrintError(int error)
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
		lua_pop(m_pState, 1); /* pop error message from the stack */
	}
}

//type conversions were needed
void LuaVariable::ToValue(Type type)
{
	if (m_Type == NONE)
	{
#ifdef _WIN32

		MessageBox(0, "LuaVariable type was not set yet", "LuaVariable", MB_OK);
		exit(-1);
#endif
	}
	switch (type)
	{
	case LuaVariable::NONE:
		break;
	case LuaVariable::BOOL:
		if (m_Type == INT)
		{
			m_BoolValue = m_IntValue >0;
		}
		else if (m_Type == DOUBLE)
		{
			m_BoolValue = m_NumberValue > 0;
		}
		else if (m_Type == STRING)
		{
			m_BoolValue = m_StringValue == "true" || m_StringValue == "1";
		}
		break;
	case LuaVariable::INT:
		if (m_Type == BOOL)
		{
			m_IntValue = (int)m_BoolValue;
		}
		else if (m_Type == DOUBLE)
		{
			m_IntValue = (int)m_NumberValue;
		}
		else if (m_Type == STRING)
		{
			m_IntValue = stoi(m_StringValue);
		}
		break;
	case LuaVariable::DOUBLE:
		if (m_Type == BOOL)
		{
			m_NumberValue = (double)m_BoolValue;
		}
		else if (m_Type == INT)
		{
			m_NumberValue = (double)m_IntValue;
		}
		else if (m_Type == STRING)
		{
			m_NumberValue = stod(m_StringValue);
		}
		break;
	case LuaVariable::STRING:
		if (m_Type == BOOL)
		{
			m_StringValue = m_BoolValue ? "true" : "false";
		}
		else if (m_Type == INT)
		{
			m_StringValue = to_string(m_IntValue);
		}
		else if (m_Type == DOUBLE)
		{
			m_StringValue = to_string(m_NumberValue);
		}
		break;
	default:
		break;
	}
}

//push value to lua
void LuaVariable::ToLua()
{
	if (m_TableName.find('.') != string::npos)
	{
		//get upper table
		std::string mystr = m_TableName.substr(0, m_TableName.find(".", 0));
		lua_getglobal(m_pState, mystr.c_str());


		string currentname = m_TableName.substr(m_TableName.find(".", 0) + 1);
		while (currentname.find(".", 0) != std::string::npos)
		{
			//get name of current table level
			string temp = currentname.substr(0, currentname.find(".", 0));
			lua_pushstring(m_pState, temp.c_str());
			lua_gettable(m_pState, -2);
			currentname = currentname.substr(currentname.find(".", 0) + 1);
		}
		lua_pushstring(m_pState, currentname.c_str());
		lua_gettable(m_pState, -2);

		switch (m_Type)
		{
		case LuaVariable::NONE:
			#ifdef _WIN32
						MessageBox(0, "LuaVariable type was not set yet", "LuaVariable", MB_OK);
						exit(-1);
			#endif
			break;
		case LuaVariable::BOOL:
		{
			lua_pushboolean(m_pState, m_BoolValue);
			lua_setfield(m_pState, -3, m_Name.c_str());
		}
		break;
		case LuaVariable::INT:
			lua_pushinteger(m_pState, m_IntValue);
			lua_setfield(m_pState, -3, m_Name.c_str());
			break;
		case LuaVariable::DOUBLE:
			lua_pushnumber(m_pState, m_NumberValue);
			lua_setfield(m_pState, -3, m_Name.c_str());
			break;
		case LuaVariable::STRING:
		{
			lua_pushstring(m_pState, m_StringValue.c_str());
			lua_setfield(m_pState, -3, m_Name.c_str());
		}
		break;
		default:
			break;
		}
		lua_settop(m_pState, 0);
	}
	else
	{
		switch (m_Type)
		{
		case LuaVariable::NONE:
			#ifdef _WIN32
					MessageBox(0, "LuaVariable type was not set yet", "LuaVariable", MB_OK);
					exit(-1);
			#endif
			break;
		case LuaVariable::BOOL:
		{
			lua_pushboolean(m_pState, m_BoolValue);
			lua_setglobal(m_pState, m_Name.c_str());
		}
		break;
		case LuaVariable::INT:
			lua_pushinteger(m_pState, m_IntValue);
			lua_setglobal(m_pState, m_Name.c_str());
			break;
		case LuaVariable::DOUBLE:
			lua_pushnumber(m_pState, m_NumberValue);
			lua_setglobal(m_pState, m_Name.c_str());
			break;
		case LuaVariable::STRING:
		{
			lua_pushstring(m_pState, m_StringValue.c_str());
			lua_setglobal(m_pState, m_Name.c_str());
		}
			break;
		default:
			break;
		}
	}
}


//push arguments on the stack
void PushArgument(lua_State* l, int arg)
{
	lua_pushnumber(l, arg);
}
void PushArgument(lua_State* l, string arg)
{
	lua_pushstring(l, arg.c_str());
}
void PushArgument(lua_State* l, double arg)
{
	lua_pushnumber(l, arg);
}
void PushArgument(lua_State* l, bool arg)
{
	lua_pushboolean(l, arg);
}
void PushArgument(lua_State* l, const char* arg)
{
	PushArgument(l, string(arg));
}
