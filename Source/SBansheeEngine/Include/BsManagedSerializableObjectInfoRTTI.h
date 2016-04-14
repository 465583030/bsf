//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "BsRTTIType.h"
#include "BsManagedSerializableObjectInfo.h"
#include "BsException.h"

namespace BansheeEngine
{
	/** @cond RTTI */
	/** @addtogroup RTTI-Impl-SEngine
	 *  @{
	 */

	class BS_SCR_BE_EXPORT ManagedSerializableAssemblyInfoRTTI : public RTTIType<ManagedSerializableAssemblyInfo, IReflectable, ManagedSerializableAssemblyInfoRTTI>
	{
	private:
		String& getName(ManagedSerializableAssemblyInfo* obj)
		{
			return obj->mName;
		}

		void setName(ManagedSerializableAssemblyInfo* obj, String& val)
		{
			obj->mName = val;
		}

		SPtr<ManagedSerializableObjectInfo> getSerializableObjectInfo(ManagedSerializableAssemblyInfo* obj, UINT32 idx) 
		{ 
			auto iter = obj->mObjectInfos.begin();
			for(UINT32 i = 0; i < idx; i++)
				iter++;

			return iter->second;
		}

		void setSerializableObjectInfo(ManagedSerializableAssemblyInfo* obj, UINT32 idx, SPtr<ManagedSerializableObjectInfo> val) 
		{ 
			obj->mTypeNameToId[val->getFullTypeName()] = val->mTypeInfo->mTypeId;
			obj->mObjectInfos[val->mTypeInfo->mTypeId] = val;
		}
		
		UINT32 getSerializableObjectInfoArraySize(ManagedSerializableAssemblyInfo* obj) { return (UINT32)obj->mObjectInfos.size(); }
		void setSerializableObjectInfoArraySize(ManagedSerializableAssemblyInfo* obj, UINT32 size) {  }

	public:
		ManagedSerializableAssemblyInfoRTTI()
		{
			addPlainField("mName", 0, &ManagedSerializableAssemblyInfoRTTI::getName, &ManagedSerializableAssemblyInfoRTTI::setName);
			addReflectablePtrArrayField("mObjectInfos", 1, &ManagedSerializableAssemblyInfoRTTI::getSerializableObjectInfo, 
				&ManagedSerializableAssemblyInfoRTTI::getSerializableObjectInfoArraySize, &ManagedSerializableAssemblyInfoRTTI::setSerializableObjectInfo, 
				&ManagedSerializableAssemblyInfoRTTI::setSerializableObjectInfoArraySize);
		}

		const String& getRTTIName() override
		{
			static String name = "ScriptSerializableAssemblyInfo";
			return name;
		}

		UINT32 getRTTIId() override
		{
			return TID_SerializableAssemblyInfo;
		}

		SPtr<IReflectable> newRTTIObject() override
		{
			return bs_shared_ptr_new<ManagedSerializableAssemblyInfo>();
		}
	};

	class BS_SCR_BE_EXPORT ManagedSerializableObjectInfoRTTI : public RTTIType<ManagedSerializableObjectInfo, IReflectable, ManagedSerializableObjectInfoRTTI>
	{
	private:
		using RTTIType<ManagedSerializableObjectInfo, IReflectable, ManagedSerializableObjectInfoRTTI>::getBaseClass;

		SPtr<ManagedSerializableTypeInfoObject> getTypeInfo(ManagedSerializableObjectInfo* obj)
		{
			return obj->mTypeInfo;
		}

		void setTypeInfo(ManagedSerializableObjectInfo* obj, SPtr<ManagedSerializableTypeInfoObject> val)
		{
			obj->mTypeInfo = val;
		}

		SPtr<ManagedSerializableObjectInfo> getBaseClass(ManagedSerializableObjectInfo* obj)
		{
			return obj->mBaseClass;
		}

		void setBaseClass(ManagedSerializableObjectInfo* obj, SPtr<ManagedSerializableObjectInfo> val)
		{
			obj->mBaseClass = val;
		}

