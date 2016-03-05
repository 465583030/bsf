//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "BsScriptResource.h"
#include "BsPrefab.h"

namespace BansheeEngine
{
	/**
	 * @brief	Interop class between C++ & CLR for Prefab.
	 */
	class BS_SCR_BE_EXPORT ScriptPrefab : public TScriptResource<ScriptPrefab, Prefab>
	{
	public:
		SCRIPT_OBJ(ENGINE_ASSEMBLY, "BansheeEngine", "Prefab")

	private:
		friend class ScriptResourceManager;

		ScriptPrefab(MonoObject* instance, const HPrefab& prefab);

		/**
		 * @brief	Creates an empty, uninitialized managed instance of the resource interop object.
		 */
		static MonoObject* createInstance();

		/************************************************************************/
		/* 								CLR HOOKS						   		*/
		/************************************************************************/
		static void internal_CreateInstance(MonoObject* instance, ScriptSceneObject* so);
		static MonoObject* internal_Instantiate(ScriptPrefab* instance);
	};
}