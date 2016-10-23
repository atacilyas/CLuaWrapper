#pragma once
#include <vector>
#include <tuple>
#include <utility>
#include <functional>

class LuaVariable;

void PushArgument(lua_State* l, int arg);
void PushArgument(lua_State* l, string arg);
void PushArgument(lua_State* l, const char* arg);
void PushArgument(lua_State* l, double arg);
void PushArgument(lua_State* l, bool arg);

//convert vector to tuple
template<class... Ts, std::size_t... Is>
void vectorToTuple(std::tuple<Ts...>& t, std::index_sequence<Is...>, vector<LuaVariable> vec)
{
	using ints = int[];
	(void)ints
	{
		//set index element to vector element
		0, (((std::get<Is>(t) = vec.at(Is))), void(), 0)...
	};
}

template<class... Types>
void vectorToTuple(std::tuple<Types...>& t, vector<LuaVariable> vec)
{
	//get index
	vectorToTuple(t, std::index_sequence_for<Types...>(), vec);
}

//call function with tuple as arguments
template<typename Func, typename Tup, std::size_t... index>
decltype(auto) invoke_helper(Func&& func, Tup&& tup, std::index_sequence<index...>)
{
	return func(std::get<index>(std::forward<Tup>(tup))...);
}

template<typename Func, typename Tup>
decltype(auto) invoke(Func&& func, Tup&& tup)
{
	constexpr auto Size = std::tuple_size<typename std::decay<Tup>::type>::value;
	return invoke_helper(std::forward<Func>(func), std::forward<Tup>(tup), std::make_index_sequence<Size>{});
}

template<typename T, typename ...args>
struct FunWrapper
{
	T(*function)(args...) = nullptr;
	static int WrapCFunc(lua_State* l)
	{
		const int n = sizeof...(args);
		// number of input arguments
		int argc = lua_gettop(l);
		// remove unused input args
		for (int i = 0; i<argc - n; i++)
		{
			lua_pop(l, 1);
		}
		// Get all input args from stack
		vector<LuaVariable> t;
		argc = lua_gettop(l);
		for (int i = 0; i < argc; i++)
		{
			LuaVariable result(l, "");
			result.GetFromStack();
			t.push_back(result);
		}
		reverse(t.begin(), t.end());

		//tuple the arguments together
		tuple<args...> argums = make_tuple((args())...);
		vectorToTuple(argums, t);

		//get the function from userdata
		FunWrapper<T, args...>* wrapper;
		wrapper = (FunWrapper<T, args...>*)lua_touserdata(l, lua_upvalueindex(1));
		//call the function
		//auto k = std::invoke<T(*)(args...)>(wrapper->function, std::forward<args>(argums)...);
		auto k = invoke(wrapper->function, argums);

		// push return value;
		PushArgument(l, k);
		return 1;
	}
};

///------------------------------------------------------///
//call class function with tuple as arguments
template<class C, typename Func, typename Tup, std::size_t... index>
decltype(auto) invokeClass_helper(C* classPtr, Func func, Tup&& tup, std::index_sequence<index...>)
{

	return ((*classPtr)->*(func))(std::get<index>(std::forward<Tup>(tup))...);
}

template<class C, typename Func, typename Tup>
decltype(auto) invokeClass(C* classPtr, Func func, Tup&& tup)
{
	constexpr auto Size = std::tuple_size<typename std::decay<Tup>::type>::value;
	return invokeClass_helper(classPtr, func, std::forward<Tup>(tup), std::make_index_sequence<Size>{});
}

//wrap classfunction to make it callable in lua
template<class C, typename T, typename ...args>
struct ClassFunWrapper
{
	ClassFunWrapper(T(C::*func)(args...))
	{
		function = func;
	};
	
	static int WrapCFunc(lua_State* l)
	{
		//get the class from userdata
		auto classptr = luaL_checkudata(l, 1, "Counter");
		auto classcast = reinterpret_cast<C**>(classptr);
		//get the function from userdata
		ClassFunWrapper<C,T, args...>* wrapper;
		wrapper = (ClassFunWrapper<C,T, args...>*)lua_touserdata(l, lua_upvalueindex(1));
		
		const int n = sizeof...(args);
		// number of input arguments
		int argc = lua_gettop(l);
		// remove unused input args
		for (int i = 0; i<argc - n; i++)
		{
			lua_pop(l, 1);
		}
		// Get all input args from stack
		vector<LuaVariable> t;
		argc = lua_gettop(l);
		for (int i = 0; i < argc; i++)
		{
			LuaVariable result(l, "");
			result.GetFromStack();
			t.push_back(result);
		}
		reverse(t.begin(), t.end());
		
		//tuple the arguments together
		tuple<args...> argums = make_tuple((args())...);
		vectorToTuple(argums, t);
		
		//call the function
		auto k = invokeClass(classcast,wrapper->function, argums);
		
		// push return value;
		PushArgument(l, k);
		return 1;
	}
	T(C::*function)(args...) = nullptr;
};

template<class C, typename ...args>
struct ClassFunWrapper<C,void,args...>
{

	ClassFunWrapper(void(C::*func)(args...))
	{
		function = func;
	};
	static int WrapCFunc(lua_State* l)
	{
		//get the class from userdata
		auto classptr = luaL_checkudata(l, 1, "Counter");
		auto classcast = reinterpret_cast<C**>(classptr);
		//get the function from userdata
		ClassFunWrapper<C, void, args...>* wrapper;
		wrapper = (ClassFunWrapper<C, void, args...>*)lua_touserdata(l, lua_upvalueindex(1));

		const int n = sizeof...(args);
		// number of input arguments
		int argc = lua_gettop(l);
		// remove unused input args
		//for (int i = 0; i<argc - n; i++)
		//{
		//	lua_pop(l, 1);
		//}
		// Get all input args from stack
		vector<LuaVariable> t;
		argc = lua_gettop(l);
		for (int i = 0; i < argc+1; i++)
		{
			LuaVariable result(l, "");
			result.GetFromStack();
			if (result.m_Type != LuaVariable::NONE)
			{
				t.push_back(result);
			}
		}
		reverse(t.begin(), t.end());

		//tuple the arguments together
		tuple<args...> argums = make_tuple((args())...);
		if (t.size() > 0)
		{
			vectorToTuple(argums, t);
		}

		//call the function
		invokeClass(classcast, wrapper->function, argums);

		// push return value;
		return 0;
	}
	void(C::*function)(args...) = nullptr;
};

//wrap class function pointer to classfunwrapper
template<class C,typename T, typename ...args>
ClassFunWrapper<C, T, args...>* make_ClassFunWrapper(T(C::*other)(args...))
{
	return new ClassFunWrapper<C, T,args...>(other);
}