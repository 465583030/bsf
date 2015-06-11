#include "BsManagedSerializableDiff.h"
#include "BsManagedSerializableObject.h"
#include "BsManagedSerializableObjectInfo.h"
#include "BsManagedSerializableField.h"
#include "BsManagedSerializableArray.h"
#include "BsManagedSerializableList.h"
#include "BsManagedSerializableDictionary.h"
#include "BsManagedSerializableDiffRTTI.h"

namespace BansheeEngine
{
	ManagedSerializableDiff::ModifiedField::ModifiedField(const ManagedSerializableTypeInfoPtr& parentType,
		const ManagedSerializableFieldInfoPtr& fieldType, const SPtr<Modification>& modification)
		:parentType(parentType), fieldType(fieldType), modification(modification)
	{

	}

	RTTITypeBase* ManagedSerializableDiff::ModifiedField::getRTTIStatic()
	{
		return ModifiedFieldRTTI::instance();
	}

	RTTITypeBase* ManagedSerializableDiff::ModifiedField::getRTTI() const
	{
		return getRTTIStatic();
	}

	ManagedSerializableDiff::ModifiedArrayEntry::ModifiedArrayEntry(UINT32 idx, const SPtr<Modification>& modification)
		:idx(idx), modification(modification)
	{

	}

	RTTITypeBase* ManagedSerializableDiff::ModifiedArrayEntry::getRTTIStatic()
	{
		return ModifiedArrayEntryRTTI::instance();
	}

	RTTITypeBase* ManagedSerializableDiff::ModifiedArrayEntry::getRTTI() const
	{
		return getRTTIStatic();
	}

	ManagedSerializableDiff::ModifiedDictionaryEntry::ModifiedDictionaryEntry(
		const ManagedSerializableFieldDataPtr& key, const SPtr<Modification>& modification)
		:key(key), modification(modification)
	{

	}

	RTTITypeBase* ManagedSerializableDiff::ModifiedDictionaryEntry::getRTTIStatic()
	{
		return ModifiedDictionaryEntryRTTI::instance();
	}

	RTTITypeBase* ManagedSerializableDiff::ModifiedDictionaryEntry::getRTTI() const
	{
		return getRTTIStatic();
	}

	ManagedSerializableDiff::Modification::~Modification()
	{
		
	}

	RTTITypeBase* ManagedSerializableDiff::Modification::getRTTIStatic()
	{
		return ModificationRTTI::instance();
	}

	RTTITypeBase* ManagedSerializableDiff::Modification::getRTTI() const
	{
		return getRTTIStatic();
	}

	SPtr<ManagedSerializableDiff::ModifiedObject> ManagedSerializableDiff::ModifiedObject::create()
	{
		return bs_shared_ptr<ModifiedObject>();
	}

	RTTITypeBase* ManagedSerializableDiff::ModifiedObject::getRTTIStatic()
	{
		return ModifiedObjectRTTI::instance();
	}

	RTTITypeBase* ManagedSerializableDiff::ModifiedObject::getRTTI() const
	{
		return getRTTIStatic();
	}

	SPtr<ManagedSerializableDiff::ModifiedArray> ManagedSerializableDiff::ModifiedArray::create()
	{
		return bs_shared_ptr<ModifiedArray>();
	}

	RTTITypeBase* ManagedSerializableDiff::ModifiedArray::getRTTIStatic()
	{
		return ModifiedArrayRTTI::instance();
	}

	RTTITypeBase* ManagedSerializableDiff::ModifiedArray::getRTTI() const
	{
		return getRTTIStatic();
	}

	SPtr<ManagedSerializableDiff::ModifiedDictionary> ManagedSerializableDiff::ModifiedDictionary::create()
	{
		return bs_shared_ptr<ModifiedDictionary>();
	}

	RTTITypeBase* ManagedSerializableDiff::ModifiedDictionary::getRTTIStatic()
	{
		return ModifiedDictionaryRTTI::instance();
	}

	RTTITypeBase* ManagedSerializableDiff::ModifiedDictionary::getRTTI() const
	{
		return getRTTIStatic();
	}

	ManagedSerializableDiff::ModifiedEntry::ModifiedEntry(const ManagedSerializableFieldDataPtr& value)
		:value(value)
	{
		
	}

