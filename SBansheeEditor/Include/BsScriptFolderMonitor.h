#pragma once

#include "BsScriptEditorPrerequisites.h"
#include "BsScriptObject.h"
#include "BsFolderMonitor.h"
#include "BsModule.h"

namespace BansheeEngine
{
	/**
	 * @brief	Interop class between C++ & CLR for FolderMonitor.
	 */
	class BS_SCR_BED_EXPORT ScriptFolderMonitor : public ScriptObject <ScriptFolderMonitor>
	{
	public:
		SCRIPT_OBJ(EDITOR_ASSEMBLY, "BansheeEditor", "FolderMonitor")

	private:
		friend class ScriptFolderMonitorManager;

		ScriptFolderMonitor(MonoObject* instance, FolderMonitor* monitor);
		~ScriptFolderMonitor();

		/**
		 * @brief	Updates the native folder monitor. Must be called once per frame.
		 */
		void update();

		/**
		 * @brief	Destroys the native folder monitor.
		 */
		void destroy();

		/**
		 * @brief	Triggered when the native folder monitor detects a file has been modified.
		 */
		void onMonitorFileModified(const Path& path);

		/**
		 * @brief	Triggered when the native folder monitor detects a file has been added.
		 */
		void onMonitorFileAdded(const Path& path);

		/**
		 * @brief	Triggered when the native folder monitor detects a file has been removed.
		 */
		void onMonitorFileRemoved(const Path& path);

		/**
		 * @brief	Triggered when the native folder monitor detects a file has been renamed.
		 */
		void onMonitorFileRenamed(const Path& from, const Path& to);

		FolderMonitor* mMonitor;

		/************************************************************************/
		/* 								CLR HOOKS						   		*/
		/************************************************************************/
		static void internal_CreateInstance(MonoObject* instance, MonoString* folder);
		static void internal_Destroy(ScriptFolderMonitor* thisPtr);

		typedef void(__stdcall *OnModifiedThunkDef) (MonoObject*, MonoString*, MonoException**);
		typedef void(__stdcall *OnRenamedThunkDef) (MonoObject*, MonoString*, MonoString*, MonoException**);

		static OnModifiedThunkDef OnModifiedThunk;
		static OnModifiedThunkDef OnAddedThunk;
		static OnModifiedThunkDef OnRemovedThunk;
		static OnRenamedThunkDef OnRenamedThunk;
	};

	/**
	 * @brief	Manages all active managed folder monitor objects.
	 */
	class BS_SCR_BED_EXPORT ScriptFolderMonitorManager : public Module<ScriptFolderMonitorManager>
	{
	public:
		/**
		 * @brief	Triggers updates on all active folder monitor objects. Should be called
		 *			once per frame.
		 */
		void update();

	private:
		friend class ScriptFolderMonitor;

		/**
		 * @brief	Registers a new managed folder monitor.
		 */
		void _registerMonitor(ScriptFolderMonitor* monitor);

		/**
		 * @brief	Unregisters a destroyed managed folder monitor.
		 */
		void _unregisterMonitor(ScriptFolderMonitor* monitor);

		UnorderedSet<ScriptFolderMonitor*> mMonitors;
	};
}