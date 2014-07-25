#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "BsScriptResource.h"
#include "BsScriptObject.h"

namespace BansheeEngine
{
	class BS_SCR_BE_EXPORT ScriptManagedResource : public ScriptObject<ScriptManagedResource, ScriptResourceBase>
	{
	public:
		SCRIPT_OBJ(BansheeEngineAssemblyName, "BansheeEngine", "ManagedResource")

		ScriptManagedResource(MonoObject* instance, const HManagedResource& resource);

		void* getNativeRaw() const { return mResource.get(); }

		HResource getNativeHandle() const { return mResource; }
		void setNativeHandle(const HResource& resource);
	private:
		friend class ScriptResourceManager;

		static void internal_createInstance(MonoObject* instance);

		void _onManagedInstanceDeleted();

		HManagedResource mResource;
	};
}