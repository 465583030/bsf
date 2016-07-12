//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsScriptAssemblyManager.h"
#include "BsManagedSerializableObjectInfo.h"
#include "BsMonoManager.h"
#include "BsMonoAssembly.h"
#include "BsMonoClass.h"
#include "BsMonoField.h"
#include "BsMonoMethod.h"
#include "BsMonoProperty.h"
#include "BsScriptManagedResource.h"
#include "BsScriptTexture2D.h"
#include "BsScriptTexture3D.h"
#include "BsScriptTextureCube.h"
#include "BsScriptSpriteTexture.h"
#include "BsScriptMaterial.h"
#include "BsScriptMesh.h"
#include "BsScriptFont.h"
#include "BsScriptShader.h"
#include "BsScriptShaderInclude.h"
#include "BsScriptPlainText.h"
#include "BsScriptScriptCode.h"
#include "BsScriptStringTable.h"
#include "BsScriptGUISkin.h"
#include "BsScriptPhysicsMaterial.h"
#include "BsScriptPhysicsMesh.h"
#include "BsScriptAudioClip.h"
#include "BsScriptPrefab.h"

namespace BansheeEngine
{
	ScriptAssemblyManager::ScriptAssemblyManager()
		: mBaseTypesInitialized(false), mSystemArrayClass(nullptr), mSystemGenericListClass(nullptr)
		, mSystemGenericDictionaryClass(nullptr), mSystemTypeClass(nullptr), mComponentClass(nullptr)
		, mSceneObjectClass(nullptr), mMissingComponentClass(nullptr), mSerializeObjectAttribute(nullptr)
		, mDontSerializeFieldAttribute(nullptr), mSerializeFieldAttribute(nullptr), mHideInInspectorAttribute(nullptr)
		, mRangeAttribute(nullptr), mStepAttribute(nullptr)
	{

	}

	ScriptAssemblyManager::~ScriptAssemblyManager()
	{

	}

	Vector<String> ScriptAssemblyManager::getScriptAssemblies() const
	{
		Vector<String> initializedAssemblies;
		for (auto& assemblyPair : mAssemblyInfos)
			initializedAssemblies.push_back(assemblyPair.first);

		return initializedAssemblies;
	}