		SPtr<ManagedSerializableFieldInfo> getSerializableFieldInfo(ManagedSerializableObjectInfo* obj, UINT32 idx) 
		{ 
			auto iter = obj->mFields.begin();
			for(UINT32 i = 0; i < idx; i++)
				iter++;

			return iter->second;
		}

		void setSerializableFieldInfo(ManagedSerializableObjectInfo* obj, UINT32 idx, SPtr<ManagedSerializableFieldInfo> val) 
		{ 
			obj->mFieldNameToId[val->mName] = val->mFieldId;
			obj->mFields[val->mFieldId] = val;
		}

		UINT32 getSerializableFieldInfoArraySize(ManagedSerializableObjectInfo* obj) { return (UINT32)obj->mFields.size(); }
		void setSerializableFieldInfoArraySize(ManagedSerializableObjectInfo* obj, UINT32 size) {  }

	public:
		ManagedSerializableObjectInfoRTTI()
		{
			addReflectablePtrField("mTypeInfo", 0, &ManagedSerializableObjectInfoRTTI::getTypeInfo, &ManagedSerializableObjectInfoRTTI::setTypeInfo);
			addReflectablePtrField("mBaseClass", 2, &ManagedSerializableObjectInfoRTTI::getBaseClass, &ManagedSerializableObjectInfoRTTI::setBaseClass);

			addReflectablePtrArrayField("mFields", 3, &ManagedSerializableObjectInfoRTTI::getSerializableFieldInfo, 
				&ManagedSerializableObjectInfoRTTI::getSerializableFieldInfoArraySize, &ManagedSerializableObjectInfoRTTI::setSerializableFieldInfo, 
				&ManagedSerializableObjectInfoRTTI::setSerializableFieldInfoArraySize);
		}

		const String& getRTTIName() override
		{
			static String name = "ScriptSerializableObjectInfo";
			return name;
		}

		UINT32 getRTTIId() override
		{
			return TID_SerializableObjectInfo;
		}

		SPtr<IReflectable> newRTTIObject() override
		{
			return bs_shared_ptr_new<ManagedSerializableObjectInfo>();
		}
	};

	class BS_SCR_BE_EXPORT ManagedSerializableFieldInfoRTTI : public RTTIType<ManagedSerializableFieldInfo, IReflectable, ManagedSerializableFieldInfoRTTI>
	{
	private:
		SPtr<ManagedSerializableTypeInfo> getTypeInfo(ManagedSerializableFieldInfo* obj)
		{
			return obj->mTypeInfo;
		}

		void setTypeInfo(ManagedSerializableFieldInfo* obj, SPtr<ManagedSerializableTypeInfo> val)
		{
			obj->mTypeInfo = val;
		}

		String& getName(ManagedSerializableFieldInfo* obj)
		{
			return obj->mName;
		}

		void setName(ManagedSerializableFieldInfo* obj, String& val)
		{
			obj->mName = val;
		}

		UINT32& getFieldId(ManagedSerializableFieldInfo* obj)
		{
			return obj->mFieldId;
		}

		void setFieldId(ManagedSerializableFieldInfo* obj, UINT32& val)
		{
			obj->mFieldId = val;
		}

		UINT32& getParentTypeId(ManagedSerializableFieldInfo* obj)
		{
			return obj->mParentTypeId;
		}

		void setParentTypeId(ManagedSerializableFieldInfo* obj, UINT32& val)
		{
			obj->mParentTypeId = val;
		}

		UINT32& getFlags(ManagedSerializableFieldInfo* obj)
		{
			return (UINT32&)obj->mFlags;
		}

		void setFlags(ManagedSerializableFieldInfo* obj, UINT32& val)
		{
			obj->mFlags = (ScriptFieldFlags)val;
		}

