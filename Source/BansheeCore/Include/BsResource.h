//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsCorePrerequisites.h"
#include "BsIReflectable.h"
#include "BsCoreObject.h"

namespace BansheeEngine
{
	/** @addtogroup Resources
	 *  @{
	 */

	/**	Base class for all resources. */
	class BS_CORE_EXPORT Resource : public IReflectable, public CoreObject
	{
	public:
		Resource(bool requiresGpuInitialization = true);
		virtual ~Resource() {};

		/**	Returns the name of the resource. */
		const WString& getName() const;

		/**	Sets the name of the resource.  */
		void setName(const WString& name);

		/**	Retrieves meta-data containing various information describing a resource. */
		SPtr<ResourceMetaData> getMetaData() const { return mMetaData; }

		/**	Returns whether or not this resource is allowed to be asynchronously loaded. */
		virtual bool allowAsyncLoading() const { return true; }

	protected:
		friend class Resources;
		friend class ResourceHandleBase;

		/**	Retrieves a list of all resources that this resource depends on. */
		virtual void getResourceDependencies(FrameVector<HResource>& dependencies) const { }

		/**	Checks if all the resources this object is dependent on are fully loaded. */
		bool areDependenciesLoaded() const;

		UINT32 mSize;
		SPtr<ResourceMetaData> mMetaData;

	/************************************************************************/
	/* 								SERIALIZATION                      		*/
	/************************************************************************/
	public:
		friend class ResourceRTTI;
		static RTTITypeBase* getRTTIStatic();
		virtual RTTITypeBase* getRTTI() const override;
	};

	/** @} */
}