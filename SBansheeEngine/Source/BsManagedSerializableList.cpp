#include "BsManagedSerializableList.h"
#include "BsManagedSerializableListRTTI.h"
#include "BsMonoManager.h"
#include "BsScriptAssemblyManager.h"
#include "BsManagedSerializableField.h"
#include "BsMonoClass.h"
#include "BsMonoMethod.h"
#include "BsMonoProperty.h"

namespace BansheeEngine
{
	ManagedSerializableList::ManagedSerializableList(const ConstructPrivately& dummy)
		:mManagedInstance(nullptr), mNumElements(0), mItemProp(nullptr), mCountProp(nullptr), mAddMethod(nullptr)
	{

	}

	ManagedSerializableList::ManagedSerializableList(const ConstructPrivately& dummy, const ManagedSerializableTypeInfoListPtr& typeInfo, MonoObject* managedInstance)
		:mListTypeInfo(typeInfo), mManagedInstance(managedInstance), mNumElements(0), mItemProp(nullptr),
		mCountProp(nullptr), mAddMethod(nullptr)
	{
		MonoClass* listClass = MonoManager::instance().findClass(mono_object_get_class(managedInstance));
		if(listClass == nullptr)
			return;

		initMonoObjects(listClass);

		mNumElements = getLength();
	}

	ManagedSerializableListPtr ManagedSerializableList::createFromExisting(MonoObject* managedInstance, const ManagedSerializableTypeInfoListPtr& typeInfo)
	{
		if(managedInstance == nullptr)
			return nullptr;

		::MonoClass* monoClass = mono_object_get_class(managedInstance);
		String elementNs = mono_class_get_namespace(monoClass);
		String elementTypeName = mono_class_get_name(monoClass);
		String fullName = elementNs + "." + elementTypeName;

		if(ScriptAssemblyManager::instance().getSystemGenericListClass()->getFullName() != fullName)
			return nullptr;

		return bs_shared_ptr<ManagedSerializableList>(ConstructPrivately(), typeInfo, managedInstance);
	}

	ManagedSerializableListPtr ManagedSerializableList::createFromNew(const ManagedSerializableTypeInfoListPtr& typeInfo, UINT32 size)
	{
		return bs_shared_ptr<ManagedSerializableList>(ConstructPrivately(), typeInfo, createManagedInstance(typeInfo, size));
	}

	MonoObject* ManagedSerializableList::createManagedInstance(const ManagedSerializableTypeInfoListPtr& typeInfo, UINT32 size)
	{
		if (!typeInfo->isTypeLoaded())
			return nullptr;

		::MonoClass* listMonoClass = typeInfo->getMonoClass();
		MonoClass* listClass = MonoManager::instance().findClass(listMonoClass);
		if (listClass == nullptr)
			return nullptr;

		void* params[1] = { &size };
		return listClass->createInstance("int", params);
	}

	ManagedSerializableListPtr ManagedSerializableList::createEmpty()
	{
		return bs_shared_ptr<ManagedSerializableList>(ConstructPrivately());
	}

	void ManagedSerializableList::deserializeManagedInstance(const Vector<ManagedSerializableFieldDataPtr>& entries)
	{
		mManagedInstance = createManagedInstance(mListTypeInfo, mNumElements);

		if (mManagedInstance == nullptr)
			return;

		MonoClass* listClass = MonoManager::instance().findClass(mListTypeInfo->getMonoClass());
		initMonoObjects(listClass);

		for(auto& arrayEntry : entries)
		{
			addFieldData(arrayEntry);
		}
	}

	void ManagedSerializableList::setFieldData(UINT32 arrayIdx, const ManagedSerializableFieldDataPtr& val)
	{
		mItemProp->setIndexed(mManagedInstance, arrayIdx, val->getValue(mListTypeInfo->mElementType));
	}


	void ManagedSerializableList::addFieldData(const ManagedSerializableFieldDataPtr& val)
	{
		void* params[1];
		params[0] = val->getValue(mListTypeInfo->mElementType);
		mAddMethod->invoke(mManagedInstance, params);
	}

	ManagedSerializableFieldDataPtr ManagedSerializableList::getFieldData(UINT32 arrayIdx)
	{
		MonoObject* obj = mItemProp->getIndexed(mManagedInstance, arrayIdx);

		return ManagedSerializableFieldData::create(mListTypeInfo->mElementType, obj);
	}

	UINT32 ManagedSerializableList::getLength() const
	{
		MonoObject* length = mCountProp->get(mManagedInstance);

		if(length == nullptr)
			return 0;

		return *(UINT32*)mono_object_unbox(length);
	}

	void ManagedSerializableList::initMonoObjects(MonoClass* listClass)
	{
		mItemProp = &listClass->getProperty("Item");
		mCountProp = &listClass->getProperty("Count");
		mAddMethod = listClass->getMethod("Add", 1);
	}

	RTTITypeBase* ManagedSerializableList::getRTTIStatic()
	{
		return ManagedSerializableListRTTI::instance();
	}

	RTTITypeBase* ManagedSerializableList::getRTTI() const
	{
		return ManagedSerializableList::getRTTIStatic();
	}
}