#include "BsUtility.h"
#include "BsRTTIType.h"

namespace BansheeEngine
{
	Vector<ResourceDependency> Utility::findResourceDependencies(IReflectable& obj, bool recursive)
	{
		Map<String, ResourceDependency> dependencies;
		findResourceDependenciesInternal(obj, recursive, dependencies);

		Vector<ResourceDependency> dependencyList(dependencies.size());
		UINT32 i = 0;
		for (auto& entry : dependencies)
		{
			dependencyList[i] = entry.second;
			i++;
		}

		return dependencyList;
	}

	void Utility::findResourceDependenciesInternal(IReflectable& obj, bool recursive, Map<String, ResourceDependency>& dependencies)
	{
		RTTITypeBase* rtti = obj.getRTTI();
		rtti->onSerializationStarted(&obj);

		UINT32 numFields = rtti->getNumFields();
		for (UINT32 i = 0; i < numFields; i++)
		{
			RTTIField* field = rtti->getField(i);
			if ((field->getFlags() & RTTI_Flag_SkipInReferenceSearch) != 0)
				continue;

			if (field->isReflectableType())
			{
				RTTIReflectableFieldBase* reflectableField = static_cast<RTTIReflectableFieldBase*>(field);

				if (reflectableField->getType()->getRTTIId() == TID_ResourceHandle)
				{
					if (reflectableField->isArray())
					{
						UINT32 numElements = reflectableField->getArraySize(&obj);
						for (UINT32 j = 0; j < numElements; j++)
						{
							HResource resource = (HResource&)reflectableField->getArrayValue(&obj, j);
							if (resource != nullptr)
							{
								ResourceDependency& dependency = dependencies[resource.getUUID()];
								dependency.resource = resource;
								dependency.numReferences++;
							}
						}
					}
					else
					{
						HResource resource = (HResource&)reflectableField->getValue(&obj);
						if (resource != nullptr)
						{
							ResourceDependency& dependency = dependencies[resource.getUUID()];
							dependency.resource = resource;
							dependency.numReferences++;
						}
					}
				}
				else if (recursive)
				{
					// Optimization, no need to retrieve its value and go deeper if it has no 
					// reflectable children that may hold the reference.
					if (hasReflectableChildren(reflectableField->getType())) 
					{
						if (reflectableField->isArray())
						{
							UINT32 numElements = reflectableField->getArraySize(&obj);
							for (UINT32 j = 0; j < numElements; j++)
							{
								IReflectable& childObj = reflectableField->getArrayValue(&obj, j);
								findResourceDependenciesInternal(childObj, true, dependencies);
							}
						}
						else
						{
							IReflectable& childObj = reflectableField->getValue(&obj);
							findResourceDependenciesInternal(childObj, true, dependencies);
						}
					}
				}
			}
			else if (field->isReflectablePtrType() && recursive)
			{
				RTTIReflectablePtrFieldBase* reflectablePtrField = static_cast<RTTIReflectablePtrFieldBase*>(field);

				// Optimization, no need to retrieve its value and go deeper if it has no 
				// reflectable children that may hold the reference.
				if (hasReflectableChildren(reflectablePtrField->getType()))
				{
					if (reflectablePtrField->isArray())
					{
						UINT32 numElements = reflectablePtrField->getArraySize(&obj);
						for (UINT32 j = 0; j < numElements; j++)
						{
							SPtr<IReflectable> childObj = reflectablePtrField->getArrayValue(&obj, j);

							if (childObj != nullptr)
								findResourceDependenciesInternal(*childObj, true, dependencies);
						}
					}
					else
					{
						SPtr<IReflectable> childObj = reflectablePtrField->getValue(&obj);

						if (childObj != nullptr)
							findResourceDependenciesInternal(*childObj, true, dependencies);
					}
				}
			}
		}

		rtti->onSerializationEnded(&obj);
	}

	bool Utility::hasReflectableChildren(RTTITypeBase* type)
	{
		UINT32 numFields = type->getNumFields();
		for (UINT32 i = 0; i < numFields; i++)
		{
			RTTIField* field = type->getField(i);
			if (field->isReflectableType() || field->isReflectablePtrType())
				return true;
		}

		const Vector<RTTITypeBase*>& derivedClasses = type->getDerivedClasses();
		for (auto& derivedClass : derivedClasses)
		{
			numFields = derivedClass->getNumFields();
			for (UINT32 i = 0; i < numFields; i++)
			{
				RTTIField* field = derivedClass->getField(i);
				if (field->isReflectableType() || field->isReflectablePtrType())
					return true;
			}
		}

		return false;
	}
}