#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "BsScriptObject.h"
#include "BsVector3.h"
#include "BsQuaternion.h"
#include "BsDegree.h"
#include "BsColor.h"
#include "BsLight.h"

namespace BansheeEngine
{
	/**
	 * @brief	Interop class between C++ & CLR for Light.
	 */
	class BS_SCR_BE_EXPORT ScriptLight : public ScriptObject <ScriptLight>
	{
	public:
		SCRIPT_OBJ(ENGINE_ASSEMBLY, "BansheeEngine", "NativeLight")

		/**
		 * @brief	Gets the wrapped native LightInternal object.
		 */
		SPtr<Light> getInternal() const { return mLight; }

	private:
		ScriptLight(MonoObject* managedInstance, const HSceneObject& parentSO);
		~ScriptLight();

		SPtr<Light> mLight;
		UINT32 mLastUpdateHash;

		/************************************************************************/
		/* 								CLR HOOKS						   		*/
		/************************************************************************/
		static void internal_create(MonoObject* managedInstance, ScriptSceneObject* parentSO);

		static Vector3 internal_getPosition(ScriptLight* thisPtr);
		static void internal_setPosition(ScriptLight* thisPtr, Vector3 position);

		static Quaternion internal_getRotation(ScriptLight* thisPtr);
		static void internal_setRotation(ScriptLight* thisPtr, Quaternion rotation);

		static LightType internal_getType(ScriptLight* thisPtr);
		static void internal_setType(ScriptLight* thisPtr, LightType type);

		static bool internal_getCastsShadow(ScriptLight* thisPtr);
		static void internal_setCastsShadow(ScriptLight* thisPtr, bool castsShadow);

		static Color internal_getColor(ScriptLight* thisPtr);
		static void internal_setColor(ScriptLight* thisPtr, Color color);

		static float internal_getRange(ScriptLight* thisPtr);
		static void internal_setRange(ScriptLight* thisPtr, float range);

		static float internal_getIntensity(ScriptLight* thisPtr);
		static void internal_setIntensity(ScriptLight* thisPtr, float intensity);

		static Degree internal_getSpotAngle(ScriptLight* thisPtr);
		static void internal_setSpotAngle(ScriptLight* thisPtr, Degree spotAngle);

		static Degree internal_getSpotFalloffAngle(ScriptLight* thisPtr);
		static void internal_setSpotFalloffAngle(ScriptLight* thisPtr, Degree spotFalloffAngle);

		static Sphere internal_getBounds(ScriptLight* thisPtr);

		static void internal_updateTransform(ScriptLight* thisPtr, ScriptSceneObject* parent);
		static void internal_onDestroy(ScriptLight* instance);
	};
}