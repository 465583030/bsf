//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsCorePrerequisites.h"
#include "BsRTTIType.h"
#include "BsResource.h"
#include "BsResourceMetaData.h"

namespace BansheeEngine
{
	/** @cond RTTI */
	/** @addtogroup RTTI-Impl-Core
	 *  @{
	 */

	class BS_CORE_EXPORT ResourceRTTI : public RTTIType<Resource, IReflectable, ResourceRTTI>
	{
	private:
		UINT32& getSize(Resource* obj) { return obj->mSize; }
		void setSize(Resource* obj, UINT32& size) { obj->mSize = size; } 

		ResourceMetaDataPtr getMetaData(Resource* obj) { return obj->mMetaData; }
		void setMetaData(Resource* obj, ResourceMetaDataPtr value) { obj->mMetaData = value; }

	public:
		ResourceRTTI()
		{
			addPlainField("mSize", 0, &ResourceRTTI::getSize, &ResourceRTTI::setSize);
			addReflectablePtrField("mMetaData", 1, &ResourceRTTI::getMetaData, &ResourceRTTI::setMetaData);
		}

		const String& getRTTIName() override
		{
			static String name = "Resource";
			return name;
		}

		UINT32 getRTTIId() override
		{
			return 100;
		}

		std::shared_ptr<IReflectable> newRTTIObject() override
		{
			BS_EXCEPT(InternalErrorException, "Cannot instantiate an abstract class.");
		}
	};

	/** @} */
	/** @endcond */
}