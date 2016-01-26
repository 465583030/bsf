//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsPrerequisites.h"
#include "BsModule.h"

namespace BansheeEngine 
{
	/**
	 * @brief	Abstraction that handles a specific set of script libraries.
	 */
	class BS_EXPORT ScriptLibrary
	{
	public:
		virtual ~ScriptLibrary() { }

		/**
		 * @brief	Called when the script system is being activated.
		 */
		virtual void initialize() = 0;

		/**
		 * @brief	Called when the script libraries should be reloaded.
		 *			(e.g. when they are recompiled).
		 */
		virtual void reload() = 0;

		/**
		 * @brief	Called when the script system is being destroyed.
		 */
		virtual void destroy() = 0;
	};

	/**
	 * @brief	Handles initialization of a scripting system.
	 */
	class BS_EXPORT ScriptManager : public Module<ScriptManager>
	{
	public:
		ScriptManager() { }
		~ScriptManager() { }

		/**
		 * @brief	Initializes the currently active script library loading the scripts
		 *			contained within.
		 */
		void initialize();

		/**
		 * @brief	Reloads any scripts in the currently active library. Should be called after
		 *			some change to the scripts was made (e.g. project was changed, or scripts were
		 *			recompiled).
		 */
		void reload();

		/**
		 * @brief	Sets the active script library that controls what kind and which
		 *			scripts are loaded.
		 */
		void _setScriptLibrary(const SPtr<ScriptLibrary>& library);

	private:
		/**
		 * @copydoc	ScriptManager::onShutDown
		 */
		void onShutDown() override;

		SPtr<ScriptLibrary> mScriptLibrary;
	};
}