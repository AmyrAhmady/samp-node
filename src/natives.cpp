#include <string>
#ifndef WIN32
#include <cstring>
#endif

#include "amx/amx2.h"
#include "common.hpp"
#include "v8.h"
#include "natives.hpp"
#include "sampgdk.h"

namespace sampnode
{
	std::unordered_map<std::string, AMX_NATIVE> pawn_natives_cache = std::unordered_map<std::string, AMX_NATIVE>();

	AMX_NATIVE native::get_address(const std::string& name)
	{
		AMX_NATIVE native;
		auto iter = pawn_natives_cache.find(name);
		if (iter != pawn_natives_cache.end()) native = iter->second;
		else
		{
			native = sampgdk::FindNative(name.c_str());
			if (native)
			{
				pawn_natives_cache[name] = native;
			}
		}
		return native;
	}

	void native::call(const v8::FunctionCallbackInfo<v8::Value>& args)
	{
		v8::Isolate* isolate = args.GetIsolate();
		v8::TryCatch eh(isolate);

		v8::String::Utf8Value str(isolate, args[0]);
		std::string name(*str);

		v8::String::Utf8Value str2(isolate, args[1]);
		char* format(*str2);

		if (!strcmp(format, "undefined"))
			format[0] = '\0';

		AMX_NATIVE native;

		native = get_address(name);
		if (!native)
		{
			L_ERROR << "[callNative] native function: " << name << " not found.";
			return;
		}

		void* params[32];
		cell param_value[20];
		int param_size[32];
		int j = 0;
		int k = 2;
		int vars = 0;
		int strs = 0;
		int strv = 0;
		size_t len = strlen(format);

		char str_format[256]{ '\0' };
		for (unsigned int i = 0; i < len; i++)
		{
			switch (format[i])
			{
			case 'i':
			{
				param_value[i] = args[k]->Int32Value();
				params[j++] = static_cast<void*>(&param_value[i]);
				k++;
				sprintf(str_format, "%si", str_format);
			}
			break;

			case 'f':
			{
				float val = 0.0;
				if (!args[k]->IsUndefined()) val = static_cast<float>(args[k]->NumberValue());

				param_value[i] = amx_ftoc(val);
				params[j++] = static_cast<void*>(&param_value[i]);
				k++;
				sprintf(str_format, "%sf", str_format);
			}
			break;

			case 's':
			{
				v8::String::Utf8Value _str(isolate, args[k]);
				const char* str(*_str);
				size_t slen = strlen(str);
				char* mystr = new char[slen + 1];
				for (size_t x = 0; x < slen; x++)
				{
					mystr[x] = str[x];
				}
				mystr[slen] = '\0';
				params[j] = static_cast<void*>(mystr);
				j++;
				k++;
				sprintf(str_format, "%ss", str_format);
				strs++;
			}
			break;

			// Array of integers
			case 'a':
			{
				if (!args[k]->IsArray())
				{
					args.GetReturnValue().Set(false);
					L_ERROR << "callNative: '" << name << "', parameter " << k << "must be an array";
					return;
				}

				v8::Local<v8::Array> a = v8::Local<v8::Array>::Cast(args[k++]);
				size_t size = a->Length();

				cell* value = new cell[size];

				for (size_t b = 0; b < size; b++)
				{
					value[b] = a->Get(b)->Int32Value();
				}

				sprintf(str_format, "%sa[%i]", str_format, static_cast<int>(size));
				params[j++] = static_cast<void*>(value);
				strs++;
			}
			break;

			// Array of floats
			case 'v':
			{
				if (!args[k]->IsArray())
				{
					args.GetReturnValue().Set(false);
					L_ERROR << "callNative: '" << name << "', parameter " << k << "must be an array";
					return;
				}
				v8::Local<v8::Array> a = v8::Local<v8::Array>::Cast(args[k++]);
				size_t size = a->Length();

				cell* value = new cell[size];

				for (size_t b = 0; b < size; b++)
				{
					float val = static_cast<float>(a->Get(b)->NumberValue());
					value[b] = amx_ftoc(val);

				}
				sprintf(str_format, "%sa[%i]", str_format, size);
				params[j++] = static_cast<void*>(value);
				strs++;

			}
			break;

			case 'F':
			case 'I':
			{
				vars++;
				params[j++] = static_cast<void*>(&param_value[i]);
				sprintf(str_format, "%sR", str_format);
			}
			break;

			case 'A':
			{
				const int size = args[k]->Int32Value();
				param_size[j] = size;
				cell* value = new cell[size];
				for (int c = 0; c < size; c++)
				{
					value[c] = 0;
				}
				params[j++] = static_cast<void*>(value);
				sprintf(str_format, "%sA[%i]", str_format, size);
				vars++;
			}
			break;

			case 'V':
			{
				const int size = args[k]->Int32Value();
				param_size[j] = size;
				cell* value = new cell[size];
				float fl = 0.0;
				for (int c = 0; c < size; c++)
				{
					value[c] = amx_ftoc(fl);
				}

				params[j++] = static_cast<void*>(value);
				sprintf(str_format, "%sA[%i]", str_format, size);
				vars++;
			}
			break;
			
			case 'S':
			{

				unsigned int strl = args[k++]->Int32Value();
				param_size[j] = static_cast<cell>(strl);

				if (strl < 1)
				{
					L_ERROR << "callNative: '" << name << "' - String length can't be 0";
					return;
				}
				sprintf(str_format, "%sS[%i]", str_format, strl);

				char* mycell = new char[strl]();
				params[j++] = &mycell[0];
				vars++;
				i++;
			}
			break;

			}
		}

		int32_t retval = sampgdk::InvokeNativeArray(native, str_format, params);

		if (vars > 0 || strs > 0)
		{
			j = 0;
			vars = 0;
			int sk = 0;

			v8::Local<v8::Array> arr = v8::Array::New(args.GetIsolate(), vars);

			for (unsigned int i = 0; i < len; i++)
			{
				switch (format[i])
				{
				case 'i':
				case 'f':
				{
					j++;
				}
				break;
				case 's':
				case 'a':
				case 'v':
				{
					delete[] static_cast<char*>(params[j++]);
				}
				break;
				case 'A':
				{
					int size = param_size[j];
					v8::Local<v8::Array> rArr = v8::Array::New(args.GetIsolate(), size);
					cell* prams = static_cast<cell*>(params[j]);
					for (int c = 0; c < size; c++)
					{
						rArr->Set(c, v8::Integer::New(args.GetIsolate(), prams[c]));
					}

					arr->Set(vars++, rArr);
					delete[] static_cast<char*>(params[j++]);
				}
				break;

				case 'V':
				{
					cell* param_array = static_cast<cell*>(params[j]);
					int size = param_size[j];
					v8::Local<v8::Array> rArr = v8::Array::New(args.GetIsolate(), size);
					for (int c = 0; c < size; c++)
					{
						rArr->Set(c, v8::Number::New(args.GetIsolate(), amx_ctof(param_array[c])));
					}
					arr->Set(vars++, rArr);
					delete[] static_cast<char*>(params[j++]);
				}
				break;
				case 'I':
				{
					int val = *static_cast<cell*>(params[j++]);
					arr->Set(vars++, v8::Integer::New(args.GetIsolate(), val));

				}
				break;
				case 'F':
				{
					float val = amx_ctof(*static_cast<cell*>(params[j++]));
					arr->Set(vars++, v8::Number::New(args.GetIsolate(), val));
				}
				break;
				case 'S':
				{

					size_t s_len = param_size[j];
					char* s_str = static_cast<char*>(params[j]);
					s_str[s_len - 1] = '\0';

					arr->Set(vars++, v8::String::NewFromUtf8(args.GetIsolate(), s_str));
					i++;
					delete[] static_cast<char*>(params[j++]);

				}
				break;
				}
			}

			if (vars == 1)
			{
				v8::Local<v8::Value> jsval = arr->Get(0);
				args.GetReturnValue().Set(jsval);
			}
			else if (vars > 1)
				args.GetReturnValue().Set(arr);
			else
				args.GetReturnValue().Set(retval);
		}
		else
		{
			args.GetReturnValue().Set(retval);
		}

		if (eh.HasCaught())
		{
			v8::String::Utf8Value str(isolate, eh.Exception());
			v8::String::Utf8Value stack(isolate, eh.StackTrace(args.GetIsolate()->GetCallingContext()).ToLocalChecked());

			L_ERROR << "[samp-node] event handling function in resource: " << *str << "\nstack:\n" << *stack << "\n";
		}
	}

