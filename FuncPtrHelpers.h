#pragma once


//get the return type and argument types from a function pointer
template<typename T>
struct function_traits;

template<typename R, typename ...Args>
struct function_traits<std::function<R(Args...)>>
{
	typedef std::tuple<Args...> args_type;
	typedef R result_type;
};

//get the class type return type and argument types from a class function pointer
template<typename T>
struct classfunction_traits;

template<class C, typename R, typename ...Args>
struct classfunction_traits<R(C::*)(Args...)>
{
	typedef std::tuple<Args...> argTypes;//argument types of function pointer
	typedef R result_type;//return type of function pointer
	typedef C class_type;//class type of function pointer
};

