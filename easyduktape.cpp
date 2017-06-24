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

#include "easyduktape.h"


eDUK::eDUK()
{
	if (this->ctx != nullptr)
		duk_destroy_heap(this->ctx);
	this->ctx = duk_create_heap_default();
}

eDUK::~eDUK()
{
	this->Destroy();
}

void eDUK::Destroy()
{
	if (this->ctx != nullptr)
		duk_destroy_heap(this->ctx);
}

bool eDUK::InitEvents()
{
	char* ev_script =
		"function ev(){\n"
		"this._ev = {};\n"
		"this.on = function(eventname, callback) {\n"
		"this._ev[eventname] = callback;\n"
		"}\n"
		"this.fire = function(eventname) {\n"
		"return this._ev[eventname].apply(null, Array.prototype.slice.call(arguments, 1));\n"
		"}}";
	duk_pcompile_string(this->ctx, 0, ev_script);
	if (duk_pcall(this->ctx, 0) != 0)
	{
		printf("Error: Failed to Init event system. %s\n", duk_safe_to_string(this->ctx, -1));
		this->pop();
		return false;
	}
	this->pop();
	return true;
}

bool eDUK::LoadScript(char* scriptname)
{
	if (this->ctx == nullptr) return false;
	if (scriptname == '\0') return false;
	if (!strlen(scriptname)) return false;
	if (duk_pcompile_file(this->ctx, 0, scriptname) != 0)
	{
		printf("Error: Failed to compile (%s). %s\n", scriptname, duk_safe_to_string(this->ctx, -1));
		this->pop();
		return false;
	}
	if (duk_pcall(this->ctx, 0) != 0)
	{
		printf("Error: Failed to execute (%s). %s\n", scriptname, duk_safe_to_string(this->ctx, -1));
		this->pop();
		return false;
	}
	this->pop();
	return true;
}

bool eDUK::RegisterFunction(char* name, duk_c_function func, int nargs)
{
	if (this->ctx == nullptr) return false;
	if (duk_get_top(this->ctx) == 0)
		duk_push_global_object(this->ctx);
	duk_push_c_function(this->ctx, func, nargs);
	bool success = duk_put_prop_string(this->ctx, -2, name) == 1;
	this->pop();
	return success;
}

bool eDUK::RegisterObject(char* name)
{
	if (this->ctx == nullptr) return false;
	if (duk_get_top(this->ctx) == 0)
		duk_push_global_object(this->ctx);
	duk_push_object(this->ctx);
	bool success = duk_put_prop_string(this->ctx, -2, name) == 1;
	this->pop();
	return success;
}

bool eDUK::RegisterVariable(char* name, duk_int_t type, void* value)
{
	if (this->ctx == nullptr) return false;
	if (duk_get_top(this->ctx) == 0)
		duk_push_global_object(this->ctx);
	switch (type)
	{
	case DUK_TYPE_BOOLEAN:
		duk_push_boolean(this->ctx, (duk_bool_t)value);
		break;
	case DUK_TYPE_NULL:
		duk_push_null(this->ctx);
		break;
	case DUK_TYPE_NUMBER:
		duk_push_int(this->ctx, (duk_int_t)value);
		break;
	case DUK_TYPE_STRING:
		duk_push_string(this->ctx, (const char*)value);
		break;
	case DUK_TYPE_UNDEFINED:
	default:
		duk_push_undefined(this->ctx);
		break;
	}
	bool success = duk_put_prop_string(this->ctx, -2, name) == 1;
	this->pop();
	return success;
}

eDUK* eDUK::GetObject(char* name)
{
	if (duk_get_top(this->ctx) == 0)
		duk_push_global_object(this->ctx);
	duk_get_prop_string(this->ctx, -1, name);
	return this;
}

eDUK* eDUK::GetGlobalObject()
{
	duk_push_global_object(this->ctx);
	return this;
}

bool eDUK::CallFunction(const char* name, const char* format, ...)
{
	va_list args;
	va_start(args, format);
	int numargs = 0;

	duk_push_global_object(this->ctx);
	duk_get_prop_string(this->ctx, -1, name);
	if (duk_is_null_or_undefined(this->ctx, -1))
	{
		this->pop();
		printf("Runtime error: Function (%s) doesn't exist\n", name);
		return false;
	}
	while (*format != '\0')
	{
		numargs++;
		switch (*format)
		{
		case 'i':
			duk_push_number(this->ctx, va_arg(args, int));
			break;
		case 'f':
			duk_push_number(this->ctx, va_arg(args, double));
			break;
		case 's':
			duk_push_string(this->ctx, va_arg(args, const char*));
			break;
		case 'n':
			duk_push_null(this->ctx);
			break;
		case 'b':
			duk_push_boolean(this->ctx, va_arg(args, bool));
			break;
		}
		format++;
	}
	bool success = duk_pcall(this->ctx, numargs) == 0;
	if (!success)
	{
		printf("Runtime error: %s\n", duk_safe_to_string(this->ctx, -1));
	}
	this->pop();
	return success;
}

void eDUK::RegisterEvent()
{
	duk_push_object(this->ctx);
	duk_eval_string(this->ctx, "ev");
	duk_new(this->ctx, 0);
	duk_put_prop_string(this->ctx, -3, "ev");
	this->pop();
}

bool eDUK::FireEvent(const char* eventname, const char* format, ...)
{
	va_list args;
	va_start(args, format);
	int numargs = 0;
	this->GetObject("ev"); // ev object

	if (duk_is_null_or_undefined(this->ctx, -1))
	{
		this->pop();
		printf("Runtime error: event (%s) does not exist\n", eventname);
		return false;
	}

	this->GetObject("fire"); // fire function
	duk_dup(this->ctx, -2); // "this" of the 'ev' object

	duk_push_string(this->ctx, eventname); // ev name
	while (*format != '\0')
	{
		numargs++;
		switch (*format)
		{
		case 'i':
			duk_push_number(this->ctx, va_arg(args, int));
			break;
		case 'f':
			duk_push_number(this->ctx, va_arg(args, double));
			break;
		case 's':
			duk_push_string(this->ctx, va_arg(args, const char*));
			break;
		case 'n':
			duk_push_null(this->ctx);
			break;
		case 'b':
			duk_push_boolean(this->ctx, va_arg(args, bool));
			break;
		}
		format++;
	}
	bool success = duk_pcall_method(this->ctx, ++numargs) == 0;
	if (!success)
	{
		printf("Runtime error: %s\n", duk_safe_to_string(this->ctx, -1));
	}
	return success;
}

bool eDUK::IsContext(duk_context* g_ctx)
{
	return this->ctx == g_ctx;
}

int eDUK::pop(int n)
{
	int old_top = duk_get_top(this->ctx);
	duk_pop_n(this->ctx, n == 0 ? old_top : n);
	return n == 0 ? old_top : n;
}