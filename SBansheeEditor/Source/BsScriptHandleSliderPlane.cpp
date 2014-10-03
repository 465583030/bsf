#include "BsScriptHandleSliderPlane.h"
#include "BsScriptMeta.h"
#include "BsMonoField.h"
#include "BsMonoClass.h"
#include "BsMonoManager.h"

namespace BansheeEngine
{
	ScriptHandleSliderPlane::ScriptHandleSliderPlane(MonoObject* instance, const Vector3& dir1, const Vector3& dir2, float length, bool fixedScale, float snapValue)
		:ScriptObject(instance), mSlider(nullptr)
	{
		mSlider = bs_new<HandleSliderPlane>(dir1, dir2, length, fixedScale, snapValue);
	}

	ScriptHandleSliderPlane::~ScriptHandleSliderPlane()
	{
		bs_delete(mSlider);
	}

	void ScriptHandleSliderPlane::initRuntimeData()
	{
		metaData.scriptClass->addInternalCall("internal_CreateInstance", &ScriptHandleSliderPlane::internal_CreateInstance);
		metaData.scriptClass->addInternalCall("internal_GetDelta", &ScriptHandleSliderPlane::internal_GetDelta);
		metaData.scriptClass->addInternalCall("internal_GetDeltaDirection", &ScriptHandleSliderPlane::internal_GetDeltaDirection);
		metaData.scriptClass->addInternalCall("internal_GetNewPosition", &ScriptHandleSliderPlane::internal_GetNewPosition);
	}

	void ScriptHandleSliderPlane::internal_CreateInstance(MonoObject* instance, Vector3 dir1, Vector3 dir2, float length, bool fixedScale, float snapValue)
	{
		ScriptHandleSliderPlane* nativeInstance = new (bs_alloc<ScriptHandleSliderPlane>())
			ScriptHandleSliderPlane(instance, dir1, dir2, length, fixedScale, snapValue);
	}

	void ScriptHandleSliderPlane::internal_GetDelta(ScriptHandleSliderPlane* nativeInstance, float* value)
	{
		*value = nativeInstance->mSlider->getDelta();
	}

	void ScriptHandleSliderPlane::internal_GetDeltaDirection(ScriptHandleSliderPlane* nativeInstance, Vector3* value)
	{
		*value = nativeInstance->mSlider->getDeltaDirection();
	}

	void ScriptHandleSliderPlane::internal_GetNewPosition(ScriptHandleSliderPlane* nativeInstance, Vector3* value)
	{
		*value = nativeInstance->mSlider->getNewPosition();
	}
}