	SPtr<ManagedSerializableDiff::ModifiedEntry> ManagedSerializableDiff::ModifiedEntry::create(const ManagedSerializableFieldDataPtr& value)
	{
		return bs_shared_ptr<ModifiedEntry>(value);
	}

	RTTITypeBase* ManagedSerializableDiff::ModifiedEntry::getRTTIStatic()
	{
		return ModifiedEntryRTTI::instance();
	}

	RTTITypeBase* ManagedSerializableDiff::ModifiedEntry::getRTTI() const
	{
		return getRTTIStatic();
	}

	ManagedSerializableDiff::ManagedSerializableDiff()
		: mModificationRoot(ModifiedObject::create())
	{
		
	}

	ManagedSerializableDiff::~ManagedSerializableDiff()
	{
	}

	ManagedSerializableDiffPtr ManagedSerializableDiff::create(const ManagedSerializableObjectPtr& oldObj, const ManagedSerializableObjectPtr& newObj)
	{
		assert(oldObj != nullptr && newObj != nullptr);

		ManagedSerializableObjectInfoPtr oldObjInfo = oldObj->getObjectInfo();
		ManagedSerializableObjectInfoPtr newObjInfo = newObj->getObjectInfo();

		ManagedSerializableDiffPtr output = bs_shared_ptr<ManagedSerializableDiff>();
		if (!oldObjInfo->mTypeInfo->matches(newObjInfo->mTypeInfo))
			return output;

		SPtr<ModifiedObject> modifications = output->generateDiff(oldObj, newObj);

		if (modifications != nullptr)
			output->mModificationRoot->entries = modifications->entries;

		return output;
	}

	SPtr<ManagedSerializableDiff::ModifiedObject> ManagedSerializableDiff::generateDiff
		(const ManagedSerializableObjectPtr& oldObj, const ManagedSerializableObjectPtr& newObj)
	{
		SPtr<ModifiedObject> output = nullptr;

		ManagedSerializableObjectInfoPtr curObjInfo = newObj->getObjectInfo();
		while (curObjInfo != nullptr)
		{
			for (auto& field : curObjInfo->mFields)
			{
				UINT32 fieldTypeId = field.second->mTypeInfo->getTypeId();

				ManagedSerializableFieldDataPtr oldData = oldObj->getFieldData(field.second);
				ManagedSerializableFieldDataPtr newData = newObj->getFieldData(field.second);
				SPtr<Modification> newMod = generateDiff(oldData, newData, fieldTypeId);
				
				if (newMod != nullptr)
				{
					if (output == nullptr)
						output = ModifiedObject::create();

					output->entries.push_back(ModifiedField(curObjInfo->mTypeInfo, field.second, newMod));
				}
			}

			curObjInfo = curObjInfo->mBaseClass;
		}

		return output;
	}