	public:
		ManagedSerializableFieldInfoRTTI()
		{
			addPlainField("mName", 0, &ManagedSerializableFieldInfoRTTI::getName, &ManagedSerializableFieldInfoRTTI::setName);
			addReflectablePtrField("mTypeInfo", 1, &ManagedSerializableFieldInfoRTTI::getTypeInfo, &ManagedSerializableFieldInfoRTTI::setTypeInfo);
			addPlainField("mFieldId", 2, &ManagedSerializableFieldInfoRTTI::getFieldId, &ManagedSerializableFieldInfoRTTI::setFieldId);
			addPlainField("mFlags", 3, &ManagedSerializableFieldInfoRTTI::getFlags, &ManagedSerializableFieldInfoRTTI::setFlags);
			addPlainField("mParentTypeId", 4, &ManagedSerializableFieldInfoRTTI::getParentTypeId, &ManagedSerializableFieldInfoRTTI::setParentTypeId);
		}

		const String& getRTTIName() override
		{
			static String name = "ScriptSerializableFieldInfo";
			return name;
		}

		UINT32 getRTTIId() override
		{
			return TID_SerializableFieldInfo;
		}

		SPtr<IReflectable> newRTTIObject() override
		{
			return bs_shared_ptr_new<ManagedSerializableFieldInfo>();
		}
	};

	class BS_SCR_BE_EXPORT ManagedSerializableTypeInfoRTTI : public RTTIType<ManagedSerializableTypeInfo, IReflectable, ManagedSerializableTypeInfoRTTI>
	{
	private:

	public:
		ManagedSerializableTypeInfoRTTI()
		{

		}

		const String& getRTTIName() override
		{
			static String name = "ScriptSerializableTypeInfo";
			return name;
		}

		UINT32 getRTTIId() override
		{
			return TID_SerializableTypeInfo;
		}

		SPtr<IReflectable> newRTTIObject() override
		{
			BS_EXCEPT(InvalidStateException, "Cannot instantiate an abstract class");
			return nullptr;
		}
	};

	class BS_SCR_BE_EXPORT ManagedSerializableTypeInfoPrimitiveRTTI : public RTTIType<ManagedSerializableTypeInfoPrimitive, ManagedSerializableTypeInfo, ManagedSerializableTypeInfoPrimitiveRTTI>
	{
	private:
		ScriptPrimitiveType& getType(ManagedSerializableTypeInfoPrimitive* obj)
		{
			return obj->mType;
		}

		void setType(ManagedSerializableTypeInfoPrimitive* obj, ScriptPrimitiveType& val)
		{
			obj->mType = val;
		}

	public:
		ManagedSerializableTypeInfoPrimitiveRTTI()
		{
			addPlainField("mType", 0, &ManagedSerializableTypeInfoPrimitiveRTTI::getType, &ManagedSerializableTypeInfoPrimitiveRTTI::setType);
		}

		const String& getRTTIName() override
		{
			static String name = "ScriptSerializableTypeInfoPrimitive";
			return name;
		}

		UINT32 getRTTIId() override
		{
			return TID_SerializableTypeInfoPrimitive;
		}

		SPtr<IReflectable> newRTTIObject() override
		{
			return bs_shared_ptr_new<ManagedSerializableTypeInfoPrimitive>();
		}
	};

	class BS_SCR_BE_EXPORT ManagedSerializableTypeInfoRefRTTI : public RTTIType<ManagedSerializableTypeInfoRef, ManagedSerializableTypeInfo, ManagedSerializableTypeInfoRefRTTI>
	{
	private:
		ScriptReferenceType& getType(ManagedSerializableTypeInfoRef* obj)
		{
			return obj->mType;
		}

		void setType(ManagedSerializableTypeInfoRef* obj, ScriptReferenceType& val)
		{
			obj->mType = val;
		}

		String& getTypeNamespace(ManagedSerializableTypeInfoRef* obj)
		{
			return obj->mTypeNamespace;
		}

		void setTypeNamespace(ManagedSerializableTypeInfoRef* obj, String& val)
		{
			obj->mTypeNamespace = val;
		}

		String& getTypeName(ManagedSerializableTypeInfoRef* obj)
		{
			return obj->mTypeName;
		}

		void setTypeName(ManagedSerializableTypeInfoRef* obj, String& val)
		{
			obj->mTypeName = val;
		}

