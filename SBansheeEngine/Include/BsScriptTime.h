#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "BsScriptObject.h"

namespace BansheeEngine
{
	class BS_SCR_BE_EXPORT ScriptTime : public ScriptObject <ScriptTime>
	{
	public:
		SCRIPT_OBJ(ENGINE_ASSEMBLY, "BansheeEngine", "Time")

	private:
		static float internal_getElapsed();
		static float internal_getFrameDelta();
		static UINT64 internal_getFrameNumber();
		static UINT64 internal_getPrecise();

		ScriptTime(MonoObject* instance);
	};
}