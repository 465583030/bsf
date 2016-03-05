//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsManagedSerializableDictionary.h"
#include "BsManagedSerializableDictionaryRTTI.h"
#include "BsMonoManager.h"
#include "BsScriptAssemblyManager.h"
#include "BsManagedSerializableField.h"
#include "BsMonoClass.h"
#include "BsMonoMethod.h"
#include "BsMonoProperty.h"
#include "BsMonoUtil.h"

namespace BansheeEngine
{
	ManagedSerializableDictionaryKeyValue::ManagedSerializableDictionaryKeyValue(const ManagedSerializableFieldDataPtr& key,
		const ManagedSerializableFieldDataPtr& value)
		:key(key), value(value)
	{
		
	}

	RTTITypeBase* ManagedSerializableDictionaryKeyValue::getRTTIStatic()
	{
		return ManagedSerializableDictionaryKeyValueRTTI::instance();
	}

	RTTITypeBase* ManagedSerializableDictionaryKeyValue::getRTTI() const
	{
		return ManagedSerializableDictionaryKeyValue::getRTTIStatic();
	}

	inline size_t ManagedSerializableDictionary::Hash::operator()(const ManagedSerializableFieldDataPtr& x) const
	{
		return x->getHash();
	}

	inline bool ManagedSerializableDictionary::Equals::operator()(const ManagedSerializableFieldDataPtr& a, const ManagedSerializableFieldDataPtr& b) const
	{
		return a->equals(b);
	}

	ManagedSerializableDictionary::Enumerator::Enumerator(MonoObject* instance, const ManagedSerializableDictionary* parent)
		:mInstance(instance), mParent(parent), mCurrent(nullptr), mIteratorInitialized(false)
	{ }

	ManagedSerializableFieldDataPtr ManagedSerializableDictionary::Enumerator::getKey() const
	{
		if (mInstance != nullptr)
		{
			MonoObject* obj = mParent->mKeyProp->get(mCurrent);

			return ManagedSerializableFieldData::create(mParent->mDictionaryTypeInfo->mKeyType, obj);
		}
		else
		{
			return mCachedIter->first;
		}
	}

	ManagedSerializableFieldDataPtr ManagedSerializableDictionary::Enumerator::getValue() const
	{
		if (mInstance != nullptr)
		{
			MonoObject* obj = mParent->mValueProp->get(mCurrent);

			return ManagedSerializableFieldData::create(mParent->mDictionaryTypeInfo->mValueType, obj);
		}
		else
		{
			return mCachedIter->second;
		}
	}

	bool ManagedSerializableDictionary::Enumerator::moveNext()
	{
		if (mInstance != nullptr)
		{
			MonoObject* returnVal = mParent->mEnumMoveNext->invoke(mInstance, nullptr);
			bool isValid = *(bool*)mono_object_unbox(returnVal);

			if (isValid)
				mCurrent = (MonoObject*)mono_object_unbox(mParent->mEnumCurrentProp->get(mInstance));
			else
				mCurrent = nullptr;

			return isValid;
		}
		else
		{
			if (!mIteratorInitialized)
			{
				mCachedIter = mParent->mCachedEntries.begin();
				mIteratorInitialized = true;
			}
			else
				++mCachedIter;

			return mCachedIter != mParent->mCachedEntries.end();
		}
	}

	ManagedSerializableDictionary::ManagedSerializableDictionary(const ConstructPrivately& dummy)
		:mManagedInstance(nullptr), mAddMethod(nullptr), mGetEnumerator(nullptr), mEnumMoveNext(nullptr), mRemoveMethod(nullptr),
		mEnumCurrentProp(nullptr), mKeyProp(nullptr), mValueProp(nullptr), mContainsKeyMethod(nullptr), mTryGetValueMethod(nullptr)
	{ }