	public:
		ManagedSerializableTypeInfoRefRTTI()
		{
			addPlainField("mType", 0, &ManagedSerializableTypeInfoRefRTTI::getType, &ManagedSerializableTypeInfoRefRTTI::setType);
			addPlainField("mTypeName", 1, &ManagedSerializableTypeInfoRefRTTI::getTypeName, &ManagedSerializableTypeInfoRefRTTI::setTypeName);
			addPlainField("mTypeNamespace", 2, &ManagedSerializableTypeInfoRefRTTI::getTypeNamespace, &ManagedSerializableTypeInfoRefRTTI::setTypeNamespace);
		}

		const String& getRTTIName() override
		{
			static String name = "ScriptSerializableTypeInfoRef";
			return name;
		}

		UINT32 getRTTIId() override
		{
			return TID_SerializableTypeInfoRef;
		}

		SPtr<IReflectable> newRTTIObject() override
		{
			return bs_shared_ptr_new<ManagedSerializableTypeInfoRef>();
		}
	};

	class BS_SCR_BE_EXPORT ManagedSerializableTypeInfoObjectRTTI : public RTTIType<ManagedSerializableTypeInfoObject, ManagedSerializableTypeInfo, ManagedSerializableTypeInfoObjectRTTI>
	{
	private:
		String& getTypeNamespace(ManagedSerializableTypeInfoObject* obj)
		{
			return obj->mTypeNamespace;
		}

		void setTypeNamespace(ManagedSerializableTypeInfoObject* obj, String& val)
		{
			obj->mTypeNamespace = val;
		}

		String& getTypeName(ManagedSerializableTypeInfoObject* obj)
		{
			return obj->mTypeName;
		}

		void setTypeName(ManagedSerializableTypeInfoObject* obj, String& val)
		{
			obj->mTypeName = val;
		}

		bool& getIsValueType(ManagedSerializableTypeInfoObject* obj)
		{
			return obj->mValueType;
		}

		void setIsValueType(ManagedSerializableTypeInfoObject* obj, bool& val)
		{
			obj->mValueType = val;
		}

		UINT32& getTypeId(ManagedSerializableTypeInfoObject* obj)
		{
			return obj->mTypeId;
		}

		void setTypeId(ManagedSerializableTypeInfoObject* obj, UINT32& val)
		{
			obj->mTypeId = val;
		}

	public:
		ManagedSerializableTypeInfoObjectRTTI()
		{
			addPlainField("mTypeName", 0, &ManagedSerializableTypeInfoObjectRTTI::getTypeName, &ManagedSerializableTypeInfoObjectRTTI::setTypeName);
			addPlainField("mTypeNamespace", 1, &ManagedSerializableTypeInfoObjectRTTI::getTypeNamespace, &ManagedSerializableTypeInfoObjectRTTI::setTypeNamespace);
			addPlainField("mValueType", 2, &ManagedSerializableTypeInfoObjectRTTI::getIsValueType, &ManagedSerializableTypeInfoObjectRTTI::setIsValueType);
			addPlainField("mTypeId", 3, &ManagedSerializableTypeInfoObjectRTTI::getIsValueType, &ManagedSerializableTypeInfoObjectRTTI::setIsValueType);
		}

		const String& getRTTIName() override
		{
			static String name = "ScriptSerializableTypeInfoObject";
			return name;
		}

		UINT32 getRTTIId() override
		{
			return TID_SerializableTypeInfoObject;
		}

		SPtr<IReflectable> newRTTIObject() override
		{
			return bs_shared_ptr_new<ManagedSerializableTypeInfoObject>();
		}
	};

	class BS_SCR_BE_EXPORT ManagedSerializableTypeInfoArrayRTTI : public RTTIType<ManagedSerializableTypeInfoArray, ManagedSerializableTypeInfo, ManagedSerializableTypeInfoArrayRTTI>
	{
	private:
		SPtr<ManagedSerializableTypeInfo> getElementType(ManagedSerializableTypeInfoArray* obj)
		{
			return obj->mElementType;
		}

		void setElementType(ManagedSerializableTypeInfoArray* obj, SPtr<ManagedSerializableTypeInfo> val)
		{
			obj->mElementType = val;
		}

		UINT32& getRank(ManagedSerializableTypeInfoArray* obj)
		{
			return obj->mRank;
		}

