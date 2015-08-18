#pragma once

#include "BsScriptEditorPrerequisites.h"
#include "BsScriptObject.h"
#include "BsHandleSlider.h"
#include "BsQuaternion.h"
#include "BsVector3.h"

namespace BansheeEngine
{
	/**
	 * @brief	Base class for all C++/CLR interop objects that deal with handle sliders.
	 */
	class BS_SCR_BED_EXPORT ScriptHandleSliderBase : public ScriptObjectBase
	{
	public:
		ScriptHandleSliderBase(MonoObject* managedInstance);
		virtual ~ScriptHandleSliderBase() { }

		/**
		 * @brief	Returns the internal native handle slider.
		 */
		virtual HandleSlider* getSlider() const = 0;

	protected:
		friend class ScriptHandleSlider;
		friend class ScriptHandleSliderManager;

		/**
		 * @brief	Destroys the internal native handle slider and unregisters it with
		 *			with handle manager.
		 */
		void destroy();

		/**
		 * @brief	Destroys the internal native handle slider.
		 */
		virtual void destroyInternal() = 0;
	};

	/**
	 * @brief	Interop class between C++ & CLR for HandleSlider.
	 */
	class BS_SCR_BED_EXPORT ScriptHandleSlider : public ScriptObject <ScriptHandleSlider>
	{
	public:
		SCRIPT_OBJ(EDITOR_ASSEMBLY, "BansheeEditor", "HandleSlider")

	private:
		ScriptHandleSlider(MonoObject* instance);

		/************************************************************************/
		/* 								CLR HOOKS						   		*/
		/************************************************************************/
		static void internal_Destroy(ScriptHandleSliderBase* nativeInstance);
		static void internal_GetPosition(ScriptHandleSliderBase* nativeInstance, Vector3* value);
		static void internal_SetPosition(ScriptHandleSliderBase* nativeInstance, Vector3 value);
		static void internal_GetRotation(ScriptHandleSliderBase* nativeInstance, Quaternion* value);
		static void internal_SetRotation(ScriptHandleSliderBase* nativeInstance, Quaternion value);
		static void internal_GetScale(ScriptHandleSliderBase* nativeInstance, Vector3* value);
		static void internal_SetScale(ScriptHandleSliderBase* nativeInstance, Vector3 value);
		static void internal_GetState(ScriptHandleSliderBase* nativeInstance, HandleSlider::State* value);
	};
}