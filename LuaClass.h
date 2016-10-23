#pragma once
#include <vector>
#include <functional>
#include "LuaFunction.h"
#include "FuncPtrHelpers.h"
using namespace std;

//a helper to pass tuple arguments through
template<class,typename, typename>
struct strip;

template<class C,typename T,typename ...args>
struct strip<C,T,std::tuple<args...>>
{
	using type = ClassFunWrapper<C,T,args...>;
};

template<class C,typename... funcs>
class LuaClass
{
private:
	//wrap the function pointers in tuple to ClassFunWrapper
	template<class A,class B>
	void WrapFunctions_Helper(A&& k, B&& o)
	{
		k.first = o.first;
		k.second = make_ClassFunWrapper(o.second);
	}

	template<typename TupA, std::size_t... indexA, typename TupB, std::size_t... indexB>
	void WrapFunctions_Expander(TupA&& tupa, std::index_sequence<indexA...>, TupB&& tupb, std::index_sequence<indexB...>)
	{
		using ints = int[];
		(void)ints
		{
			0, (WrapFunctions_Helper(std::get<indexA>(std::forward<TupA>(tupa)), std::get<indexB>(std::forward<TupB>(tupb))), void(), 0)...
		};
	}

	template<typename TupA,typename TupB>
	void WrapFunctions(TupA&& tupa, TupB&& tupb)
	{
		constexpr auto SizeA = std::tuple_size<typename std::decay<TupA>::type>::value;
		constexpr auto SizeB = std::tuple_size<typename std::decay<TupB>::type>::value;

		WrapFunctions_Expander(	std::forward<TupA>(tupa), std::make_index_sequence<SizeA>{}
							,	std::forward<TupB>(tupb), std::make_index_sequence<SizeB>{});
	}
public:
	string m_ClassName;
	tuple<	pair<string,
		typename strip<
				C,//class type
				typename classfunction_traits<funcs>::result_type,//function return types
				typename classfunction_traits<funcs>::argTypes>::type*>...//function argument types
		> publicFunctions;
	LuaClass(pair<string, funcs>... pairs)
	{
		WrapFunctions(publicFunctions, make_tuple(pairs...));

		m_ClassName = typeid(C).name();
		m_ClassName.erase(m_ClassName.begin(), m_ClassName.begin() + m_ClassName.find(' ') + 1);
	}

	~LuaClass()
	{
		constexpr auto Size = std::tuple_size<typename std::decay<decltype(publicFunctions)>::type>::value;
		WrapDeleter_Expander(std::forward<decltype(publicFunctions)>(publicFunctions), std::make_index_sequence<Size>{});
	}

	template<typename... Args>
	static int newLuaClass(lua_State* L)
	{
		vector<LuaVariable> t;
		int argc = lua_gettop(L);
		for (int i = 0; i < argc; i++)
		{
			LuaVariable result(L, "");
			result.GetFromStack();
			t.push_back(result);
		}
		reverse(t.begin(), t.end());

		//tuple the arguments together
		tuple<Args...> argums = make_tuple((Args())...);
		vectorToTuple(argums, t);
		
		*reinterpret_cast<C**>(lua_newuserdata(L, sizeof(C*))) = Create(argums);

		string classname = typeid(C).name();
		classname.erase(classname.begin(), classname.begin() + classname.find(' ') + 1);
		luaL_setmetatable(L, classname.c_str());

		return 1;
	}

	static int destroy(lua_State* L)
	{
		delete *reinterpret_cast<C**>(lua_touserdata(L, 1));
		return 0;
	}

	void PushAllFunctions(lua_State* l)
	{
		constexpr auto Size = std::tuple_size<typename std::decay<decltype(publicFunctions)>::type>::value;
		PushAllFunctions_Expander(l, std::forward<decltype(publicFunctions)>(publicFunctions), std::make_index_sequence<Size>{});
	}

private:
	//make a new variabale of class C
	template<typename Tup, std::size_t... index>
	static C* Create_helper(Tup&& tup, std::index_sequence<index...>)
	{
		return new C(std::get<index>(std::forward<Tup>(tup))...);
	}

	template<typename Tup>
	static C* Create(Tup&& tup)
	{
		constexpr auto Size = std::tuple_size<typename std::decay<Tup>::type>::value;
		return Create_helper(std::forward<Tup>(tup), std::make_index_sequence<Size>{});
	}

	//delete class function wrappers
	template<class A>
	void WrapDeleter_Helper(A&& k)
	{
		auto vv = k;
		if (vv.second != nullptr && vv.second->function != nullptr)
		{
			delete vv.second;
			vv.second = nullptr;
		}
		
		k = make_pair(vv.first,nullptr);
	}

	template<typename Tup, std::size_t... index>
	void WrapDeleter_Expander(Tup&& tup, std::index_sequence<index...>)
	{
		using ints = int[];
		(void)ints
		{
			0, (WrapDeleter_Helper(std::get<index>(std::forward<Tup>(tup))), void(), 0)...
		};
	}

	//push lua functions
	template<typename T, typename... Args>
	void PushAllFunctions_Helper(lua_State* l,pair<string, ClassFunWrapper<C, T, Args...>*>&& k)
	{
		lua_pushlightuserdata(l, (ClassFunWrapper<C, T, Args...>*)k.second);
		lua_pushcclosure(l, ClassFunWrapper<C, T, Args...>::WrapCFunc, 1);
		lua_setfield(l, -2, k.first.c_str());
	}

	template<typename Tup, std::size_t... index>
	void PushAllFunctions_Expander(lua_State* l, Tup&& tup, std::index_sequence<index...>)
	{
		using ints = int[];
		(void)ints
		{
			0, (PushAllFunctions_Helper(l, std::get<index>(std::forward<Tup>(tup))), void(), 0)...
		};
	}
};

