//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "BsIReflectable.h"
#include <mono/jit/jit.h>

namespace BansheeEngine
{
	/**
	 * @brief	Allows access to an underlying managed array, or a cached version of that array that
	 *			can be serialized/deserialized.
	 *
	 * @note	This class can be in two states:
	 *			 - Linked - When the object has a link to a managed object. This is the default 
	 *                      state when a new instance of ManagedSerializableObject is created.
	 *						Any operations during this state will operate directly on the linked
	 *						managed object.
	 *			 - Serialized - When the object has no link to the managed object but instead just
	 *							contains cached object and field data that may be used for initializing
	 *							a managed object. Any operations during this state will operate
	 *							only on the cached internal data.
	 *			You can transfer between these states by calling serialize(linked->serialized) &
	 *			deserialize (serialized->linked).
	 *	
	 */
	class BS_SCR_BE_EXPORT ManagedSerializableArray : public IReflectable
	{
	private:
		struct ConstructPrivately {};

	public:
		ManagedSerializableArray(const ConstructPrivately& dummy, const ManagedSerializableTypeInfoArrayPtr& typeInfo, MonoObject* managedInstance);
		ManagedSerializableArray(const ConstructPrivately& dummy);

		/**
		 * @brief	Returns the internal managed instance of the array. This will return null if
		 *			the object is in serialized mode.
		 */
		MonoObject* getManagedInstance() const { return mManagedInstance; }

		/**
		 * @brief	Returns the type information for the internal array.
		 */
		ManagedSerializableTypeInfoArrayPtr getTypeInfo() const { return mArrayTypeInfo; }

		/**
		 * @brief	Changes the size of the array. Operates on managed object if in linked state, 
		 *			or on cached data otherwise.
		 *
		 * @param	newSizes	Array of sizes, one per array dimension. Number of sizes
		 *						must match number of array dimensions as specified by its type.
		 */
		void resize(const Vector<UINT32>& newSizes);

		/**
		 * @brief	Returns the size of a specific dimension of the array. Operates on 
		 *			managed object if in linked state, or on cached data otherwise.
		 */
		UINT32 getLength(UINT32 dimension) const { return mNumElements[dimension]; }

		/**
		 * @brief	Returns the sizes of a all dimensions of the array. Operates on 
		 *			managed object if in linked state, or on cached data otherwise.
		 */
		Vector<UINT32> getLengths() const { return mNumElements; }

		/**
		 * @brief	Returns the total of all sizes of all dimensions of the array. Operates on 
		 *			managed object if in linked state, or on cached data otherwise.
		 */
		UINT32 getTotalLength() const;

		/**
		 * @brief	Sets a new element value at the specified array index. Operates on 
		 *			managed object if in linked state, or on cached data otherwise.
		 *
		 * @param	arrayIdx	Index at which to set the value.
		 * @param	val			Wrapper around the value to store in the array. Must be of the
		 *						array element type.
		 */
		void setFieldData(UINT32 arrayIdx, const ManagedSerializableFieldDataPtr& val);

		/**
		 * @brief	Returns the element value at the specified array index. Operates on 
		 *			managed object if in linked state, or on cached data otherwise.
		 *
		 * @param	arrayIdx	Index at which to retrieve the value.
		 *
		 * @return	A wrapper around the element value in the array.
		 */
		ManagedSerializableFieldDataPtr getFieldData(UINT32 arrayIdx);

		/**
		 * @brief	Serializes the internal managed object into a set of cached data that can be saved
		 *			in memory/disk and can be deserialized later. Does nothing if object is already is 
		 *			serialized mode. When in serialized mode the reference to the managed instance will be lost.
		 */
		void serialize();

		/**
		 * @brief	Deserializes a set of cached data into a managed object. This action may fail in case the cached
		 *			data contains a type that no longer exists. You may check if it completely successfully if ::getManagedInstance
		 *			returns non-null after.
		 *
		 *			This action transfers the object into linked mode. All further operations will operate directly on the managed instance
		 *			and the cached data will be cleared. If you call this method on an already linked object the old object will be
		 *			replaced and initialized with empty data (since cached data does not exist).
		 */
		void deserialize();

		/**
		 * @brief	Creates a managed serializable array that references an existing managed array. Created object will be in linked mode.
		 *
		 * @param	managedInstance		Constructed managed instance of the array to link with. Its type must correspond with the provided type info.
		 * @param	typeInfo			Type information for the array and its elements.
		 */
		static ManagedSerializableArrayPtr createFromExisting(MonoObject* managedInstance, const ManagedSerializableTypeInfoArrayPtr& typeInfo);

		/**
		 * @brief	Creates a managed serializable array that creates and references a brand new managed array instance.
		 *
		 * @param	typeInfo	Type of the array to create.
		 * @param	sizes		Array of sizes, one per array dimension. Number of sizes must match number 
		 *						of array dimensions as specified by its type.
		 */
		static ManagedSerializableArrayPtr createNew(const ManagedSerializableTypeInfoArrayPtr& typeInfo, const Vector<UINT32>& sizes);

		/**
		 * @brief	Creates a managed array instance.
		 *
		 * @param	typeInfo	Type of the array to create.
		 * @param	sizes		Array of sizes, one per array dimension. Number of sizes must match number 
		 *						of array dimensions as specified by its type.
		 */
		static MonoObject* createManagedInstance(const ManagedSerializableTypeInfoArrayPtr& typeInfo, const Vector<UINT32>& sizes);

	protected:
		/**
		 * @brief	Retrieves needed Mono types and methods. Should be called
		 *			before performing any operations with the managed object.
		 */
		void initMonoObjects();

		/**
		 * @brief	Returns the size of the specified dimension of the array.
		 *			Operates on the internal managed object.
		 */
		UINT32 getLengthInternal(UINT32 dimension) const;

		/**
		 * @brief	Sets a value at the specified index in the array.
		 *			Operates on the internal managed object.
		 */
		void setValueInternal(UINT32 arrayIdx, void* val);

		/**
		 * @brief	Converts a multi-dimensional array index into
		 *			a sequential one-dimensional index.
		 */
		UINT32 toSequentialIdx(const Vector<UINT32>& idx) const;

		MonoObject* mManagedInstance;
		::MonoClass* mElementMonoClass;
		MonoMethod* mCopyMethod;

		ManagedSerializableTypeInfoArrayPtr mArrayTypeInfo;
		Vector<ManagedSerializableFieldDataPtr> mCachedEntries;
		Vector<UINT32> mNumElements;
		UINT32 mElemSize;

		/************************************************************************/
		/* 								RTTI		                     		*/
		/************************************************************************/
		
		/**
		 * @brief	Creates an empty and uninitialized object used for serialization purposes.
		 */
		static ManagedSerializableArrayPtr createNew();

	public:
		friend class ManagedSerializableArrayRTTI;
		static RTTITypeBase* getRTTIStatic();
		virtual RTTITypeBase* getRTTI() const override;
	};
}