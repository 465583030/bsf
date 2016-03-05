//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsScriptColor.h"
#include "BsMonoManager.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"

namespace BansheeEngine
{
	ScriptColor::ScriptColor(MonoObject* instance)
		:ScriptObject(instance)
	{ }

	void ScriptColor::initRuntimeData()
	{ }

	MonoObject* ScriptColor::box(const Color& value)
	{
		// We're casting away const but it's fine since structs are passed by value anyway
		return mono_value_box(MonoManager::instance().getDomain(),
			metaData.scriptClass->_getInternalClass(), (void*)&value);
	}

	Color ScriptColor::unbox(MonoObject* obj)
	{
		return *(Color*)mono_object_unbox(obj);
	}
}