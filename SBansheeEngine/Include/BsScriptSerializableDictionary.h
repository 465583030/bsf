#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "BsScriptObject.h"

namespace BansheeEngine
{
	class BS_SCR_BE_EXPORT ScriptSerializableDictionary : public ScriptObject<ScriptSerializableDictionary>
	{
	public:
		SCRIPT_OBJ(ENGINE_ASSEMBLY, "BansheeEngine", "SerializableDictionary")

		static ScriptSerializableDictionary* create(const ScriptSerializableProperty* parentProperty);

	private:
		static MonoObject* internal_createKeyProperty(ScriptSerializableDictionary* nativeInstance);
		static MonoObject* internal_createValueProperty(ScriptSerializableDictionary* nativeInstance);

		ScriptSerializableDictionary(MonoObject* instance, const ManagedSerializableTypeInfoDictionaryPtr& typeInfo);

		ManagedSerializableTypeInfoDictionaryPtr mTypeInfo;
	};
}