	SPtr<ManagedSerializableDiff::Modification> ManagedSerializableDiff::generateDiff(
		const ManagedSerializableFieldDataPtr& oldData, const ManagedSerializableFieldDataPtr& newData,
		UINT32 entryTypeId)
	{
		bool isPrimitive = entryTypeId == TID_SerializableTypeInfoPrimitive;

		SPtr<Modification> newMod = nullptr;
		if (isPrimitive)
		{
			if (!oldData->equals(newData))
				newMod = ModifiedEntry::create(newData);
		}
		else
		{
			switch (entryTypeId)
			{
			case TID_SerializableTypeInfoObject:
			{
				SPtr<ManagedSerializableFieldDataObject> oldObjData =
					std::static_pointer_cast<ManagedSerializableFieldDataObject>(oldData);
				SPtr<ManagedSerializableFieldDataObject> newObjData =
					std::static_pointer_cast<ManagedSerializableFieldDataObject>(newData);

				if (oldObjData->value != nullptr && newObjData->value != nullptr)
				{
					newMod = generateDiff(oldObjData->value, newObjData->value);
				}
				else // We either record null if new value is null, or the entire object if old value is null
				{
					newMod = ModifiedEntry::create(newData);
				}
			}
				break;
			case TID_SerializableTypeInfoArray:
			{
				SPtr<ManagedSerializableFieldDataArray> oldArrayData =
					std::static_pointer_cast<ManagedSerializableFieldDataArray>(oldData);
				SPtr<ManagedSerializableFieldDataArray> newArrayData =
					std::static_pointer_cast<ManagedSerializableFieldDataArray>(newData);

				if (oldArrayData->value != nullptr && newArrayData->value != nullptr)
				{
					UINT32 oldLength = oldArrayData->value->getTotalLength();
					UINT32 newLength = newArrayData->value->getTotalLength();

					SPtr<ModifiedArray> arrayMods = nullptr;
					for (UINT32 i = 0; i < newLength; i++)
					{
						SPtr<Modification> arrayElemMod = nullptr;

						ManagedSerializableFieldDataPtr newArrayElem = newArrayData->value->getFieldData(i);
						if (i < oldLength)
						{
							ManagedSerializableFieldDataPtr oldArrayElem = oldArrayData->value->getFieldData(i);

							UINT32 arrayElemTypeId = newArrayData->value->getTypeInfo()->mElementType->getTypeId();
							arrayElemMod = generateDiff(oldArrayElem, newArrayElem, arrayElemTypeId);
						}
						else
						{
							arrayElemMod = ModifiedEntry::create(newArrayElem);
						}

						if (arrayElemMod != nullptr)
						{
							if (arrayMods == nullptr)
							{
								arrayMods = ModifiedArray::create();
								arrayMods->origSizes = oldArrayData->value->getLengths();
								arrayMods->newSizes = newArrayData->value->getLengths();
							}

							arrayMods->entries.push_back(ModifiedArrayEntry(i, arrayElemMod));
						}
					}

					newMod = arrayMods;
				}
				else // We either record null if new value is null, or the entire array if old value is null
				{
					newMod = ModifiedEntry::create(newData);
				}
			}
				break;
			case TID_SerializableTypeInfoList:
			{
				SPtr<ManagedSerializableFieldDataList> oldListData =
					std::static_pointer_cast<ManagedSerializableFieldDataList>(oldData);
				SPtr<ManagedSerializableFieldDataList> newListData =
					std::static_pointer_cast<ManagedSerializableFieldDataList>(newData);

				if (oldListData->value != nullptr && newListData->value != nullptr)
				{
					UINT32 oldLength = oldListData->value->getLength();
					UINT32 newLength = newListData->value->getLength();

					SPtr<ModifiedArray> listMods = nullptr;
					for (UINT32 i = 0; i < newLength; i++)
					{
						SPtr<Modification> listElemMod = nullptr;

						ManagedSerializableFieldDataPtr newListElem = newListData->value->getFieldData(i);
						if (i < oldLength)
						{
							ManagedSerializableFieldDataPtr oldListElem = oldListData->value->getFieldData(i);

							UINT32 arrayElemTypeId = newListData->value->getTypeInfo()->mElementType->getTypeId();
							listElemMod = generateDiff(oldListElem, newListElem, arrayElemTypeId);
						}
						else
						{
							listElemMod = ModifiedEntry::create(newListElem);
						}

						if (listElemMod != nullptr)
						{
							if (listMods == nullptr)
							{
								listMods = ModifiedArray::create();
								listMods->origSizes.push_back(oldLength);
								listMods->newSizes.push_back(newLength);
							}

							listMods->entries.push_back(ModifiedArrayEntry(i, listElemMod));
						}
					}

					newMod = listMods;
				}
				else // We either record null if new value is null, or the entire list if old value is null
				{
					newMod = ModifiedEntry::create(newData);
				}
			}
				break;
			case TID_SerializableTypeInfoDictionary:
			{
				SPtr<ManagedSerializableFieldDataDictionary> oldDictData =
					std::static_pointer_cast<ManagedSerializableFieldDataDictionary>(oldData);
				SPtr<ManagedSerializableFieldDataDictionary> newDictData =
					std::static_pointer_cast<ManagedSerializableFieldDataDictionary>(newData);

				if (oldDictData->value != nullptr && newDictData->value != nullptr)
				{
					SPtr<ModifiedDictionary> dictMods = nullptr;

					auto newEnumerator = newDictData->value->getEnumerator();
					while (newEnumerator.moveNext())
					{
						SPtr<Modification> dictElemMod = nullptr;

						ManagedSerializableFieldDataPtr key = newEnumerator.getKey();
						if (oldDictData->value->contains(key))
						{
							UINT32 dictElemTypeId = newDictData->value->getTypeInfo()->mValueType->getTypeId();

							dictElemMod = generateDiff(oldDictData->value->getFieldData(key), 
								newEnumerator.getValue(), dictElemTypeId);
						}
						else
						{
							dictElemMod = ModifiedEntry::create(newEnumerator.getValue());
						}

						if (dictElemMod != nullptr)
						{
							if (dictMods == nullptr)
								dictMods = ModifiedDictionary::create();

							dictMods->entries.push_back(ModifiedDictionaryEntry(key, dictElemMod));
						}
					}

					auto oldEnumerator = oldDictData->value->getEnumerator();
					while (oldEnumerator.moveNext())
					{
						ManagedSerializableFieldDataPtr key = oldEnumerator.getKey();
						if (!newDictData->value->contains(oldEnumerator.getKey()))
						{
							if (dictMods == nullptr)
								dictMods = ModifiedDictionary::create();

							dictMods->removed.push_back(key);
						}
					}
				}
				else // We either record null if new value is null, or the entire dictionary if old value is null
				{
					newMod = ModifiedEntry::create(newData);
				}
			}
				break;
			}
		}

		return newMod;
	}

