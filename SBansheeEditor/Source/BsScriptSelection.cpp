#include "BsScriptSelection.h"
#include "BsScriptMeta.h"
#include "BsMonoClass.h"
#include "BsSelection.h"
#include "BsScriptSceneObject.h"
#include "BsMonoUtil.h"
#include "BsScriptGameObjectManager.h"

namespace BansheeEngine
{
	void ScriptSelection::initRuntimeData()
	{
		metaData.scriptClass->addInternalCall("Internal_GetSceneObjectSelection", &ScriptSelection::internal_GetSceneObjectSelection);
		metaData.scriptClass->addInternalCall("Internal_SetSceneObjectSelection", &ScriptSelection::internal_SetSceneObjectSelection);
		metaData.scriptClass->addInternalCall("Internal_GetResourceUUIDSelection", &ScriptSelection::internal_GetResourceUUIDSelection);
		metaData.scriptClass->addInternalCall("Internal_SetResourceUUIDSelection", &ScriptSelection::internal_SetResourceUUIDSelection);
		metaData.scriptClass->addInternalCall("Internal_GetResourcePathSelection", &ScriptSelection::internal_GetResourcePathSelection);
		metaData.scriptClass->addInternalCall("Internal_SetResourcePathSelection", &ScriptSelection::internal_SetResourcePathSelection);
	}

	void ScriptSelection::internal_GetSceneObjectSelection(MonoArray** selection)
	{
		Vector<HSceneObject> sceneObjects = Selection::instance().getSceneObjects();

		::MonoClass* sceneObjectMonoClass = ScriptSceneObject::getMetaData()->scriptClass->_getInternalClass();
		MonoArray* sceneObjectArray = mono_array_new(MonoManager::instance().getDomain(),
			sceneObjectMonoClass, (UINT32)sceneObjects.size());

		for (UINT32 i = 0; i < (UINT32)sceneObjects.size(); i++)
		{
			// TODO - This bit is commonly used, I should add a method in ScriptGameObjectManager
			ScriptSceneObject* scriptSceneObject = ScriptGameObjectManager::instance().getScriptSceneObject(sceneObjects[i]);
			if (scriptSceneObject == nullptr)
				scriptSceneObject = ScriptGameObjectManager::instance().createScriptSceneObject(sceneObjects[i]);

			MonoObject* sceneMonoObject = scriptSceneObject->getManagedInstance();

			void* elemAddr = mono_array_addr_with_size(sceneObjectArray, sizeof(MonoObject*), i);
			memcpy(elemAddr, &sceneMonoObject, sizeof(MonoObject*));
		}

		*selection = sceneObjectArray;
	}

	void ScriptSelection::internal_SetSceneObjectSelection(MonoArray* selection)
	{
		Vector<HSceneObject> sceneObjects;

		UINT32 arrayLen = (UINT32)mono_array_length(selection);
		for (UINT32 i = 0; i < arrayLen; i++)
		{
			MonoObject* monoSO = mono_array_get(selection, MonoObject*, i);
			ScriptSceneObject* scriptSO = ScriptSceneObject::toNative(monoSO);
			HSceneObject so = static_object_cast<SceneObject>(scriptSO->getNativeHandle());

			sceneObjects.push_back(so);
		}

		Selection::instance().setSceneObjects(sceneObjects);
	}

	void ScriptSelection::internal_GetResourceUUIDSelection(MonoArray** selection)
	{
		Vector<String> uuids = Selection::instance().getResourceUUIDs();

		MonoArray* uuidArray = mono_array_new(MonoManager::instance().getDomain(),
			mono_get_string_class(), (UINT32)uuids.size());

		for (UINT32 i = 0; i < (UINT32)uuids.size(); i++)
		{
			MonoString* monoString = MonoUtil::stringToMono(MonoManager::instance().getDomain(), uuids[i]);

			void* elemAddr = mono_array_addr_with_size(uuidArray, sizeof(MonoString*), i);
			memcpy(elemAddr, &monoString, sizeof(MonoString*));
		}

		*selection = uuidArray;
	}

	void ScriptSelection::internal_SetResourceUUIDSelection(MonoArray* selection)
	{
		Vector<String> uuids;

		UINT32 arrayLen = (UINT32)mono_array_length(selection);
		for (UINT32 i = 0; i < arrayLen; i++)
		{
			MonoString* monoString = mono_array_get(selection, MonoString*, i);
			String uuid = MonoUtil::monoToString(monoString);

			uuids.push_back(uuid);
		}

		Selection::instance().setResourceUUIDs(uuids);
	}

	void ScriptSelection::internal_GetResourcePathSelection(MonoArray** selection)
	{
		Vector<Path> paths = Selection::instance().getResourcePaths();

		MonoArray* pathArray = mono_array_new(MonoManager::instance().getDomain(),
			mono_get_string_class(), (UINT32)paths.size());

		for (UINT32 i = 0; i < (UINT32)paths.size(); i++)
		{
			MonoString* monoString = MonoUtil::stringToMono(MonoManager::instance().getDomain(), paths[i].toString());

			void* elemAddr = mono_array_addr_with_size(pathArray, sizeof(MonoString*), i);
			memcpy(elemAddr, &monoString, sizeof(MonoString*));
		}

		*selection = pathArray;
	}

	void ScriptSelection::internal_SetResourcePathSelection(MonoArray* selection)
	{
		Vector<Path> paths;

		UINT32 arrayLen = (UINT32)mono_array_length(selection);
		for (UINT32 i = 0; i < arrayLen; i++)
		{
			MonoString* monoString = mono_array_get(selection, MonoString*, i);
			Path path  = MonoUtil::monoToString(monoString);

			paths.push_back(path);
		}

		Selection::instance().setResourcePaths(paths);
	}
}