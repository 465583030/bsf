#pragma once

#include "BsScriptEditorPrerequisites.h"
#include "BsScriptObject.h"
#include "BsVector2I.h"

namespace BansheeEngine
{
	/** Interop class between C++ & CLR for SceneHandles. */
	class BS_SCR_BED_EXPORT ScriptSceneHandles : public ScriptObject <ScriptSceneHandles>
	{
	public:
		SCRIPT_OBJ(EDITOR_ASSEMBLY, "BansheeEditor", "SceneHandles")

	private:
		ScriptSceneHandles(MonoObject* object, EditorWidgetBase* parentWidget, const SPtr<Camera>& camera);
		~ScriptSceneHandles();

		/**
		 * @brief	Checks is the pointer currently within the provided window, and if it is not
		 *			the cursor is wrapped in such a way so that it is returned to within the window bounds.
		 *		
		 * @return	How far was the cursor moved due to wrapping. This will be (0, 0) if the cursor is within
		 *			window bounds initially.
		 */
		Vector2I wrapCursorToWindow() const;

		EditorWidgetBase* mParentWidget;
		SPtr<Camera> mCamera;
		Vector2I mMouseDeltaCompensate;

		/************************************************************************/
		/* 								CLR HOOKS						   		*/
		/************************************************************************/
		static void internal_Create(MonoObject* managedInstance, ScriptEditorWindow* parentWindow, ScriptCamera* camera);
		static void internal_Draw(ScriptSceneHandles* thisPtr);
		static void internal_UpdateInput(ScriptSceneHandles* thisPtr, Vector2I* inputPos, Vector2I* inputDelta);
		static void internal_TrySelect(ScriptSceneHandles* thisPtr, Vector2I* inputPos);
		static bool internal_IsActive(ScriptSceneHandles* thisPtr);
		static void internal_ClearSelection(ScriptSceneHandles* thisPtr);
	};
}