	void ManagedSerializableDiff::apply(const ManagedSerializableObjectPtr& obj)
	{
		applyDiff(mModificationRoot, obj);
	}

	void ManagedSerializableDiff::applyDiff(const SPtr<ModifiedObject>& mod, const ManagedSerializableObjectPtr& obj)
	{
		ManagedSerializableObjectInfoPtr objInfo = obj->getObjectInfo();
		for (auto& modEntry : mod->entries)
		{
			ManagedSerializableFieldInfoPtr fieldType = modEntry.fieldType;
			ManagedSerializableTypeInfoPtr typeInfo = modEntry.parentType;

			ManagedSerializableFieldInfoPtr matchingFieldInfo = objInfo->findMatchingField(fieldType, typeInfo);
			if (matchingFieldInfo == nullptr)
				continue; // Field no longer exists in the type

			ManagedSerializableFieldDataPtr origData = obj->getFieldData(matchingFieldInfo);

			ManagedSerializableFieldDataPtr newData = applyDiff(modEntry.modification, matchingFieldInfo->mTypeInfo, origData);
			if (newData != nullptr)
				obj->setFieldData(matchingFieldInfo, newData);
		}
	}

	void ManagedSerializableDiff::applyDiff(const SPtr<ModifiedArray>& mod, const ManagedSerializableArrayPtr& obj)
	{
		obj->resize(mod->newSizes);

		for (auto& modEntry : mod->entries)
		{
			UINT32 arrayIdx = modEntry.idx;

			ManagedSerializableFieldDataPtr origData = obj->getFieldData(arrayIdx);
			ManagedSerializableFieldDataPtr newData = applyDiff(mod, obj->getTypeInfo()->mElementType, origData);

			if (newData != nullptr)
				obj->setFieldData(arrayIdx, newData);
		}
	}

	void ManagedSerializableDiff::applyDiff(const SPtr<ModifiedArray>& mod, const ManagedSerializableListPtr& obj)
	{
		obj->resize(mod->newSizes[0]);

		for (auto& modEntry : mod->entries)
		{
			UINT32 arrayIdx = modEntry.idx;

			ManagedSerializableFieldDataPtr origData = obj->getFieldData(arrayIdx);
			ManagedSerializableFieldDataPtr newData = applyDiff(mod, obj->getTypeInfo()->mElementType, origData);

			if (newData != nullptr)
				obj->setFieldData(arrayIdx, newData);
		}
	}

	void ManagedSerializableDiff::applyDiff(const SPtr<ModifiedDictionary>& mod, const ManagedSerializableDictionaryPtr& obj)
	{
		for (auto& modEntry : mod->entries)
		{
			ManagedSerializableFieldDataPtr key = modEntry.key;

			ManagedSerializableFieldDataPtr origData = obj->getFieldData(key);
			ManagedSerializableFieldDataPtr newData = applyDiff(mod, obj->getTypeInfo()->mValueType, origData);

			if (newData != nullptr)
				obj->setFieldData(key, newData);
		}

		for (auto& key : mod->removed)
		{
			obj->removeFieldData(key);
		}
	}

