#include "BsScriptVector.h"
#include "BsMonoManager.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"

namespace BansheeEngine
{
	ScriptVector2::ScriptVector2(MonoObject* instance)
		:ScriptObject(instance)
	{ }

	void ScriptVector2::initRuntimeData()
	{ }

	MonoObject* ScriptVector2::box(const Vector2& value)
	{
		// We're casting away const but it's fine since structs are passed by value anyway
		return mono_value_box(MonoManager::instance().getDomain(),
			metaData.scriptClass->_getInternalClass(), (void*)&value);
	}

	Vector2 ScriptVector2::unbox(MonoObject* obj)
	{
		return *(Vector2*)mono_object_unbox(obj);
	}

	ScriptVector3::ScriptVector3(MonoObject* instance)
		:ScriptObject(instance)
	{ }

	void ScriptVector3::initRuntimeData()
	{ }

	MonoObject* ScriptVector3::box(const Vector3& value)
	{
		// We're casting away const but it's fine since structs are passed by value anyway
		return mono_value_box(MonoManager::instance().getDomain(),
			metaData.scriptClass->_getInternalClass(), (void*)&value);
	}

	Vector3 ScriptVector3::unbox(MonoObject* obj)
	{
		return *(Vector3*)mono_object_unbox(obj);
	}

	ScriptVector4::ScriptVector4(MonoObject* instance)
		:ScriptObject(instance)
	{ }

	void ScriptVector4::initRuntimeData()
	{ }

	MonoObject* ScriptVector4::box(const Vector4& value)
	{
		// We're casting away const but it's fine since structs are passed by value anyway
		return mono_value_box(MonoManager::instance().getDomain(),
			metaData.scriptClass->_getInternalClass(), (void*)&value);
	}

	Vector4 ScriptVector4::unbox(MonoObject* obj)
	{
		return *(Vector4*)mono_object_unbox(obj);
	}
}