#pragma once

#include "BsScriptEditorPrerequisites.h"
#include "BsScriptObject.h"
#include "BsModalWindow.h"
#include "BsVector2I.h"

namespace BansheeEngine
{
	class ManagedModalWindow;

	/**
	 * @brief	Interop class between C++ & CLR for ManagedModalWindow.
	 */
	class BS_SCR_BED_EXPORT ScriptModalWindow : public ScriptObject <ScriptModalWindow>
	{
		/**
		 * @brief	Contains data about the managed handle to a modal window.
		 */
		struct ModalWindowHandle
		{
			uint32_t gcHandle;
			ManagedModalWindow* nativeObj;
		};
	public:
		SCRIPT_OBJ(EDITOR_ASSEMBLY, "BansheeEditor", "ModalWindow")

		~ScriptModalWindow();

	private:
		friend class ManagedModalWindow;

		ScriptModalWindow(ManagedModalWindow* editorWidget);

		/**
		 * @brief	Triggered when assembly refresh has started.
		 */
		void onAssemblyRefreshStarted();

		/**
		 * @brief	Triggered when the native modal window is closed.
		 */
		void notifyWindowDestroyed();

		ManagedModalWindow* mModalWindow;
		HEvent mOnAssemblyRefreshStartedConn;

		static MonoField* guiPanelField;

		/************************************************************************/
		/* 								CLR HOOKS						   		*/
		/************************************************************************/
		static void internal_createInstance(MonoObject* instance, bool allowCloseButton);
		static void internal_close(ScriptModalWindow* thisPtr);
		static UINT32 internal_getWidth(ScriptModalWindow* thisPtr);
		static UINT32 internal_getHeight(ScriptModalWindow* thisPtr);
		static void internal_setWidth(ScriptModalWindow* thisPtr, UINT32 value);
		static void internal_setHeight(ScriptModalWindow* thisPtr, UINT32 value);
		static void internal_setTitle(ScriptModalWindow* thisPtr, MonoObject* title);
		static void internal_screenToWindowPos(ScriptModalWindow* thisPtr, Vector2I* screenPos, Vector2I* windowPos);
		static void internal_windowToScreenPos(ScriptModalWindow* thisPtr, Vector2I* windowPos, Vector2I* screenPos);
	};

	/**
	 * @brief	Modal window implementation that handles managed modal window implementations.
	 *			Each implementation is wrapped in this object and then managed by its parent interop
	 *			object of ScriptModalWindow type.
	 */
	class BS_SCR_BED_EXPORT ManagedModalWindow : public ModalWindow
	{
	public:
		ManagedModalWindow(bool allowCloseButton, MonoObject* managedInstance);
		~ManagedModalWindow();

		/**
		 * @brief	Attempts to create a managed instance for the modal window described by the
		 *			type provided upon construction.
		 *
		 * @return	True if the managed instance was created.
		 */
		bool createManagedInstance();

		/**
		 * @brief	Releases the internally held handle to the managed instance. This will cause
		 *			managed instance to be destroyed if no other references are being held.
		 */
		void releaseManagedInstance();

		/**
		 * @brief	Sets the parent interop object that handles part of the communication
		 *			between this object and the managed instance.
		 */
		void setParent(ScriptModalWindow* parent);

		/**
		 * @copydoc	ModalWindow::update 
		 */
		void update() override;
		
		/**
		 * @brief	Loads all required mono methods, fields and types required
		 *			for operation of this object. Must be called after construction
		 *			and after assembly refresh.
		 *
		 * @param	windowClass	Mono class to load the types from.
		 */
		void reloadMonoTypes(MonoClass* windowClass);

		/**
		 * @brief	Triggers OnInitialize callbacks on the managed instance.
		 */
		void triggerOnInitialize();

		/**
		 * @brief	Triggers OnDestroy callbacks on the managed instance.
		 */
		void triggerOnDestroy();

		/**
		 * @brief	Returns the managed instance for the modal window
		 *			represented by this object.
		 */
		MonoObject* getManagedInstance() const { return mManagedInstance; }
	protected:
		/**
		 * @copydoc	ModalWindow::resized
		 */
		virtual void resized() override;

		/**
		 * @copydoc	ModalWindow::close
		 */
		virtual void close() override;

	private:
		friend class ScriptModalWindow;

		typedef void(__stdcall *OnInitializeThunkDef) (MonoObject*, MonoException**);
		typedef void(__stdcall *OnDestroyThunkDef) (MonoObject*, MonoException**);
		typedef void(__stdcall *UpdateThunkDef) (MonoObject*, MonoException**);

		String mNamespace;
		String mTypename;

		OnInitializeThunkDef mOnInitializeThunk;
		OnDestroyThunkDef mOnDestroyThunk;
		UpdateThunkDef mUpdateThunk;
		MonoMethod* mOnWindowResizedMethod;

		bool mIsInitialized;
		MonoObject* mManagedInstance;
		uint32_t mGCHandle;

		ScriptModalWindow* mScriptParent;
		ScriptGUILayout* mContentsPanel;
	};
}