	void ScriptAssemblyManager::loadAssemblyInfo(const String& assemblyName)
	{
		if(!mBaseTypesInitialized)
			initializeBaseTypes();

		// Process all classes and fields
		UINT32 mUniqueTypeId = 1;

		MonoAssembly* curAssembly = MonoManager::instance().getAssembly(assemblyName);
		if(curAssembly == nullptr)
			return;

		SPtr<ManagedSerializableAssemblyInfo> assemblyInfo = bs_shared_ptr_new<ManagedSerializableAssemblyInfo>();
		assemblyInfo->mName = assemblyName;

		mAssemblyInfos[assemblyName] = assemblyInfo;

		MonoClass* managedResourceClass = ScriptManagedResource::getMetaData()->scriptClass;

		// Populate class data
		const Vector<MonoClass*>& allClasses = curAssembly->getAllClasses();
		for(auto& curClass : allClasses)
		{
			if ((curClass->isSubClassOf(mComponentClass) || curClass->isSubClassOf(managedResourceClass) ||
				curClass->hasAttribute(mSerializeObjectAttribute)) && curClass != mComponentClass && curClass != managedResourceClass)
			{
				SPtr<ManagedSerializableTypeInfoObject> typeInfo = bs_shared_ptr_new<ManagedSerializableTypeInfoObject>();
				typeInfo->mTypeNamespace = curClass->getNamespace();
				typeInfo->mTypeName = curClass->getTypeName();
				typeInfo->mTypeId = mUniqueTypeId++;

				MonoPrimitiveType monoPrimitiveType = MonoUtil::getPrimitiveType(curClass->_getInternalClass());

				if(monoPrimitiveType == MonoPrimitiveType::ValueType)
					typeInfo->mValueType = true;
				else
					typeInfo->mValueType = false;

				SPtr<ManagedSerializableObjectInfo> objInfo = bs_shared_ptr_new<ManagedSerializableObjectInfo>();

				objInfo->mTypeInfo = typeInfo;
				objInfo->mMonoClass = curClass;

				assemblyInfo->mTypeNameToId[objInfo->getFullTypeName()] = typeInfo->mTypeId;
				assemblyInfo->mObjectInfos[typeInfo->mTypeId] = objInfo;
			}
		}

		// Populate field data
		for(auto& curClassInfo : assemblyInfo->mObjectInfos)
		{
			SPtr<ManagedSerializableObjectInfo> objInfo = curClassInfo.second;

			UINT32 mUniqueFieldId = 1;
			const Vector<MonoField*>& fields = objInfo->mMonoClass->getAllFields();

			for(auto& field : fields)
			{
				if(field->isStatic())
					continue;

				SPtr<ManagedSerializableTypeInfo> typeInfo = getTypeInfo(field->getType());
				if (typeInfo == nullptr)
					continue;

				SPtr<ManagedSerializableFieldInfo> fieldInfo = bs_shared_ptr_new<ManagedSerializableFieldInfo>();
				fieldInfo->mFieldId = mUniqueFieldId++;
				fieldInfo->mName = field->getName();
				fieldInfo->mMonoField = field;
				fieldInfo->mTypeInfo = typeInfo;
				fieldInfo->mParentTypeId = objInfo->mTypeInfo->mTypeId;
				
				MonoFieldVisibility visibility = field->getVisibility();
				if (visibility == MonoFieldVisibility::Public)
				{
					if (!field->hasAttribute(mDontSerializeFieldAttribute))
						fieldInfo->mFlags = (ScriptFieldFlags)((UINT32)fieldInfo->mFlags | (UINT32)ScriptFieldFlags::Serializable);

					if (!field->hasAttribute(mHideInInspectorAttribute))
						fieldInfo->mFlags = (ScriptFieldFlags)((UINT32)fieldInfo->mFlags | (UINT32)ScriptFieldFlags::Inspectable);
				}
				else
				{
					if (field->hasAttribute(mSerializeFieldAttribute))
						fieldInfo->mFlags = (ScriptFieldFlags)((UINT32)fieldInfo->mFlags | (UINT32)ScriptFieldFlags::Serializable);
				}
				if (field->hasAttribute(mRangeAttribute))
					fieldInfo->mFlags = (ScriptFieldFlags)((UINT32)fieldInfo->mFlags | (UINT32)ScriptFieldFlags::Range);
				if (field->hasAttribute(mStepAttribute))
					fieldInfo->mFlags = (ScriptFieldFlags)((UINT32)fieldInfo->mFlags | (UINT32)ScriptFieldFlags::Step);

				objInfo->mFieldNameToId[fieldInfo->mName] = fieldInfo->mFieldId;
				objInfo->mFields[fieldInfo->mFieldId] = fieldInfo;
			}
		}

		// Form parent/child connections
		for(auto& curClass : assemblyInfo->mObjectInfos)
		{
			MonoClass* base = curClass.second->mMonoClass->getBaseClass();
			while(base != nullptr)
			{
				SPtr<ManagedSerializableObjectInfo> baseObjInfo;
				if(getSerializableObjectInfo(base->getNamespace(), base->getTypeName(), baseObjInfo))
				{
					curClass.second->mBaseClass = baseObjInfo;
					baseObjInfo->mDerivedClasses.push_back(curClass.second);

					break;
				}

				base = base->getBaseClass();
			}
		}
	}

	void ScriptAssemblyManager::clearAssemblyInfo()
	{
		clearScriptObjects();
		mAssemblyInfos.clear();
	}