	ManagedSerializableDictionary::ManagedSerializableDictionary(const ConstructPrivately& dummy, const ManagedSerializableTypeInfoDictionaryPtr& typeInfo, MonoObject* managedInstance)
		:mDictionaryTypeInfo(typeInfo), mManagedInstance(managedInstance), mAddMethod(nullptr), mGetEnumerator(nullptr), mEnumMoveNext(nullptr),
		mEnumCurrentProp(nullptr), mKeyProp(nullptr), mValueProp(nullptr), mContainsKeyMethod(nullptr), mTryGetValueMethod(nullptr), mRemoveMethod(nullptr)
	{
		MonoClass* dictClass = MonoManager::instance().findClass(mono_object_get_class(managedInstance));
		if (dictClass == nullptr)
			return;

		initMonoObjects(dictClass);
	}

	ManagedSerializableDictionaryPtr ManagedSerializableDictionary::createFromExisting(MonoObject* managedInstance, const ManagedSerializableTypeInfoDictionaryPtr& typeInfo)
	{
		if(managedInstance == nullptr)
			return nullptr;

		String elementNs;
		String elementTypeName;
		MonoUtil::getClassName(managedInstance, elementNs, elementTypeName);

		String fullName = elementNs + "." + elementTypeName;

		if(ScriptAssemblyManager::instance().getSystemGenericDictionaryClass()->getFullName() != fullName)
			return nullptr;

		return bs_shared_ptr_new<ManagedSerializableDictionary>(ConstructPrivately(), typeInfo, managedInstance);
	}

	ManagedSerializableDictionaryPtr ManagedSerializableDictionary::createNew(const ManagedSerializableTypeInfoDictionaryPtr& typeInfo)
	{
		return bs_shared_ptr_new<ManagedSerializableDictionary>(ConstructPrivately(), typeInfo, createManagedInstance(typeInfo));
	}

	MonoObject* ManagedSerializableDictionary::createManagedInstance(const ManagedSerializableTypeInfoDictionaryPtr& typeInfo)
	{
		if (!typeInfo->isTypeLoaded())
			return nullptr;

		::MonoClass* dictionaryMonoClass = typeInfo->getMonoClass();
		MonoClass* dictionaryClass = MonoManager::instance().findClass(dictionaryMonoClass);
		if (dictionaryClass == nullptr)
			return nullptr;

		return dictionaryClass->createInstance();
	}

	ManagedSerializableDictionaryPtr ManagedSerializableDictionary::createEmpty()
	{
		return bs_shared_ptr_new<ManagedSerializableDictionary>(ConstructPrivately());
	}

	void ManagedSerializableDictionary::serialize()
	{
		if (mManagedInstance == nullptr)
			return;

		MonoClass* dictionaryClass = MonoManager::instance().findClass(mono_object_get_class(mManagedInstance));
		if (dictionaryClass == nullptr)
			return;

		initMonoObjects(dictionaryClass);
		mCachedEntries.clear();

		Enumerator enumerator = getEnumerator();

		while (enumerator.moveNext())
		{
			ManagedSerializableFieldDataPtr key = enumerator.getKey();
			mCachedEntries.insert(std::make_pair(key, enumerator.getValue()));
		}

		// Serialize children
		for (auto& fieldEntry : mCachedEntries)
		{
			fieldEntry.first->serialize();
			fieldEntry.second->serialize();
		}

		mManagedInstance = nullptr;
	}

	void ManagedSerializableDictionary::deserialize()
	{
		mManagedInstance = createManagedInstance(mDictionaryTypeInfo);

		if (mManagedInstance == nullptr)
		{
			mCachedEntries.clear();
			return;
		}

		::MonoClass* dictionaryMonoClass = mDictionaryTypeInfo->getMonoClass();
		MonoClass* dictionaryClass = MonoManager::instance().findClass(dictionaryMonoClass);
		if (dictionaryClass == nullptr)
			return;

		initMonoObjects(dictionaryClass);

		// Deserialize children
		for (auto& fieldEntry : mCachedEntries)
		{
			fieldEntry.first->deserialize();
			fieldEntry.second->deserialize();
		}

		UINT32 idx = 0;
		for (auto& entry : mCachedEntries)
		{
			setFieldData(entry.first, entry.second);
			idx++;
		}

		mCachedEntries.clear();
	}

