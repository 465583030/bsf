#include "BsScriptUndoRedo.h"
#include "BsScriptMeta.h"
#include "BsMonoClass.h"
#include "BsMonoMethod.h"
#include "BsScriptSceneObject.h"
#include "BsMonoUtil.h"
#include "BsScriptGameObjectManager.h"
#include "BsUndoRedo.h"
#include "BsCmdRecordSO.h"
#include "BsCmdCloneSO.h"
#include "BsCmdCreateSO.h"
#include "BsCmdDeleteSO.h"
#include "BsCmdInstantiateSO.h"
#include "BsCmdReparentSO.h"
#include "BsScriptPrefab.h"
#include "BsPrefab.h"

namespace BansheeEngine
{
	ScriptUndoRedo::ScriptUndoRedo(MonoObject* instance)
		:ScriptObject(instance)
	{

	}

	void ScriptUndoRedo::initRuntimeData()
	{
		metaData.scriptClass->addInternalCall("Internal_Undo", &ScriptUndoRedo::internal_Undo);
		metaData.scriptClass->addInternalCall("Internal_Redo", &ScriptUndoRedo::internal_Redo);
		metaData.scriptClass->addInternalCall("Internal_PushGroup", &ScriptUndoRedo::internal_PushGroup);
		metaData.scriptClass->addInternalCall("Internal_PopGroup", &ScriptUndoRedo::internal_PopGroup);
		metaData.scriptClass->addInternalCall("Internal_RecordSO", &ScriptUndoRedo::internal_RecordSO);
		metaData.scriptClass->addInternalCall("Internal_CloneSO", &ScriptUndoRedo::internal_CloneSO);
		metaData.scriptClass->addInternalCall("Internal_CloneSOMulti", &ScriptUndoRedo::internal_CloneSOMulti);
		metaData.scriptClass->addInternalCall("Internal_Instantiate", &ScriptUndoRedo::internal_Instantiate);
		metaData.scriptClass->addInternalCall("Internal_CreateSO", &ScriptUndoRedo::internal_CreateSO);
		metaData.scriptClass->addInternalCall("Internal_DeleteSO", &ScriptUndoRedo::internal_DeleteSO);
		metaData.scriptClass->addInternalCall("Internal_ReparentSO", &ScriptUndoRedo::internal_ReparentSO);
		metaData.scriptClass->addInternalCall("Internal_ReparentSOMulti", &ScriptUndoRedo::internal_ReparentSOMulti);
	}

	void ScriptUndoRedo::internal_Undo()
	{
		UndoRedo::instance().undo();
	}

	void ScriptUndoRedo::internal_Redo()
	{
		UndoRedo::instance().redo();
	}

	void ScriptUndoRedo::internal_PushGroup(MonoString* name)
	{
		String nativeName = MonoUtil::monoToString(name);
		UndoRedo::instance().pushGroup(nativeName);
	}

	void ScriptUndoRedo::internal_PopGroup(MonoString* name)
	{
		String nativeName = MonoUtil::monoToString(name);
		UndoRedo::instance().popGroup(nativeName);
	}

	void ScriptUndoRedo::internal_RecordSO(ScriptSceneObject* soPtr, MonoString* description)
	{
		WString nativeDescription = MonoUtil::monoToWString(description);
		CmdRecordSO::execute(soPtr->getNativeSceneObject(), nativeDescription);
	}

	MonoObject* ScriptUndoRedo::internal_CloneSO(ScriptSceneObject* soPtr, MonoString* description)
	{
		WString nativeDescription = MonoUtil::monoToWString(description);
		HSceneObject clone = CmdCloneSO::execute(soPtr->getNativeSceneObject(), nativeDescription);

		return ScriptGameObjectManager::instance().createScriptSceneObject(clone)->getManagedInstance();
	}

	MonoArray* ScriptUndoRedo::internal_CloneSOMulti(MonoArray* soPtrs, MonoString* description)
	{
		WString nativeDescription = MonoUtil::monoToWString(description);

		ScriptArray input(soPtrs);
		ScriptArray output = ScriptArray::create<ScriptSceneObject>(input.size());

		for (UINT32 i = 0; i < input.size(); i++)
		{
			ScriptSceneObject* soPtr = input.get<ScriptSceneObject*>(i);
			HSceneObject clone = CmdCloneSO::execute(soPtr->getNativeSceneObject(), nativeDescription);
			ScriptSceneObject* cloneSoPtr = ScriptGameObjectManager::instance().createScriptSceneObject(clone);

			output.set(i, cloneSoPtr->getManagedInstance());
		}

		return output.getInternal();
	}

	MonoObject* ScriptUndoRedo::internal_Instantiate(ScriptPrefab* prefabPtr, MonoString* description)
	{
		HPrefab prefab = prefabPtr->getHandle();
		if (!prefab.isLoaded())
			return nullptr;

		WString nativeDescription = MonoUtil::monoToWString(description);
		HSceneObject clone = CmdInstantiateSO::execute(prefab, nativeDescription);

		return ScriptGameObjectManager::instance().createScriptSceneObject(clone)->getManagedInstance();
	}

	MonoObject* ScriptUndoRedo::internal_CreateSO(MonoString* name, MonoString* description)
	{
		String nativeName = MonoUtil::monoToString(name);
		WString nativeDescription = MonoUtil::monoToWString(description);
		HSceneObject newObj = CmdCreateSO::execute(nativeName, 0, nativeDescription);

		return ScriptGameObjectManager::instance().createScriptSceneObject(newObj)->getManagedInstance();
	}

	void ScriptUndoRedo::internal_DeleteSO(ScriptSceneObject* soPtr, MonoString* description)
	{
		WString nativeDescription = MonoUtil::monoToWString(description);
		CmdDeleteSO::execute(soPtr->getNativeSceneObject(), nativeDescription);
	}

	void ScriptUndoRedo::internal_ReparentSO(ScriptSceneObject* soPtr, ScriptSceneObject* parentSOPtr, MonoString* description)
	{
		HSceneObject parent;
		if (parentSOPtr != nullptr)
			parent = parentSOPtr->getNativeSceneObject();

		WString nativeDescription = MonoUtil::monoToWString(description);

		HSceneObject so = soPtr->getNativeSceneObject();
		CmdReparentSO::execute(so, parent, nativeDescription);
	}

	void ScriptUndoRedo::internal_ReparentSOMulti(MonoArray* soPtrs, ScriptSceneObject* parentSOPtr, MonoString* description)
	{
		HSceneObject parent;
		if (parentSOPtr != nullptr)
			parent = parentSOPtr->getNativeSceneObject();

		WString nativeDescription = MonoUtil::monoToWString(description);

		ScriptArray input(soPtrs);
		for (UINT32 i = 0; i < input.size(); i++)
		{
			ScriptSceneObject* soPtr = input.get<ScriptSceneObject*>(i);
			HSceneObject so = soPtr->getNativeSceneObject();
			CmdReparentSO::execute(so, parent, nativeDescription);
		}
	}
}