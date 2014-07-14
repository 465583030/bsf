#include "BsScriptGUIFixedSpace.h"
#include "BsScriptMeta.h"
#include "BsMonoField.h"
#include "BsMonoClass.h"
#include "BsMonoManager.h"
#include "BsSpriteTexture.h"
#include "BsMonoUtil.h"
#include "BsGUILayout.h"
#include "BsGUISpace.h"
#include "BsScriptGUILayout.h"

namespace BansheeEngine
{
	ScriptGUIFixedSpace::ScriptGUIFixedSpace(MonoObject* instance, GUIFixedSpace& fixedSpace, GUILayout* parentLayout)
		:TScriptGUIElementBase(instance, &fixedSpace), mFixedSpace(fixedSpace), mParentLayout(parentLayout), mIsDestroyed(false)
	{

	}

	void ScriptGUIFixedSpace::initRuntimeData()
	{
		metaData.scriptClass->addInternalCall("Internal_CreateInstance", &ScriptGUIFixedSpace::internal_createInstance);
		metaData.scriptClass->addInternalCall("Internal_SetSize", &ScriptGUIFixedSpace::internal_setSize);
	}

	void ScriptGUIFixedSpace::destroy()
	{
		if (!mIsDestroyed)
		{
			mParentLayout->removeSpace(mFixedSpace);
			mParentLayout = nullptr;

			mIsDestroyed = true;
		}
	}

	void ScriptGUIFixedSpace::setParent(GUILayout* parentLayout)
	{
		// FixedSpace parent is static, so do nothing
	}

	void ScriptGUIFixedSpace::internal_createInstance(MonoObject* instance, MonoObject* parentLayout, UINT32 size)
	{
		ScriptGUILayout* scriptLayout = ScriptGUILayout::toNative(parentLayout);
		GUILayout* nativeLayout = scriptLayout->getInternalValue();
		GUIFixedSpace& space = nativeLayout->addSpace(size);

		ScriptGUIFixedSpace* nativeInstance = new (bs_alloc<ScriptGUIFixedSpace>()) ScriptGUIFixedSpace(instance, space, nativeLayout);
	}

	void ScriptGUIFixedSpace::internal_setSize(ScriptGUIFixedSpace* nativeInstance, UINT32 size)
	{
		nativeInstance->mFixedSpace.setSize(size);
	}
}