		void setRank(ManagedSerializableTypeInfoArray* obj, UINT32& val)
		{
			obj->mRank = val;
		}

	public:
		ManagedSerializableTypeInfoArrayRTTI()
		{
			addReflectablePtrField("mElementType", 0, &ManagedSerializableTypeInfoArrayRTTI::getElementType, &ManagedSerializableTypeInfoArrayRTTI::setElementType);
			addPlainField("mRank", 1, &ManagedSerializableTypeInfoArrayRTTI::getRank, &ManagedSerializableTypeInfoArrayRTTI::setRank);
		}

		const String& getRTTIName() override
		{
			static String name = "ScriptSerializableTypeInfoArray";
			return name;
		}

		UINT32 getRTTIId() override
		{
			return TID_SerializableTypeInfoArray;
		}

		SPtr<IReflectable> newRTTIObject() override
		{
			return bs_shared_ptr_new<ManagedSerializableTypeInfoArray>();
		}
	};

	class BS_SCR_BE_EXPORT ManagedSerializableTypeInfoListRTTI : public RTTIType<ManagedSerializableTypeInfoList, ManagedSerializableTypeInfo, ManagedSerializableTypeInfoListRTTI>
	{
	private:
		SPtr<ManagedSerializableTypeInfo> getElementType(ManagedSerializableTypeInfoList* obj)
		{
			return obj->mElementType;
		}

		void setElementType(ManagedSerializableTypeInfoList* obj, SPtr<ManagedSerializableTypeInfo> val)
		{
			obj->mElementType = val;
		}

	public:
		ManagedSerializableTypeInfoListRTTI()
		{
			addReflectablePtrField("mElementType", 0, &ManagedSerializableTypeInfoListRTTI::getElementType, &ManagedSerializableTypeInfoListRTTI::setElementType);
		}

		const String& getRTTIName() override
		{
			static String name = "ScriptSerializableTypeInfoList";
			return name;
		}

		UINT32 getRTTIId() override
		{
			return TID_SerializableTypeInfoList;
		}

		SPtr<IReflectable> newRTTIObject() override
		{
			return bs_shared_ptr_new<ManagedSerializableTypeInfoList>();
		}
	};

	class BS_SCR_BE_EXPORT ManagedSerializableTypeInfoDictionaryRTTI : public RTTIType<ManagedSerializableTypeInfoDictionary, ManagedSerializableTypeInfo, ManagedSerializableTypeInfoDictionaryRTTI>
	{
	private:
		SPtr<ManagedSerializableTypeInfo> getKeyType(ManagedSerializableTypeInfoDictionary* obj)
		{
			return obj->mKeyType;
		}

		void setKeyType(ManagedSerializableTypeInfoDictionary* obj, SPtr<ManagedSerializableTypeInfo> val)
		{
			obj->mKeyType = val;
		}

		SPtr<ManagedSerializableTypeInfo> getValueType(ManagedSerializableTypeInfoDictionary* obj)
		{
			return obj->mValueType;
		}

		void setValueType(ManagedSerializableTypeInfoDictionary* obj, SPtr<ManagedSerializableTypeInfo> val)
		{
			obj->mValueType = val;
		}

	public:
		ManagedSerializableTypeInfoDictionaryRTTI()
		{
			addReflectablePtrField("mKeyType", 0, &ManagedSerializableTypeInfoDictionaryRTTI::getKeyType, &ManagedSerializableTypeInfoDictionaryRTTI::setKeyType);
			addReflectablePtrField("mValueType", 1, &ManagedSerializableTypeInfoDictionaryRTTI::getValueType, &ManagedSerializableTypeInfoDictionaryRTTI::setValueType);
		}

		const String& getRTTIName() override
		{
			static String name = "ScriptSerializableTypeInfoDictionary";
			return name;
		}

		UINT32 getRTTIId() override
		{
			return TID_SerializableTypeInfoDictionary;
		}

		SPtr<IReflectable> newRTTIObject() override
		{
			return bs_shared_ptr_new<ManagedSerializableTypeInfoDictionary>();
		}
	};

	/** @} */
	/** @endcond */
}