	ManagedSerializableFieldDataPtr ManagedSerializableDiff::applyDiff(const SPtr<Modification>& mod, const ManagedSerializableTypeInfoPtr& fieldType,
		const ManagedSerializableFieldDataPtr& origData)
	{
		ManagedSerializableFieldDataPtr newData;
		switch (origData->getTypeId())
		{
		case TID_SerializableFieldDataObject:
		{
			SPtr<ManagedSerializableFieldDataObject> origObjData = std::static_pointer_cast<ManagedSerializableFieldDataObject>(origData);
			ManagedSerializableObjectPtr childObj = origObjData->value;

			ManagedSerializableTypeInfoObjectPtr objTypeInfo =
				std::static_pointer_cast<ManagedSerializableTypeInfoObject>(fieldType);

			if (childObj == nullptr) // Object was deleted in original but we have modifications for it, so we create it
			{
				childObj = ManagedSerializableObject::createNew(objTypeInfo);
				newData = ManagedSerializableFieldData::create(objTypeInfo, childObj->getManagedInstance());
			}

			SPtr<ModifiedObject> childMod = std::static_pointer_cast<ModifiedObject>(mod);
			applyDiff(childMod, childObj);
		}
			break;
		case TID_SerializableFieldDataArray:
		{
			SPtr<ManagedSerializableFieldDataArray> origArrayData = std::static_pointer_cast<ManagedSerializableFieldDataArray>(origData);
			ManagedSerializableArrayPtr childArray = origArrayData->value;

			ManagedSerializableTypeInfoArrayPtr arrayTypeInfo =
				std::static_pointer_cast<ManagedSerializableTypeInfoArray>(fieldType);

			SPtr<ModifiedArray> childMod = std::static_pointer_cast<ModifiedArray>(mod);
			if (childArray == nullptr) // Object was deleted in original but we have modifications for it, so we create it
			{
				childArray = ManagedSerializableArray::createNew(arrayTypeInfo, childMod->origSizes);
				newData = ManagedSerializableFieldData::create(arrayTypeInfo, childArray->getManagedInstance());
			}

			applyDiff(childMod, childArray);
		}
			break;
		case TID_SerializableFieldDataList:
		{
			SPtr<ManagedSerializableFieldDataList> origListData = std::static_pointer_cast<ManagedSerializableFieldDataList>(origData);
			ManagedSerializableListPtr childList = origListData->value;

			ManagedSerializableTypeInfoListPtr listTypeInfo =
				std::static_pointer_cast<ManagedSerializableTypeInfoList>(fieldType);

			SPtr<ModifiedArray> childMod = std::static_pointer_cast<ModifiedArray>(mod);
			if (childList == nullptr) // Object was deleted in original but we have modifications for it, so we create it
			{
				childList = ManagedSerializableList::createNew(listTypeInfo, childMod->origSizes[0]);
				newData = ManagedSerializableFieldData::create(listTypeInfo, childList->getManagedInstance());
			}

			applyDiff(childMod, childList);
		}
			break;
		case TID_SerializableFieldDataDictionary:
		{
			SPtr<ManagedSerializableFieldDataDictionary> origObjData = std::static_pointer_cast<ManagedSerializableFieldDataDictionary>(origData);
			ManagedSerializableDictionaryPtr childDict = origObjData->value;

			ManagedSerializableTypeInfoDictionaryPtr dictTypeInfo =
				std::static_pointer_cast<ManagedSerializableTypeInfoDictionary>(fieldType);

			if (childDict == nullptr) // Object was deleted in original but we have modifications for it, so we create it
			{
				childDict = ManagedSerializableDictionary::createNew(dictTypeInfo);
				newData = ManagedSerializableFieldData::create(dictTypeInfo, childDict->getManagedInstance());
			}

			SPtr<ModifiedDictionary> childMod = std::static_pointer_cast<ModifiedDictionary>(mod);
			applyDiff(childMod, childDict);
		}
			break;
		default: // Primitive field
		{
			SPtr<ModifiedEntry> childMod = std::static_pointer_cast<ModifiedEntry>(mod);
			newData = childMod->value;
		}
			break;
		}

		return newData;
	}

	RTTITypeBase* ManagedSerializableDiff::getRTTIStatic()
	{
		return ManagedSerializableDiffRTTI::instance();
	}

	RTTITypeBase* ManagedSerializableDiff::getRTTI() const
	{
		return ManagedSerializableDiff::getRTTIStatic();
	}
}