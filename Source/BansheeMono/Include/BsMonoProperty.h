//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsMonoPrerequisites.h"
#include <mono/jit/jit.h>

namespace BansheeEngine
{
	/** @addtogroup Mono
	 *  @{
	 */

	/**
	 * Encapsulates information about a single Mono (managed) property belonging to some managed class. This object
	 * also allows you to set or retrieve values to/from specific instances containing the property.
	 */
	class BS_MONO_EXPORT MonoProperty
	{
	public:
		/**
		 * Returns a boxed value contained in the property in the specified instance.
		 *
		 * @param[in]	instance	Object instance to access the property on. Can be null for static properties.
		 * @return					A boxed value of the property.
		 */
		MonoObject* get(MonoObject* instance) const;

		/**
		 * Sets a value of the property in the specified instance. 
		 *
		 * @param[in]	instance	Object instance to access the property on. Can be null for static properties.
		 * @param[in]	value		Value to set on the property. For value type it should be a pointer to the value and for
		 *							reference type it should be a pointer to MonoObject.
		 */
		void set(MonoObject* instance, void* value) const;

		/**
		 * Returns a boxed value contained in the property in the specified instance. Used for properties with indexers.
		 *
		 * @param[in]	instance	Object instance to access the property on. Can be null for static properties.
		 * @param[in]	index		Index of the value to retrieve.
		 * @return					A boxed value of the property.
		 */
		MonoObject* getIndexed(MonoObject* instance, UINT32 index) const;

		/**
		 * Sets a value of the property in the specified instance. Used for properties with indexers.
		 *
		 * @param[in]	instance	Object instance to access the property on. Can be null for static properties.
		 * @param[in]	index		Index of the value to set.
		 * @param[in]	value		Value to set on the property. For value type it should be a pointer to the value and for
		 *							reference type it should be a pointer to MonoObject.
		 */
		void setIndexed(MonoObject* instance, UINT32 index, void* value) const;

		/**	Returns the data type the property holds. */
		MonoClass* getReturnType();
	private:
		friend class MonoClass;

		MonoProperty(::MonoProperty* monoProp);

		::MonoProperty* mProperty;
		::MonoMethod* mGetMethod;
		::MonoMethod* mSetMethod;
		MonoClass* mGetReturnType;
	};

	/** @} */
}