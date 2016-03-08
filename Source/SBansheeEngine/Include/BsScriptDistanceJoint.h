//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "BsScriptCollider.h"
#include "BsScriptJoint.h"

namespace BansheeEngine
{
	/** @addtogroup ScriptInteropEngine
	 *  @{
	 */

	/** Interop class between C++ & CLR for DistanceJoint. */
	class BS_SCR_BE_EXPORT ScriptDistanceJoint : public TScriptJoint<ScriptDistanceJoint>
	{
	public:
		SCRIPT_OBJ(ENGINE_ASSEMBLY, "BansheeEngine", "NativeDistanceJoint")
	private:
		ScriptDistanceJoint(MonoObject* instance, const SPtr<Joint>& joint);

		/** Returns the native distance joint wrapped by this object. */
		DistanceJoint* getDistanceJoint() const;

		/************************************************************************/
		/* 								CLR HOOKS						   		*/
		/************************************************************************/
		static void internal_CreateInstance(MonoObject* instance);
		static float internal_GetDistance(ScriptDistanceJoint* thisPtr);
		static void internal_SetMinDistance(ScriptDistanceJoint* thisPtr, float value);
		static void internal_SetMaxDistance(ScriptDistanceJoint* thisPtr, float value);
		static void internal_SetTolerance(ScriptDistanceJoint* thisPtr, float value);
		static void internal_SetSpring(ScriptDistanceJoint* thisPtr, Spring* value);
		static void internal_SetEnableMinDistanceLimit(ScriptDistanceJoint* thisPtr, bool value);
		static void internal_SetEnableMaxDistanceLimit(ScriptDistanceJoint* thisPtr, bool value);
		static void internal_SetEnableSpring(ScriptDistanceJoint* thisPtr, bool value);
	};

	/** @} */
}