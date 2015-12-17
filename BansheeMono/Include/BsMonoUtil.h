#pragma once

#include "BsMonoPrerequisites.h"
#include "BsException.h"
#include "BsDebug.h"
#include "BsMonoArray.h"
#include <mono/jit/jit.h>
#include <codecvt>

namespace BansheeEngine
{
	/**
	 * @brief	Utility class containing methods for various common Mono/Script related
	 *			operations.
	 */
	class BS_MONO_EXPORT MonoUtil
	{
	public:
		/**
		 * @brief	Converts a Mono (i.e. managed) string to a native wide string.
		 */
		static WString monoToWString(MonoString* str)
		{
			if(str == nullptr)
				return StringUtil::WBLANK;

			int len = mono_string_length(str);
			mono_unichar2* monoChars = mono_string_chars(str);

			WString ret(len, '0');
			for(int i = 0; i < len; i++)
				ret[i] = monoChars[i];

			return ret;
		}

		/**
		 * @brief	Converts a Mono (i.e. managed) string to a native narrow string.
		 */
		static String monoToString(MonoString* str)
		{
			if(str == nullptr)
				return StringUtil::BLANK;

			int len = mono_string_length(str);
			mono_unichar2* monoChars = mono_string_chars(str);

			String ret(len, '0');
			for(int i = 0; i < len; i++)
				ret[i] = (char)monoChars[i];

			return ret;
		}

		/**
		 * @brief	Converts a native wide string to a Mono (i.e. managed) string.
		 */
		static MonoString* wstringToMono(const WString& str)
		{
			if (sizeof(wchar_t) == 2) // Assuming UTF-16
				return mono_string_from_utf16((mono_unichar2*)str.c_str());
			else // Assuming UTF-32
			{
				const std::codecvt_mode convMode = (std::codecvt_mode)(std::little_endian);
				typedef std::codecvt_utf16<char32_t, 1114111, convMode> utf16utf32;

				std::wstring_convert<utf16utf32, char32_t> conversion("?");
				char32_t* start = (char32_t*)str.data();
				char32_t* end = (start + (str.size() - 1) / 4);

				mono_unichar2* convertedStr = (mono_unichar2*)conversion.to_bytes(start, end).c_str();
				return mono_string_from_utf16(convertedStr);
			}
		}

		/**
		 * @brief	Converts a native narrow string to a Mono (i.e. managed) string.
		 */
		static MonoString* stringToMono(const String& str)
		{
			return wstringToMono(toWString(str));
		}

		/**
		 * @brief	Outputs name and namespace for the type of the specified object.
		 */
		static void getClassName(MonoObject* obj, String& ns, String& typeName)
		{
			if (obj == nullptr)
				return;

			::MonoClass* monoClass = mono_object_get_class(obj);
			getClassName(monoClass, ns, typeName);
		}

		/**
		 * @brief	Outputs name and namespace for the specified type.
		 */
		static void getClassName(::MonoClass* monoClass, String& ns, String& typeName)
		{
			::MonoClass* nestingClass = mono_class_get_nesting_type(monoClass);

			if (nestingClass == nullptr)
			{
				ns = mono_class_get_namespace(monoClass);
				typeName = mono_class_get_name(monoClass);

				return;
			}
			else
			{
				typeName = String("+") + mono_class_get_name(monoClass);

				do 
				{
					::MonoClass* nextNestingClass = mono_class_get_nesting_type(nestingClass);
					if (nextNestingClass != nullptr)
					{
						typeName = String("+") + mono_class_get_name(nestingClass) + typeName;
						nestingClass = nextNestingClass;
					}
					else
					{
						ns = mono_class_get_namespace(nestingClass);
						typeName = mono_class_get_name(nestingClass) + typeName;

						break;
					}
				} while (true);
			}
		}

		/**
		 * @copydoc	throwIfException
		 */
		static void throwIfException(MonoException* exception)
		{
			throwIfException(reinterpret_cast<MonoObject*>(exception));
		}

		/**
		 * @brief	Throws a native exception if the provided object is a valid managed exception.
		 */
		static void throwIfException(MonoObject* exception)
		{
			if(exception != nullptr)
			{
				::MonoClass* exceptionClass = mono_object_get_class(exception);
				::MonoProperty* exceptionMsgProp = mono_class_get_property_from_name(exceptionClass, "Message");
				::MonoMethod* exceptionMsgGetter = mono_property_get_get_method(exceptionMsgProp);
				MonoString* exceptionMsg = (MonoString*)mono_runtime_invoke(exceptionMsgGetter, exception, nullptr, nullptr);

				::MonoProperty* exceptionStackProp = mono_class_get_property_from_name(exceptionClass, "StackTrace");
				::MonoMethod* exceptionStackGetter = mono_property_get_get_method(exceptionStackProp);
				MonoString* exceptionStackTrace = (MonoString*)mono_runtime_invoke(exceptionStackGetter, exception, nullptr, nullptr);

				String msg =  "Managed exception: " + toString(monoToWString(exceptionMsg)) + "\n" + toString(monoToWString(exceptionStackTrace));

				LOGERR(msg);
			}
		}

		template<class T, class... Args>
		static void invokeThunk(T* thunk, Args... args)
		{
			MonoException* exception = nullptr;
			thunk(std::forward<Args>(args)..., &exception);

			throwIfException(exception);
		}
	};
}