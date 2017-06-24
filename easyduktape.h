/*
	Copyright 2016 Kirollos

	Licensed under the Apache License, Version 2.0 (the "License");
	you may not use this file except in compliance with the License.
	You may obtain a copy of the License at

	http://www.apache.org/licenses/LICENSE-2.0

	Unless required by applicable law or agreed to in writing, software
	distributed under the License is distributed on an "AS IS" BASIS,
	WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
	See the License for the specific language governing permissions and
	limitations under the License.
*/

#pragma once

#ifndef __E_DUKTAPE_H
#define __E_DUKTAPE_H
#include "duktape.h" // 1.5.0
#include <vector>

#define EDUK_FUNCTION(func_name) duk_ret_t func_name (duk_context* ctx)

class eDUK
{
private:
	duk_context* ctx = nullptr;
public:
	eDUK();
	~eDUK();
	void Destroy();
	bool InitEvents();
	bool LoadScript(char* scriptname);
	bool RegisterFunction(char* name, duk_c_function func, int nargs);
	bool RegisterObject(char* name);
	bool RegisterVariable(char* name, duk_int32_t type, void* value);
	eDUK* GetObject(char* name);
	eDUK* GetGlobalObject();
	bool CallFunction(const char* name, const char* format, ...);
	void RegisterEvent();
	bool FireEvent(const char* eventname, const char* format, ...);
	bool IsContext(duk_context* g_ctx);
	int pop(int n = 0);
};

#endif