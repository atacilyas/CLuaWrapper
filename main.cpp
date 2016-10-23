#include "stdafx.h"
#include "LuaScript.h"
#include "LuaClass.h"
// Windows Header Files:
#include <windows.h>
// C RunTime Header Files
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <crtdbg.h>
#include <Initguid.h>
#include <dxgidebug.h>
#include <time.h>

double add(int k,double t)
{
	return k+t;
}

string PrintString()
{
	return "This is a string!";
}

class Counter
{
public:
	Counter(int initial,string name)
	{
		value = initial;
		m_name = name;
	}
	~Counter()
	{
	}
	int GetValue()
	{
		value = value + 1;
		return value;
	}
	void SetValue(int i)
	{
		value = i;
	}
	string PrintName()
	{
		return m_name;
	}

	string newvalue = "toromo";
private:
	int value;
	string m_name;
};

//global variables check
//lua functions check
//C functions check
//table variable (GET done SET done)
//classes (functions done,variables TODO)
//initialized classes TODO

int testlua()
{
	LuaScript t;
	//load lua file
	t.LoadFile("main.lua");
	//change or set a variable
	t["numberOne"] = 400;
	//run some lua code
	t("numberOne = numberOne/200");
	cout << "variable number: " << (string)t["numberOne"] << endl;
	//Set the C functions to lua
	t["AddingFunction"] = add;
	t["PrintString"] = PrintString;
	//call lua function
	auto l = t["AddingFunction"](10, 55.20);
	cout << "return value: " << ((string)l.at(0)).c_str() << endl;
	auto i = t["PrintString"]();
	cout << "return value: " << ((string)i.at(0)).c_str() << endl;

	//set/get table
	t["table"]["var2"]["var3"] = 12;
	cout << "table field number: " << (string)t["table"]["var2"]["var3"] << endl;

	LuaClass<Counter, int(Counter::*)(),void	(Counter::*)(int), string(Counter::*)()>*LuaCounter = new
	LuaClass<Counter, int(Counter::*)(),void	(Counter::*)(int), string(Counter::*)()>
		(
			make_pair("GetValue",&Counter::GetValue),
			make_pair("SetValue",&Counter::SetValue),
			make_pair("PrintName",  &Counter::PrintName)
		);

	////set class type
	t.AddClass<int,string>(LuaCounter);
	t["main"]();

	delete LuaCounter;

	system("Pause");
	return 0;
}

int WINAPI wWinMain(HINSTANCE, HINSTANCE, WCHAR*, int)
{
	//notify user if heap is corrupt
	HeapSetInformation(NULL, HeapEnableTerminationOnCorruption, NULL, 0);

	// Enable run-time memory leak check for debug builds.
#if defined(DEBUG) | defined(_DEBUG)
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);

	typedef HRESULT(__stdcall *fPtr)(const IID&, void**);
	HMODULE hDll = LoadLibrary("dxgidebug.dll");
	fPtr DXGIGetDebugInterface = (fPtr)GetProcAddress(hDll, "DXGIGetDebugInterface");

	IDXGIDebug* pDXGIDebug;
	DXGIGetDebugInterface(__uuidof(IDXGIDebug), (void**)&pDXGIDebug);
	//_CrtSetBreakAlloc(190);//break at line
#endif

	testlua();

#if defined(DEBUG) | defined(_DEBUG)

	// unresolved external  
	if (pDXGIDebug) pDXGIDebug->ReportLiveObjects(DXGI_DEBUG_ALL, DXGI_DEBUG_RLO_ALL);
	pDXGIDebug->Release();
#endif

}

int main()
{
	HINSTANCE hInstance = GetModuleHandle(NULL);
	return wWinMain(hInstance, 0, nullptr, SW_SHOW);
}