	SPtr<ManagedSerializableTypeInfo> ScriptAssemblyManager::getTypeInfo(MonoClass* monoClass)
	{
		if(!mBaseTypesInitialized)
			BS_EXCEPT(InvalidStateException, "Calling determineType without previously initializing base types.");

		MonoPrimitiveType monoPrimitiveType = MonoUtil::getPrimitiveType(monoClass->_getInternalClass());
		
		// If enum get the enum base data type
		bool isEnum = MonoUtil::isEnum(monoClass->_getInternalClass());
		if (isEnum)
			monoPrimitiveType = MonoUtil::getEnumPrimitiveType(monoClass->_getInternalClass());

		//  Determine field type
		switch(monoPrimitiveType)
		{
		case MonoPrimitiveType::Boolean:
			{
				SPtr<ManagedSerializableTypeInfoPrimitive> typeInfo = bs_shared_ptr_new<ManagedSerializableTypeInfoPrimitive>();
				typeInfo->mType = ScriptPrimitiveType::Bool;
				return typeInfo;
			}
		case MonoPrimitiveType::Char:
			{
				SPtr<ManagedSerializableTypeInfoPrimitive> typeInfo = bs_shared_ptr_new<ManagedSerializableTypeInfoPrimitive>();
				typeInfo->mType = ScriptPrimitiveType::Char;
				return typeInfo;
			}
		case MonoPrimitiveType::I8:
			{
				SPtr<ManagedSerializableTypeInfoPrimitive> typeInfo = bs_shared_ptr_new<ManagedSerializableTypeInfoPrimitive>();
				typeInfo->mType = ScriptPrimitiveType::I8;
				return typeInfo;
			}
		case MonoPrimitiveType::U8:
			{
				SPtr<ManagedSerializableTypeInfoPrimitive> typeInfo = bs_shared_ptr_new<ManagedSerializableTypeInfoPrimitive>();
				typeInfo->mType = ScriptPrimitiveType::U8;
				return typeInfo;
			}
		case MonoPrimitiveType::I16:
			{
				SPtr<ManagedSerializableTypeInfoPrimitive> typeInfo = bs_shared_ptr_new<ManagedSerializableTypeInfoPrimitive>();
				typeInfo->mType = ScriptPrimitiveType::I16;
				return typeInfo;
			}
		case MonoPrimitiveType::U16:
			{
				SPtr<ManagedSerializableTypeInfoPrimitive> typeInfo = bs_shared_ptr_new<ManagedSerializableTypeInfoPrimitive>();
				typeInfo->mType = ScriptPrimitiveType::U16;
				return typeInfo;
			}
		case MonoPrimitiveType::I32:
			{
				SPtr<ManagedSerializableTypeInfoPrimitive> typeInfo = bs_shared_ptr_new<ManagedSerializableTypeInfoPrimitive>();
				typeInfo->mType = ScriptPrimitiveType::I32;
				return typeInfo;
			}
		case MonoPrimitiveType::U32:
			{
				SPtr<ManagedSerializableTypeInfoPrimitive> typeInfo = bs_shared_ptr_new<ManagedSerializableTypeInfoPrimitive>();
				typeInfo->mType = ScriptPrimitiveType::U32;
				return typeInfo;
			}
		case MonoPrimitiveType::I64:
			{
				SPtr<ManagedSerializableTypeInfoPrimitive> typeInfo = bs_shared_ptr_new<ManagedSerializableTypeInfoPrimitive>();
				typeInfo->mType = ScriptPrimitiveType::I64;
				return typeInfo;
			}
		case MonoPrimitiveType::U64:
			{
				SPtr<ManagedSerializableTypeInfoPrimitive> typeInfo = bs_shared_ptr_new<ManagedSerializableTypeInfoPrimitive>();
				typeInfo->mType = ScriptPrimitiveType::U64;
				return typeInfo;
			}
		case MonoPrimitiveType::String:
			{
				SPtr<ManagedSerializableTypeInfoPrimitive> typeInfo = bs_shared_ptr_new<ManagedSerializableTypeInfoPrimitive>();
				typeInfo->mType = ScriptPrimitiveType::String;
				return typeInfo;
			}
		case MonoPrimitiveType::R32:
			{
				SPtr<ManagedSerializableTypeInfoPrimitive> typeInfo = bs_shared_ptr_new<ManagedSerializableTypeInfoPrimitive>();
				typeInfo->mType = ScriptPrimitiveType::Float;
				return typeInfo;
			}
		case MonoPrimitiveType::R64:
			{
				SPtr<ManagedSerializableTypeInfoPrimitive> typeInfo = bs_shared_ptr_new<ManagedSerializableTypeInfoPrimitive>();
				typeInfo->mType = ScriptPrimitiveType::Double;
				return typeInfo;
			}
		case MonoPrimitiveType::Class:
			if(monoClass->isSubClassOf(ScriptResource::getMetaData()->scriptClass)) // Resource
			{
				SPtr<ManagedSerializableTypeInfoRef> typeInfo = bs_shared_ptr_new<ManagedSerializableTypeInfoRef>();
				typeInfo->mTypeNamespace = monoClass->getNamespace();
				typeInfo->mTypeName = monoClass->getTypeName();

				if(monoClass == ScriptResource::getMetaData()->scriptClass)
					typeInfo->mType = ScriptReferenceType::Resource;
				else if (monoClass->isSubClassOf(ScriptTexture2D::getMetaData()->scriptClass))
					typeInfo->mType = ScriptReferenceType::Texture2D;
				else if (monoClass->isSubClassOf(ScriptTexture3D::getMetaData()->scriptClass))
					typeInfo->mType = ScriptReferenceType::Texture3D;
				else if (monoClass->isSubClassOf(ScriptTextureCube::getMetaData()->scriptClass))
					typeInfo->mType = ScriptReferenceType::TextureCube;
				else if (monoClass->isSubClassOf(ScriptSpriteTexture::getMetaData()->scriptClass))
					typeInfo->mType = ScriptReferenceType::SpriteTexture;
				else if (monoClass->isSubClassOf(ScriptManagedResource::getMetaData()->scriptClass))
					typeInfo->mType = ScriptReferenceType::ManagedResource;
				else if (monoClass->isSubClassOf(ScriptShader::getMetaData()->scriptClass))
					typeInfo->mType = ScriptReferenceType::Shader;
				else if (monoClass->isSubClassOf(ScriptShaderInclude::getMetaData()->scriptClass))
					typeInfo->mType = ScriptReferenceType::ShaderInclude;
				else if (monoClass->isSubClassOf(ScriptMaterial::getMetaData()->scriptClass))
					typeInfo->mType = ScriptReferenceType::Material;
				else if (monoClass->isSubClassOf(ScriptMesh::getMetaData()->scriptClass))
					typeInfo->mType = ScriptReferenceType::Mesh;
				else if (monoClass->isSubClassOf(ScriptPlainText::getMetaData()->scriptClass))
					typeInfo->mType = ScriptReferenceType::PlainText;
				else if (monoClass->isSubClassOf(ScriptScriptCode::getMetaData()->scriptClass))
					typeInfo->mType = ScriptReferenceType::ScriptCode;
				else if (monoClass->isSubClassOf(ScriptPrefab::getMetaData()->scriptClass))
					typeInfo->mType = ScriptReferenceType::Prefab;
				else if (monoClass->isSubClassOf(ScriptFont::getMetaData()->scriptClass))
					typeInfo->mType = ScriptReferenceType::Font;
				else if (monoClass->isSubClassOf(ScriptStringTable::getMetaData()->scriptClass))
					typeInfo->mType = ScriptReferenceType::StringTable;
				else if (monoClass->isSubClassOf(ScriptGUISkin::getMetaData()->scriptClass))
					typeInfo->mType = ScriptReferenceType::GUISkin;
				else if (monoClass->isSubClassOf(ScriptPhysicsMaterial::getMetaData()->scriptClass))
					typeInfo->mType = ScriptReferenceType::PhysicsMaterial;
				else if (monoClass->isSubClassOf(ScriptPhysicsMesh::getMetaData()->scriptClass))
					typeInfo->mType = ScriptReferenceType::PhysicsMesh;
				else if (monoClass->isSubClassOf(ScriptAudioClip::getMetaData()->scriptClass))
					typeInfo->mType = ScriptReferenceType::AudioClip;
				else
				{
					assert(false && "Unrecognized resource type");
				}

				return typeInfo;
			}
			else if (monoClass->isSubClassOf(mSceneObjectClass) || monoClass->isSubClassOf(mComponentClass)) // Game object
			{
				SPtr<ManagedSerializableTypeInfoRef> typeInfo = bs_shared_ptr_new<ManagedSerializableTypeInfoRef>();
				typeInfo->mTypeNamespace = monoClass->getNamespace();
				typeInfo->mTypeName = monoClass->getTypeName();

				if (monoClass == mComponentClass)
					typeInfo->mType = ScriptReferenceType::Component;
				else if (monoClass->isSubClassOf(mSceneObjectClass))
					typeInfo->mType = ScriptReferenceType::SceneObject;
				else if (monoClass->isSubClassOf(mComponentClass))
					typeInfo->mType = ScriptReferenceType::ManagedComponent;

				return typeInfo;
			}
			else
			{
				SPtr<ManagedSerializableObjectInfo> objInfo;
				if (getSerializableObjectInfo(monoClass->getNamespace(), monoClass->getTypeName(), objInfo))
					return objInfo->mTypeInfo;
			}

			break;
		case MonoPrimitiveType::ValueType:
			{
				SPtr<ManagedSerializableObjectInfo> objInfo;
				if (getSerializableObjectInfo(monoClass->getNamespace(), monoClass->getTypeName(), objInfo))
					return objInfo->mTypeInfo;
			}

			break;
		case MonoPrimitiveType::Generic:
			if(monoClass->getFullName() == mSystemGenericListClass->getFullName()) // Full name is part of CIL spec, so it is just fine to compare like this
			{
				SPtr<ManagedSerializableTypeInfoList> typeInfo = bs_shared_ptr_new<ManagedSerializableTypeInfoList>();

				MonoProperty& itemProperty = monoClass->getProperty("Item");
				MonoClass* itemClass = itemProperty.getReturnType();

				if (itemClass != nullptr)
					typeInfo->mElementType = getTypeInfo(itemClass);
				
				if (typeInfo->mElementType == nullptr)
					return nullptr;

				return typeInfo;
			}
			else if(monoClass->getFullName() == mSystemGenericDictionaryClass->getFullName())
			{
				SPtr<ManagedSerializableTypeInfoDictionary> typeInfo = bs_shared_ptr_new<ManagedSerializableTypeInfoDictionary>();

				MonoMethod* getEnumerator = monoClass->getMethod("GetEnumerator");
				MonoClass* enumClass = getEnumerator->getReturnType();

				MonoProperty& currentProp = enumClass->getProperty("Current");
				MonoClass* keyValuePair = currentProp.getReturnType();

				MonoProperty& keyProperty = keyValuePair->getProperty("Key");
				MonoProperty& valueProperty = keyValuePair->getProperty("Value");

				MonoClass* keyClass = keyProperty.getReturnType();
				if(keyClass != nullptr)
					typeInfo->mKeyType = getTypeInfo(keyClass);

				MonoClass* valueClass = valueProperty.getReturnType();
				if(valueClass != nullptr)
					typeInfo->mValueType = getTypeInfo(valueClass);

				if (typeInfo->mKeyType == nullptr || typeInfo->mValueType == nullptr)
					return nullptr;

				return typeInfo;
			}
			break;
		case MonoPrimitiveType::Array:
			{
				SPtr<ManagedSerializableTypeInfoArray> typeInfo = bs_shared_ptr_new<ManagedSerializableTypeInfoArray>();

				::MonoClass* elementClass = ScriptArray::getElementClass(monoClass->_getInternalClass());
				if(elementClass != nullptr)
				{
					MonoClass* monoElementClass = MonoManager::instance().findClass(elementClass);
					if(monoElementClass != nullptr)
						typeInfo->mElementType = getTypeInfo(monoElementClass);
				}

				if (typeInfo->mElementType == nullptr)
					return nullptr;

				typeInfo->mRank = ScriptArray::getRank(monoClass->_getInternalClass());

				return typeInfo;
			}
		default:
			break;
		}

		return nullptr;
	}

