#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "BsScriptResource.h"
#include "BsScriptObject.h"
#include "BsFont.h"

namespace BansheeEngine
{
	class BS_SCR_BE_EXPORT ScriptFont : public ScriptObject<ScriptFont, ScriptResourceBase>
	{
	public:
		SCRIPT_OBJ(BansheeEngineAssemblyName, "BansheeEngine", "Font")

		void* getNativeRaw() const { return mFont.get(); }

		HResource getNativeHandle() const { return mFont; }
		void setNativeHandle(const HResource& resource);
	private:
		friend class ScriptResourceManager;

		ScriptFont(MonoObject* instance, const HFont& font);

		void _onManagedInstanceDeleted();

		HFont mFont;
	};
}