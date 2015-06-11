#include "BsManagedSerializableArray.h"
#include "BsManagedSerializableArrayRTTI.h"
#include "BsMonoManager.h"
#include "BsScriptAssemblyManager.h"
#include "BsManagedSerializableField.h"
#include "BsMonoClass.h"
#include "BsMonoMethod.h"

namespace BansheeEngine
{
	ManagedSerializableArray::ManagedSerializableArray(const ConstructPrivately& dummy)
		:mManagedInstance(nullptr), mElemSize(0), mElementMonoClass(nullptr), mCopyMethod(nullptr)
	{

	}

	ManagedSerializableArray::ManagedSerializableArray(const ConstructPrivately& dummy, const ManagedSerializableTypeInfoArrayPtr& typeInfo, MonoObject* managedInstance)
		: mArrayTypeInfo(typeInfo), mManagedInstance(managedInstance), mElemSize(0), 
		mElementMonoClass(nullptr), mCopyMethod(nullptr)
	{
		::MonoClass* monoClass = mono_object_get_class(mManagedInstance);
		mElemSize = mono_array_element_size(monoClass);

		initMonoObjects();

		mNumElements.resize(typeInfo->mRank);
		for(UINT32 i = 0; i < typeInfo->mRank; i++)
			mNumElements[i] = getLengthInternal(i);
	}

	ManagedSerializableArrayPtr ManagedSerializableArray::createFromExisting(MonoObject* managedInstance, const ManagedSerializableTypeInfoArrayPtr& typeInfo)
	{
		if(managedInstance == nullptr)
			return nullptr;

		if(!ScriptAssemblyManager::instance().getSystemArrayClass()->isInstanceOfType(managedInstance))
			return nullptr;

		return bs_shared_ptr<ManagedSerializableArray>(ConstructPrivately(), typeInfo, managedInstance);
	}

	ManagedSerializableArrayPtr ManagedSerializableArray::createNew(const ManagedSerializableTypeInfoArrayPtr& typeInfo, const Vector<UINT32>& sizes)
	{
		return bs_shared_ptr<ManagedSerializableArray>(ConstructPrivately(), typeInfo, createManagedInstance(typeInfo, sizes));
	}

	ManagedSerializableArrayPtr ManagedSerializableArray::createNew()
	{
		return bs_shared_ptr<ManagedSerializableArray>(ConstructPrivately());
	}

	MonoObject* ManagedSerializableArray::createManagedInstance(const ManagedSerializableTypeInfoArrayPtr& typeInfo, const Vector<UINT32>& sizes)
	{
		if (!typeInfo->isTypeLoaded())
			return nullptr;

		MonoClass* arrayClass = ScriptAssemblyManager::instance().getSystemArrayClass();

		MonoMethod* createInstance = arrayClass->getMethodExact("CreateInstance", "Type,int[]");
		MonoArray* lengthArray = mono_array_new(MonoManager::instance().getDomain(), mono_get_int32_class(), (UINT32)sizes.size());

		for (UINT32 i = 0; i < (UINT32)sizes.size(); i++)
		{
			void* elemAddr = mono_array_addr_with_size(lengthArray, sizeof(int), i);
			memcpy(elemAddr, &sizes[i], sizeof(int));
		}

		void* params[2] = {
			mono_type_get_object(MonoManager::instance().getDomain(), mono_class_get_type(typeInfo->mElementType->getMonoClass())), lengthArray };

		return createInstance->invoke(nullptr, params);
	}

	void ManagedSerializableArray::deserializeManagedInstance(const Vector<ManagedSerializableFieldDataPtr>& entries)
	{
		mManagedInstance = createManagedInstance(mArrayTypeInfo, mNumElements);

		if (mManagedInstance == nullptr)
			return;

		initMonoObjects();

		UINT32 idx = 0;
		for (auto& arrayEntry : entries)
		{
			setFieldData(idx, arrayEntry);
			idx++;
		}
	}

	void ManagedSerializableArray::setFieldData(UINT32 arrayIdx, const ManagedSerializableFieldDataPtr& val)
	{
		if(mono_class_is_valuetype(mElementMonoClass))
			setValue(arrayIdx, val->getValue(mArrayTypeInfo->mElementType));
		else
		{
			MonoObject* ptrToObj = (MonoObject*)val->getValue(mArrayTypeInfo->mElementType);
			setValue(arrayIdx, &ptrToObj);
		}
	}