	void native::call_float(const v8::FunctionCallbackInfo<v8::Value>& args)
	{
		v8::Isolate* isolate = args.GetIsolate();
		v8::TryCatch eh(isolate);

		v8::String::Utf8Value str(isolate, args[0]);
		std::string name(*str);

		v8::String::Utf8Value str2(isolate, args[1]);
		char* format(*str2);

		if (!strcmp(format, "undefined"))
			format[0] = '\0';

		AMX_NATIVE native;

		native = get_address(name);
		if (!native)
		{
			L_ERROR << "[callNative] native function: " << name << " not found.";
			return;
		}

		void* params[32];
		cell param_value[20];
		int param_size[32];
		int j = 0;
		int k = 2;
		int vars = 0;
		int strs = 0;
		int strv = 0;
		size_t len = strlen(format);

		char str_format[256]{ '\0' };
		for (unsigned int i = 0; i < len; i++)
		{
			switch (format[i])
			{
			case 'i':
			{
				param_value[i] = args[k]->Int32Value();
				params[j++] = static_cast<void*>(&param_value[i]);
				k++;
				sprintf(str_format, "%si", str_format);
			}
			break;

			case 'f':
			{
				float val = 0.0;
				if (!args[k]->IsUndefined()) val = static_cast<float>(args[k]->NumberValue());

				param_value[i] = amx_ftoc(val);
				params[j++] = static_cast<void*>(&param_value[i]);
				k++;
				sprintf(str_format, "%sf", str_format);
			}
			break;

			case 's':
			{
				v8::String::Utf8Value _str(isolate, args[k]);
				const char* str(*_str);
				size_t slen = strlen(str);
				char* mystr = new char[slen + 1];
				for (size_t x = 0; x < slen; x++)
				{
					mystr[x] = str[x];
				}
				mystr[slen] = '\0';
				params[j] = static_cast<void*>(mystr);
				j++;
				k++;
				sprintf(str_format, "%ss", str_format);
				strs++;
			}
			break;

			// Array of integers
			case 'a':
			{
				if (!args[k]->IsArray())
				{
					args.GetReturnValue().Set(false);
					L_ERROR << "callNative: '" << name << "', parameter " << k << "must be an array";
					return;
				}

				v8::Local<v8::Array> a = v8::Local<v8::Array>::Cast(args[k++]);
				size_t size = a->Length();

				cell* value = new cell[size];

				for (size_t b = 0; b < size; b++)
				{
					value[b] = a->Get(b)->Int32Value();
				}

				sprintf(str_format, "%sa[%i]", str_format, static_cast<int>(size));
				params[j++] = static_cast<void*>(value);
				strs++;
			}
			break;

			// Array of floats
			case 'v':
			{
				if (!args[k]->IsArray())
				{
					args.GetReturnValue().Set(false);
					L_ERROR << "callNative: '" << name << "', parameter " << k << "must be an array";
					return;
				}
				v8::Local<v8::Array> a = v8::Local<v8::Array>::Cast(args[k++]);
				size_t size = a->Length();

				cell* value = new cell[size];

				for (size_t b = 0; b < size; b++)
				{
					float val = static_cast<float>(a->Get(b)->NumberValue());
					value[b] = amx_ftoc(val);

				}
				sprintf(str_format, "%sa[%i]", str_format, size);
				params[j++] = static_cast<void*>(value);
				strs++;

			}
			break;

			case 'F':
			case 'I':
			{
				vars++;
				params[j++] = static_cast<void*>(&param_value[i]);
				sprintf(str_format, "%sR", str_format);
			}
			break;

			case 'A':
			{
				const int size = args[k]->Int32Value();
				param_size[j] = size;
				cell* value = new cell[size];
				for (int c = 0; c < size; c++)
				{
					value[c] = 0;
				}
				params[j++] = static_cast<void*>(value);
				sprintf(str_format, "%sA[%i]", str_format, size);
				vars++;
			}
			break;

			case 'V':
			{
				const int size = args[k]->Int32Value();
				param_size[j] = size;
				cell* value = new cell[size];
				float fl = 0.0;
				for (int c = 0; c < size; c++)
				{
					value[c] = amx_ftoc(fl);
				}

				params[j++] = static_cast<void*>(value);
				sprintf(str_format, "%sA[%i]", str_format, size);
				vars++;
			}
			break;

			case 'S':
			{

				unsigned int strl = args[k++]->Int32Value();
				param_size[j] = static_cast<cell>(strl);

				if (strl < 1)
				{
					L_ERROR << "callNative: '" << name << "' - String length can't be 0";
					return;
				}
				sprintf(str_format, "%sS[%i]", str_format, strl);

				char* mycell = new char[strl]();
				params[j++] = &mycell[0];
				vars++;
				i++;
			}
			break;

			}
		}

		int32_t retval = sampgdk::InvokeNativeArray(native, str_format, params);

		if (vars > 0 || strs > 0)
		{
			j = 0;
			vars = 0;
			int sk = 0;

			v8::Local<v8::Array> arr = v8::Array::New(args.GetIsolate(), vars);

			for (unsigned int i = 0; i < len; i++)
			{
				switch (format[i])
				{
				case 'i':
				case 'f':
				{
					j++;
				}
				break;
				case 's':
				case 'a':
				case 'v':
				{
					delete[] static_cast<char*>(params[j++]);
				}
				break;
				case 'A':
				{
					int size = param_size[j];
					v8::Local<v8::Array> rArr = v8::Array::New(args.GetIsolate(), size);
					cell* prams = static_cast<cell*>(params[j]);
					for (int c = 0; c < size; c++)
					{
						rArr->Set(c, v8::Integer::New(args.GetIsolate(), prams[c]));
					}

					arr->Set(vars++, rArr);
					delete[] static_cast<char*>(params[j++]);
				}
				break;

				case 'V':
				{
					cell* param_array = static_cast<cell*>(params[j]);
					int size = param_size[j];
					v8::Local<v8::Array> rArr = v8::Array::New(args.GetIsolate(), size);
					for (int c = 0; c < size; c++)
					{
						rArr->Set(c, v8::Number::New(args.GetIsolate(), amx_ctof(param_array[c])));
					}
					arr->Set(vars++, rArr);
					delete[] static_cast<char*>(params[j++]);
				}
				break;
				case 'I':
				{
					int val = *static_cast<cell*>(params[j++]);
					arr->Set(vars++, v8::Integer::New(args.GetIsolate(), val));

				}
				break;
				case 'F':
				{
					float val = amx_ctof(*static_cast<cell*>(params[j++]));
					arr->Set(vars++, v8::Number::New(args.GetIsolate(), val));
				}
				break;
				case 'S':
				{

					size_t s_len = param_size[j];
					char* s_str = static_cast<char*>(params[j]);
					s_str[s_len - 1] = '\0';

					arr->Set(vars++, v8::String::NewFromUtf8(args.GetIsolate(), s_str));
					i++;
					delete[] static_cast<char*>(params[j++]);

				}
				break;
				}
			}

			if (vars == 1)
			{
				v8::Local<v8::Value> jsval = arr->Get(0);
				args.GetReturnValue().Set(jsval);
			}
			else if (vars > 1)
			{
				args.GetReturnValue().Set(arr);
			}
			else
			{
				args.GetReturnValue().Set(static_cast<double>(amx_ctof(retval)));
			}
				
		}
		else
		{
			args.GetReturnValue().Set(static_cast<double>(amx_ctof(retval)));
		}

		if (eh.HasCaught())
		{
			v8::String::Utf8Value str(isolate, eh.Exception());
			v8::String::Utf8Value stack(isolate, eh.StackTrace(args.GetIsolate()->GetCallingContext()).ToLocalChecked());

			L_ERROR << "[samp-node] event handling function in resource: " << *str << "\nstack:\n" << *stack << "\n";
		}
	}
}