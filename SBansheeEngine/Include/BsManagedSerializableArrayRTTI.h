#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "BsRTTIType.h"
#include "BsGameObjectManager.h"
#include "BsManagedSerializableArray.h"
#include "BsScriptAssemblyManager.h"
#include "BsMonoManager.h"
#include "BsMonoClass.h"

namespace BansheeEngine
{
	class BS_SCR_BE_EXPORT ManagedSerializableArrayRTTI : public RTTIType<ManagedSerializableArray, IReflectable, ManagedSerializableArrayRTTI>
	{
	private:
		ManagedSerializableTypeInfoArrayPtr getTypeInfo(ManagedSerializableArray* obj)
		{
			return obj->mArrayTypeInfo;
		}

		void setTypeInfo(ManagedSerializableArray* obj, ManagedSerializableTypeInfoArrayPtr val)
		{
			obj->mArrayTypeInfo = val;
		}

		UINT32& getElementSize(ManagedSerializableArray* obj)
		{
			return (UINT32&)obj->mElemSize;
		}

		void setElementSize(ManagedSerializableArray* obj, UINT32& numElements)
		{
			obj->mElemSize = numElements;
		}

		UINT32& getNumElements(ManagedSerializableArray* obj, UINT32 arrayIdx)
		{
			return (UINT32&)obj->mNumElements[arrayIdx];
		}

		void setNumElements(ManagedSerializableArray* obj, UINT32 arrayIdx, UINT32& numElements)
		{
			obj->mNumElements[arrayIdx] = numElements;
		}

		UINT32 getNumElementsNumEntries(ManagedSerializableArray* obj)
		{
			return (UINT32)obj->mNumElements.size();
		}

		void setNumElementsNumEntries(ManagedSerializableArray* obj, UINT32 numEntries)
		{
			obj->mNumElements.resize(numEntries);
		}

		ManagedSerializableFieldDataPtr getArrayEntry(ManagedSerializableArray* obj, UINT32 arrayIdx)
		{
			return obj->getFieldData(arrayIdx);
		}

		void setArrayEntry(ManagedSerializableArray* obj, UINT32 arrayIdx, ManagedSerializableFieldDataPtr val)
		{
			obj->setFieldData(arrayIdx, val);
		}

		UINT32 getNumArrayEntries(ManagedSerializableArray* obj)
		{
			return obj->getTotalLength();
		}

		void setNumArrayEntries(ManagedSerializableArray* obj, UINT32 numEntries)
		{
			obj->mCachedEntries = Vector<ManagedSerializableFieldDataPtr>(numEntries);
		}

	public:
		ManagedSerializableArrayRTTI()
		{
			addReflectablePtrField("mArrayTypeInfo", 0, &ManagedSerializableArrayRTTI::getTypeInfo, &ManagedSerializableArrayRTTI::setTypeInfo);
			addPlainField("mElementSize", 1, &ManagedSerializableArrayRTTI::getElementSize, &ManagedSerializableArrayRTTI::setElementSize);
			addPlainArrayField("mNumElements", 2, &ManagedSerializableArrayRTTI::getNumElements, &ManagedSerializableArrayRTTI::getNumElementsNumEntries, 
				&ManagedSerializableArrayRTTI::setNumElements, &ManagedSerializableArrayRTTI::setNumElementsNumEntries);
			addReflectablePtrArrayField("mArrayEntries", 3, &ManagedSerializableArrayRTTI::getArrayEntry, &ManagedSerializableArrayRTTI::getNumArrayEntries, 
				&ManagedSerializableArrayRTTI::setArrayEntry, &ManagedSerializableArrayRTTI::setNumArrayEntries);
		}

		virtual const String& getRTTIName()
		{
			static String name = "ScriptSerializableArray";
			return name;
		}

		virtual UINT32 getRTTIId()
		{
			return TID_ScriptSerializableArray;
		}

		virtual std::shared_ptr<IReflectable> newRTTIObject()
		{
			return ManagedSerializableArray::createNew();
		}
	};
}