	ManagedSerializableFieldDataPtr ManagedSerializableDictionary::getFieldData(const ManagedSerializableFieldDataPtr& key)
	{
		if (mManagedInstance != nullptr)
		{
			MonoObject* value = nullptr;

			void* params[2];
			params[0] = key->getValue(mDictionaryTypeInfo->mKeyType);
			params[1] = &value;

			mTryGetValueMethod->invoke(mManagedInstance, params);

			MonoObject* boxedValue = value;
			::MonoClass* valueTypeClass = mDictionaryTypeInfo->mValueType->getMonoClass();
			if (mono_class_is_valuetype(valueTypeClass))
			{
				if (value != nullptr)
					boxedValue = mono_value_box(MonoManager::instance().getDomain(), valueTypeClass, &value);
			}

			return ManagedSerializableFieldData::create(mDictionaryTypeInfo->mValueType, boxedValue);
		}
		else
		{
			return mCachedEntries[key];
		}
	}

	void ManagedSerializableDictionary::setFieldData(const ManagedSerializableFieldDataPtr& key, const ManagedSerializableFieldDataPtr& val)
	{
		if (mManagedInstance != nullptr)
		{
			void* params[2];
			params[0] = key->getValue(mDictionaryTypeInfo->mKeyType);
			params[1] = val->getValue(mDictionaryTypeInfo->mValueType);

			mAddMethod->invoke(mManagedInstance, params);
		}
		else
		{
			mCachedEntries[key] = val;
		}
	}

	void ManagedSerializableDictionary::removeFieldData(const ManagedSerializableFieldDataPtr& key)
	{
		if (mManagedInstance != nullptr)
		{
			void* params[1];
			params[0] = key->getValue(mDictionaryTypeInfo->mKeyType);

			mRemoveMethod->invoke(mManagedInstance, params);
		}
		else
		{
			auto findIter = mCachedEntries.find(key);
			if (findIter != mCachedEntries.end())
				mCachedEntries.erase(findIter);
		}
	}

	bool ManagedSerializableDictionary::contains(const ManagedSerializableFieldDataPtr& key) const
	{
		if (mManagedInstance != nullptr)
		{
			void* params[1];
			params[0] = key->getValue(mDictionaryTypeInfo->mKeyType);

			MonoObject* returnVal = mContainsKeyMethod->invoke(mManagedInstance, params);
			return *(bool*)mono_object_unbox(returnVal);
		}
		else
			return mCachedEntries.find(key) != mCachedEntries.end();
	}

	ManagedSerializableDictionary::Enumerator ManagedSerializableDictionary::getEnumerator() const
	{
		return Enumerator((MonoObject*)mono_object_unbox(mGetEnumerator->invoke(mManagedInstance, nullptr)), this);
	}

	void ManagedSerializableDictionary::initMonoObjects(MonoClass* dictionaryClass)
	{
		mAddMethod = dictionaryClass->getMethod("Add", 2);
		mRemoveMethod = dictionaryClass->getMethod("Remove", 1);
		mTryGetValueMethod = dictionaryClass->getMethod("TryGetValue", 2);
		mContainsKeyMethod = dictionaryClass->getMethod("ContainsKey", 1);
		mGetEnumerator = dictionaryClass->getMethod("GetEnumerator");

		MonoClass* enumeratorClass = mGetEnumerator->getReturnType();
		assert(enumeratorClass != nullptr);

		mEnumMoveNext = enumeratorClass->getMethod("MoveNext");
		mEnumCurrentProp = &enumeratorClass->getProperty("Current");

		MonoClass* keyValuePairClass = mEnumCurrentProp->getReturnType();
		assert(keyValuePairClass != nullptr);

		mKeyProp = &keyValuePairClass->getProperty("Key");
		mValueProp = &keyValuePairClass->getProperty("Value");
	}

	RTTITypeBase* ManagedSerializableDictionary::getRTTIStatic()
	{
		return ManagedSerializableDictionaryRTTI::instance();
	}

	RTTITypeBase* ManagedSerializableDictionary::getRTTI() const
	{
		return ManagedSerializableDictionary::getRTTIStatic();
	}
}