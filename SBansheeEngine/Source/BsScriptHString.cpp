#include "BsScriptHString.h"
#include "BsScriptMeta.h"
#include "BsMonoField.h"
#include "BsMonoClass.h"
#include "BsMonoManager.h"
#include "BsMonoUtil.h"

namespace BansheeEngine
{
	ScriptHString::ScriptHString(MonoObject* instance, const HString& string)
		:ScriptObject(instance), mString(string)
	{ }

	void ScriptHString::initRuntimeData()
	{
		metaData.scriptClass->addInternalCall("Internal_CreateInstance", &ScriptHString::internal_createInstance);
		metaData.scriptClass->addInternalCall("Internal_SetParameter", &ScriptHString::internal_setParameter);
		metaData.scriptClass->addInternalCall("Internal_GetValue", &ScriptHString::internal_getValue);
	}

	void ScriptHString::internal_createInstance(MonoObject* instance, MonoString* identifier)
	{
		HString string(MonoUtil::monoToWString(identifier));
		
		ScriptHString* nativeInstance = new (bs_alloc<ScriptHString>()) ScriptHString(instance, string);
	}

	void ScriptHString::internal_setParameter(HString* nativeInstance, UINT32 idx, MonoString* value)
	{
		nativeInstance->setParameter(idx, MonoUtil::monoToWString(value));
	}

	void ScriptHString::internal_getValue(HString* nativeInstance, MonoString** value)
	{
		*value = MonoUtil::wstringToMono(MonoManager::instance().getDomain(), nativeInstance->getValue());
	}
}