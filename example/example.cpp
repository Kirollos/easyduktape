/*
Simple example of how to deal with most of the easy-duktape functions
*/

#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <ctime>
#include "easyduktape.h" // must include this

eDUK* duk; // define the instance pointer for our easy-duktape class

/* DUKTAPE FUNCTIONS */
// Regarding functions, you have to define c-function in order for duktape to call it when it's orded in the stack
// To simplify that, I've created a macro in easyduktape.h
// Usage: EDUK_FUNCTION(func_name)
// func_name can be anything, doesn't have to match the function name in javascript-side.
EDUK_FUNCTION(gettimestamp) // defining our function
{
	if (!duk->IsContext(ctx)) return 0; // Checking whether the context that has called this function is within our context
	std::time_t time = std::time(nullptr);
	// Push global object into stack, push our global object "$time" into stack, then execute the event-fire function
	duk->GetGlobalObject()->GetObject("$time")->FireEvent("ts_fire", "i", time);
	// Pop Results of the above execution ^
	duk->pop();
	// Push return value of this function
	duk_push_int(ctx, (duk_int_t)time);
	// Must return 1 for the stack to return the value ^
	return 1;
}

EDUK_FUNCTION(win_setcolour)
{
	if (!duk->IsContext(ctx)) return 0;
	duk_int_t back = duk_require_int(ctx, 0); // Grab first parameter from the stack
	duk_int_t fore = duk_require_int(ctx, 1); // Grab second parameter from the stack
	if (fore > 16 || back > 16) // more than 0xf
	{
		printf("$os.windows.setcolour error! color ranges from 0-16\r\n");
		duk_push_boolean(ctx, false); // Return value false
		return 1;
	}
	char* szCmd = new char[10];
	sprintf(szCmd, "color %x", ((back << 4) + fore));
	system(szCmd);
	delete(szCmd);
	printf(">>Setting colour to (%i,%i)!\r\n", back, fore);
	duk_push_int(ctx, (back << 4) + fore); // Return value of the new colour
	return 1;
}
////////////////////////////////////////////

int main()
{
	duk = new eDUK(); // Initialize duktape context with default heap size
	duk->InitEvents(); // Initialize event system
	duk->RegisterObject("$time"); // Register new global object "$time"
	duk->GetObject("$time")->RegisterEvent(); // Register event system for global object "$time"
	duk->GetObject("$time")->RegisterFunction("ts", gettimestamp, 0); // Register function "ts" in global object "$time"
	duk->RegisterObject("$os"); // Register new global object "$os"
	duk->GetObject("$os")->RegisterObject("windows"); // Register new object "windows" in global object "$os"
#if defined _WIN32 || defined _WIN64
	duk->GetObject("$os")->GetObject("windows")->RegisterVariable("iswindows", DUK_TYPE_BOOLEAN, (bool*)true); // Register new variable with boolean value "true"
	duk->GetObject("$os")->GetObject("windows")->RegisterFunction("setcolour", win_setcolour, 2); //(back, fore) // Register new function in object "$os.windows"
#else
	duk->GetObject("$os")->GetObject("windows")->RegisterVariable("iswindows", DUK_TYPE_BOOLEAN, (bool*)false);
#endif
	duk->LoadScript("example.js"); // Compile and load script "example.js"
	duk->CallFunction("main", ""); // Call defined function in the stack (from loaded script)
	getchar();
	delete duk; // Destroy the context
	return 0;
}