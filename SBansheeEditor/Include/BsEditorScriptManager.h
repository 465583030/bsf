#pragma once

#include "BsScriptEditorPrerequisites.h"
#include "BsModule.h"

namespace BansheeEngine
{
	class BS_SCR_BED_EXPORT EditorScriptManager : public Module<EditorScriptManager>
	{
	public:
		EditorScriptManager();
		~EditorScriptManager();

		void update();

	private:
		void triggerOnInitialize();
		void onAssemblyRefreshDone();
		void loadMonoTypes();
		static void debug_refreshAssembly();

		static const float EDITOR_UPDATE_RATE;

		MonoAssembly* mEditorAssembly;
		float mLastUpdateTime;

		MonoClass* mProgramEdClass;
		MonoMethod* mUpdateMethod;

		HEvent mOnDomainLoadConn;
		HEvent mOnAssemblyRefreshDoneConn;

		static bool mDebugRefresh;
	};
}