	ManagedSerializableFieldDataPtr ManagedSerializableArray::getFieldData(UINT32 arrayIdx)
	{
		MonoArray* array = (MonoArray*)mManagedInstance;

		UINT32 numElems = (UINT32)mono_array_length(array);
		assert(arrayIdx < numElems);

		void* arrayValue = mono_array_addr_with_size(array, mElemSize, arrayIdx);

		if(mono_class_is_valuetype(mElementMonoClass))
		{
			MonoObject* boxedObj = nullptr;

			if (arrayValue != nullptr)
				boxedObj = mono_value_box(MonoManager::instance().getDomain(), mElementMonoClass, arrayValue);

			return ManagedSerializableFieldData::create(mArrayTypeInfo->mElementType, boxedObj);
		}
		else
			return ManagedSerializableFieldData::create(mArrayTypeInfo->mElementType, *(MonoObject**)arrayValue);
	}
	
	void ManagedSerializableArray::setValue(UINT32 arrayIdx, void* val)
	{
		MonoArray* array = (MonoArray*)mManagedInstance;

		UINT32 numElems = (UINT32)mono_array_length(array);
		assert(arrayIdx < numElems);
	
		void* elemAddr = mono_array_addr_with_size(array, mElemSize, arrayIdx);
		memcpy(elemAddr, val, mElemSize);
	}

	void ManagedSerializableArray::initMonoObjects()
	{
		mElementMonoClass = mArrayTypeInfo->mElementType->getMonoClass();

		MonoClass* arrayClass = ScriptAssemblyManager::instance().getSystemArrayClass();
		mCopyMethod = arrayClass->getMethodExact("Copy", "Array,Array,int");
	}

	UINT32 ManagedSerializableArray::toSequentialIdx(const Vector<UINT32>& idx) const
	{
		UINT32 mNumDims = (UINT32)mNumElements.size();

		if(idx.size() != mNumDims)
			BS_EXCEPT(InvalidParametersException, "Provided index doesn't have the correct number of dimensions");

		if(mNumElements.size() == 0)
			return 0;

		UINT32 curIdx = 0;
		UINT32 prevDimensionSize = 1;
		
		for(INT32 i = mNumDims - 1; i >= 0; i--)
		{
			curIdx += idx[i] * prevDimensionSize;

			prevDimensionSize *= mNumElements[i];
		}

		return curIdx;
	}

	void ManagedSerializableArray::resize(const Vector<UINT32>& newSizes)
	{
		assert(mArrayTypeInfo->mRank == (UINT32)newSizes.size());

		UINT32 srcCount = 1;
		for (auto& numElems : mNumElements)
			srcCount *= numElems;

		UINT32 dstCount = 1;
		for (auto& numElems : newSizes)
			dstCount *= numElems;

		UINT32 copyCount = std::min(srcCount, dstCount);

		MonoObject* newArray = createManagedInstance(mArrayTypeInfo, newSizes);

		void* params[3];
		params[0] = getManagedInstance();;
		params[1] = newArray;
		params[2] = &copyCount;

		mCopyMethod->invoke(nullptr, params);

		mManagedInstance = newArray;
		mNumElements = newSizes;
	}

	UINT32 ManagedSerializableArray::getLengthInternal(UINT32 dimension) const
	{
		MonoClass* systemArray = ScriptAssemblyManager::instance().getSystemArrayClass();
		MonoMethod* getLength = systemArray->getMethod("GetLength", 1);

		void* params[1] = { &dimension };
		MonoObject* returnObj = getLength->invoke(mManagedInstance, params);

		return *(UINT32*)mono_object_unbox(returnObj);
	}

	UINT32 ManagedSerializableArray::getTotalLength() const
	{
		UINT32 totalNumElements = 1;
		for (auto& numElems : mNumElements)
			totalNumElements *= numElems;

		return totalNumElements;
	}

	RTTITypeBase* ManagedSerializableArray::getRTTIStatic()
	{
		return ManagedSerializableArrayRTTI::instance();
	}

	RTTITypeBase* ManagedSerializableArray::getRTTI() const
	{
		return ManagedSerializableArray::getRTTIStatic();
	}
}