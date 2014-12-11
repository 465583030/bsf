#pragma once

#include "BsScriptEditorPrerequisites.h"
#include "BsScriptObject.h"
#include "BsEditorWidget.h"
#include "BsVector2I.h"

namespace BansheeEngine
{
	class ScriptEditorWidget;

	class BS_SCR_BED_EXPORT ScriptEditorWindow : public ScriptObject<ScriptEditorWindow>
	{
		struct EditorWindowHandle
		{
			uint32_t gcHandle;
			ScriptEditorWindow* nativeObj;
		};
	public:
		SCRIPT_OBJ(BansheeEditorAssemblyName, "BansheeEditor", "EditorWindow")

		~ScriptEditorWindow();

		EditorWidgetBase* getEditorWidget() const { return mEditorWidget; }

		static void registerManagedEditorWindows();
	private:
		friend class ScriptEditorWidget;

		ScriptEditorWindow(MonoObject* instance, const String& windowName, const String& displayName, EditorWidgetBase* editorWidget);

		static MonoObject* internal_createOrGetInstance(MonoString* ns, MonoString* typeName);

		static bool internal_hasFocus(ScriptEditorWindow* thisPtr);
		static void internal_screenToWindowPos(ScriptEditorWindow* thisPtr, Vector2I screenPos, Vector2I* windowPos);
		static void internal_windowToScreenPos(ScriptEditorWindow* thisPtr, Vector2I windowPos, Vector2I* screenPos);
		static UINT32 internal_getWidth(ScriptEditorWindow* thisPtr);
		static UINT32 internal_getHeight(ScriptEditorWindow* thisPtr);

		static void internal_initializeGUIPanel(ScriptEditorWindow* thisPtr, MonoObject* panel);

		void onWidgetMoved(INT32 x, INT32 y);
		void onWidgetResized(UINT32 width, UINT32 height);
		void onWidgetParentChanged(EditorWidgetContainer* newParent);
		void onFocusChanged(bool inFocus);

		void _onManagedInstanceDeleted();

		String mName;
		EditorWidgetBase* mEditorWidget;
		Vector<ScriptGUIPanel*> mPanels;
		HEvent mOnWidgetMovedConn;
		HEvent mOnWidgetResizedConn;
		HEvent mOnParentChangedConn;
		HEvent mOnFocusChangedConn;

		static MonoMethod* onResizedMethod;
		static MonoMethod* onFocusChangedMethod;

		// Global editor window management methods
		static void registerScriptEditorWindow(ScriptEditorWindow* editorWindow);
		static void unregisterScriptEditorWindow(const String& windowTypeName);

		static UnorderedMap<String, EditorWindowHandle> OpenScriptEditorWindows;

		static EditorWidgetBase* openEditorWidgetCallback(String ns, String type, EditorWidgetContainer& parentContainer);
	};

	class BS_SCR_BED_EXPORT ScriptEditorWidget : public EditorWidgetBase
	{
	public:
		ScriptEditorWidget(const String& windowTypeName, const HString& displayName, 
			EditorWidgetContainer& parentContainer, MonoClass* monoClass, MonoObject* managedInstance);
		~ScriptEditorWidget();

		void update();

	private:
		typedef void(__stdcall *UpdateThunkDef) (MonoObject*, MonoException**);

		String mWindowTypeName;
		UpdateThunkDef mUpdateThunk;
		MonoObject* mManagedInstance;
		ScriptEditorWindow* mScriptParent;
	};
}