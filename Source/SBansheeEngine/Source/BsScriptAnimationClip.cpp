//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsScriptAnimationClip.h"
#include "BsScriptAnimationCurves.h"
#include "BsScriptMeta.h"
#include "BsMonoClass.h"

namespace BansheeEngine
{
	ScriptAnimationClip::ScriptAnimationClip(MonoObject* instance, const HAnimationClip& animationClip)
		:TScriptResource(instance, animationClip)
	{

	}

	void ScriptAnimationClip::initRuntimeData()
	{
		metaData.scriptClass->addInternalCall("Internal_GetAnimationCurves", &ScriptAnimationClip::internal_GetAnimationCurves);
		metaData.scriptClass->addInternalCall("Internal_SetAnimationCurves", &ScriptAnimationClip::internal_SetAnimationCurves);
		metaData.scriptClass->addInternalCall("Internal_GetAnimationEvents", &ScriptAnimationClip::internal_GetAnimationEvents);
		metaData.scriptClass->addInternalCall("Internal_SetAnimationEvents", &ScriptAnimationClip::internal_SetAnimationEvents);
		metaData.scriptClass->addInternalCall("Internal_GetLength", &ScriptAnimationClip::internal_GetLength);
	}

	MonoObject* ScriptAnimationClip::createInstance()
	{
		return metaData.scriptClass->createInstance();
	}

	MonoObject* ScriptAnimationClip::internal_GetAnimationCurves(ScriptAnimationClip* thisPtr)
	{
		SPtr<AnimationCurves> curves = thisPtr->getHandle()->getCurves();
		return ScriptAnimationCurves::toManaged(curves);
	}

	void ScriptAnimationClip::internal_SetAnimationCurves(ScriptAnimationClip* thisPtr, MonoObject* curves)
	{
		SPtr<AnimationCurves> nativeCurves = ScriptAnimationCurves::toNative(curves);
		thisPtr->getHandle()->setCurves(*nativeCurves);
	}

	MonoArray* ScriptAnimationClip::internal_GetAnimationEvents(ScriptAnimationClip* thisPtr)
	{
		const Vector<AnimationEvent>& events = thisPtr->getHandle()->getEvents();

		UINT32 numEvents = (UINT32)events.size();
		ScriptArray eventsArray = ScriptArray::create<ScriptAnimationEvent>(numEvents);
		for (UINT32 i = 0; i < numEvents; i++)
			eventsArray.set(i, ScriptAnimationEvent::toManaged(events[i]));

		return eventsArray.getInternal();
	}

	void ScriptAnimationClip::internal_SetAnimationEvents(ScriptAnimationClip* thisPtr, MonoArray* events)
	{
		Vector<AnimationEvent> nativeEvents;

		if (events != nullptr)
		{
			ScriptArray eventsArray(events);

			for (UINT32 i = 0; i < eventsArray.size(); i++)
			{
				AnimationEvent event = ScriptAnimationEvent::toNative(eventsArray.get<MonoObject*>(i));
				nativeEvents.push_back(event);
			}
		}

		thisPtr->getHandle()->setEvents(nativeEvents);
	}

	float ScriptAnimationClip::internal_GetLength(ScriptAnimationClip* thisPtr)
	{
		return thisPtr->getHandle()->getLength();
	}

	MonoField* ScriptAnimationEvent::sNameField = nullptr;
	MonoField* ScriptAnimationEvent::sTimeField = nullptr;

	ScriptAnimationEvent::ScriptAnimationEvent(MonoObject* instance)
		:ScriptObject(instance)
	{ }

	void ScriptAnimationEvent::initRuntimeData()
	{
		sNameField = metaData.scriptClass->getField("Name");
		sTimeField = metaData.scriptClass->getField("Time");
	}

	AnimationEvent ScriptAnimationEvent::toNative(MonoObject* instance)
	{
		AnimationEvent output;

		MonoString* monoName = nullptr;
		sNameField->getValue(instance, &monoName);

		output.name = MonoUtil::monoToString(monoName);

		sTimeField->getValue(instance, &output.time);

		return output;
	}

	MonoObject* ScriptAnimationEvent::toManaged(const AnimationEvent& event)
	{
		MonoString* monoString = MonoUtil::stringToMono(event.name);
		float time = event.time;

		void* params[2] = { monoString, &time };
		return metaData.scriptClass->createInstance("string, float", params);
	}
}