//__________________________ Banshee Project - A modern game development toolkit _________________________________//
//_____________________________________ www.banshee-project.com __________________________________________________//
//________________________ Copyright (c) 2014 Marko Pintera. All rights reserved. ________________________________//
#pragma once

#include "BsCorePrerequisites.h"
#include "BsRTTIType.h"
#include "BsResource.h"

namespace BansheeEngine
{
	class BS_CORE_EXPORT ResourceRTTI : public RTTIType<Resource, IReflectable, ResourceRTTI>
	{
	private:
		UINT32& getSize(Resource* obj) { return obj->mSize; }
		void setSize(Resource* obj, UINT32& size) { obj->mSize = size; } 

		String& getName(Resource* obj) { return obj->mName; }
		void setName(Resource* obj, String& name) { obj->mName = name; }

	public:
		ResourceRTTI()
		{
			addPlainField("mSize", 0, &ResourceRTTI::getSize, &ResourceRTTI::setSize);
			addPlainField("mName", 1, &ResourceRTTI::getName, &ResourceRTTI::setName);
		}

		virtual const String& getRTTIName()
		{
			static String name = "Resource";
			return name;
		}

		virtual UINT32 getRTTIId()
		{
			return 100;
		}

		virtual std::shared_ptr<IReflectable> newRTTIObject()
		{
			BS_EXCEPT(InternalErrorException, "Cannot instantiate an abstract class.");
		}
	};
}