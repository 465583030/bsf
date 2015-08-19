#include "BsScriptResources.h"
#include "BsMonoManager.h"
#include "BsMonoClass.h"
#include "BsMonoMethod.h"
#include "BsMonoUtil.h"
#include "BsResources.h"
#include "BsGameResourceManager.h"
#include "BsScriptResourceManager.h"
#include "BsScriptResource.h"

namespace BansheeEngine
{
	ScriptResources::ScriptResources(MonoObject* instance)
		:ScriptObject(instance)
	{ }

	void ScriptResources::initRuntimeData()
	{
		metaData.scriptClass->addInternalCall("Internal_Load", &ScriptResources::internal_Load);
		metaData.scriptClass->addInternalCall("Internal_Unload", &ScriptResources::internal_Unload);
		metaData.scriptClass->addInternalCall("Internal_UnloadUnused", &ScriptResources::internal_UnloadUnused);
	}

	MonoObject* ScriptResources::internal_Load(MonoString* path)
	{
		Path nativePath = MonoUtil::monoToWString(path);

		HResource resource = GameResourceManager::instance().load(nativePath);
		if (resource == nullptr)
			return nullptr;

		ScriptResourceBase* scriptResource;
		ScriptResourceManager::instance().getScriptResource(resource, &scriptResource, true);

		return scriptResource->getManagedInstance();
	}

	void ScriptResources::internal_Unload(ScriptResourceBase* resourcePtr)
	{
		gResources().unload(resourcePtr->getNativeHandle());
	}

	void ScriptResources::internal_UnloadUnused()
	{
		gResources().unloadAllUnused();
	}
}