	void ScriptAssemblyManager::clearScriptObjects()
	{
		mBaseTypesInitialized = false;

		mSystemArrayClass = nullptr;
		mSystemGenericListClass = nullptr;
		mSystemGenericDictionaryClass = nullptr;
		mSystemTypeClass = nullptr;

		mSerializeObjectAttribute = nullptr;
		mDontSerializeFieldAttribute = nullptr;

		mComponentClass = nullptr;
		mSceneObjectClass = nullptr;
		mMissingComponentClass = nullptr;

		mSerializeFieldAttribute = nullptr;
		mHideInInspectorAttribute = nullptr;
		mRangeAttribute = nullptr;
		mStepAttribute = nullptr;
	}

	void ScriptAssemblyManager::initializeBaseTypes()
	{
		// Get necessary classes for detecting needed class & field information
		MonoAssembly* corlib = MonoManager::instance().getAssembly("corlib");
		if(corlib == nullptr)
			BS_EXCEPT(InvalidStateException, "corlib assembly is not loaded.");

		MonoAssembly* bansheeEngineAssembly = MonoManager::instance().getAssembly(ENGINE_ASSEMBLY);
		if(bansheeEngineAssembly == nullptr)
			BS_EXCEPT(InvalidStateException, String(ENGINE_ASSEMBLY) +  " assembly is not loaded.");

		mSystemArrayClass = corlib->getClass("System", "Array");
		if(mSystemArrayClass == nullptr)
			BS_EXCEPT(InvalidStateException, "Cannot find System.Array managed class.");

		mSystemGenericListClass = corlib->getClass("System.Collections.Generic", "List`1");
		if(mSystemGenericListClass == nullptr)
			BS_EXCEPT(InvalidStateException, "Cannot find List<T> managed class.");

		mSystemGenericDictionaryClass = corlib->getClass("System.Collections.Generic", "Dictionary`2");
		if(mSystemGenericDictionaryClass == nullptr)
			BS_EXCEPT(InvalidStateException, "Cannot find Dictionary<TKey, TValue> managed class.");

		mSystemTypeClass = corlib->getClass("System", "Type");
		if (mSystemTypeClass == nullptr)
			BS_EXCEPT(InvalidStateException, "Cannot find Type managed class.");

		mSerializeObjectAttribute = bansheeEngineAssembly->getClass("BansheeEngine", "SerializeObject");
		if(mSerializeObjectAttribute == nullptr)
			BS_EXCEPT(InvalidStateException, "Cannot find SerializableObject managed class.");

		mDontSerializeFieldAttribute = bansheeEngineAssembly->getClass("BansheeEngine", "DontSerializeField");
		if(mDontSerializeFieldAttribute == nullptr)
			BS_EXCEPT(InvalidStateException, "Cannot find DontSerializeField managed class.");

		mRangeAttribute = bansheeEngineAssembly->getClass("BansheeEngine", "Range");
		if (mRangeAttribute == nullptr)
			BS_EXCEPT(InvalidStateException, "Cannot find Range managed class.");

		mStepAttribute = bansheeEngineAssembly->getClass("BansheeEngine", "Step");
		if (mStepAttribute == nullptr)
			BS_EXCEPT(InvalidStateException, "Cannot find Step managed class.");

		mComponentClass = bansheeEngineAssembly->getClass("BansheeEngine", "Component");
		if(mComponentClass == nullptr)
			BS_EXCEPT(InvalidStateException, "Cannot find Component managed class.");

		mMissingComponentClass = bansheeEngineAssembly->getClass("BansheeEngine", "MissingComponent");
		if (mMissingComponentClass == nullptr)
			BS_EXCEPT(InvalidStateException, "Cannot find MissingComponent managed class.");

		mSceneObjectClass = bansheeEngineAssembly->getClass("BansheeEngine", "SceneObject");
		if(mSceneObjectClass == nullptr)
			BS_EXCEPT(InvalidStateException, "Cannot find SceneObject managed class.");

		mSerializeFieldAttribute = bansheeEngineAssembly->getClass("BansheeEngine", "SerializeField");
		if(mSerializeFieldAttribute == nullptr)
			BS_EXCEPT(InvalidStateException, "Cannot find SerializeField managed class.");

		mHideInInspectorAttribute = bansheeEngineAssembly->getClass("BansheeEngine", "HideInInspector");
		if(mHideInInspectorAttribute == nullptr)
			BS_EXCEPT(InvalidStateException, "Cannot find HideInInspector managed class.");

		mBaseTypesInitialized = true;
	}

	bool ScriptAssemblyManager::getSerializableObjectInfo(const String& ns, const String& typeName, SPtr<ManagedSerializableObjectInfo>& outInfo)
	{
		String fullName = ns + "." + typeName;
		for(auto& curAssembly : mAssemblyInfos)
		{
			if (curAssembly.second == nullptr)
				continue;

			auto iterFind = curAssembly.second->mTypeNameToId.find(fullName);
			if(iterFind != curAssembly.second->mTypeNameToId.end())
			{
				outInfo = curAssembly.second->mObjectInfos[iterFind->second];

				return true;
			}
		}

		return false;
	}

	bool ScriptAssemblyManager::hasSerializableObjectInfo(const String& ns, const String& typeName)
	{
		String fullName = ns + "." + typeName;
		for(auto& curAssembly : mAssemblyInfos)
		{
			auto iterFind = curAssembly.second->mTypeNameToId.find(fullName);
			if(iterFind != curAssembly.second->mTypeNameToId.end())
				return true;
		}

		return false;
	}
}