//__________________________ Banshee Project - A modern game development toolkit _________________________________//
//_____________________________________ www.banshee-project.com __________________________________________________//
//________________________ Copyright (c) 2014 Marko Pintera. All rights reserved. ________________________________//
#pragma once

#include "BsCorePrerequisites.h"
#include "BsRTTIType.h"
#include "BsGpuProgramImportOptions.h"

namespace BansheeEngine
{
	class BS_CORE_EXPORT GpuProgramImportOptionsRTTI : public RTTIType<GpuProgramImportOptions, IReflectable, GpuProgramImportOptionsRTTI>
	{
	private:
		String& getEntryPoint(GpuProgramImportOptions* obj) { return obj->mEntryPoint; }
		void setEntryPoint(GpuProgramImportOptions* obj, String& value) { obj->mEntryPoint = value; }

		String& getLanguage(GpuProgramImportOptions* obj) { return obj->mLanguage; }
		void setLanguage(GpuProgramImportOptions* obj, String& value) { obj->mLanguage = value; }

		GpuProgramProfile& getProfile(GpuProgramImportOptions* obj) { return obj->mProfile; }
		void setProfile(GpuProgramImportOptions* obj, GpuProgramProfile& value) { obj->mProfile = value; }

		GpuProgramType& getType(GpuProgramImportOptions* obj) { return obj->mType; }
		void setType(GpuProgramImportOptions* obj, GpuProgramType& value) { obj->mType = value; }

		HGpuProgInclude& getInclude(GpuProgramImportOptions* obj, UINT32 arrIdx) { return obj->mIncludes[arrIdx]; }
		void setInclude(GpuProgramImportOptions* obj, UINT32 arrIdx, HGpuProgInclude& value) { obj->mIncludes[arrIdx] = value; }
		UINT32 getNumIncludes(GpuProgramImportOptions* obj) { return (UINT32)obj->mIncludes.size(); }
		void setNumIncludes(GpuProgramImportOptions* obj, UINT32 num) { obj->mIncludes.clear(); obj->mIncludes.resize(num); }

	public:
		GpuProgramImportOptionsRTTI()
		{
			addPlainField("mEntryPoint", 0, &GpuProgramImportOptionsRTTI::getEntryPoint, &GpuProgramImportOptionsRTTI::setEntryPoint);
			addPlainField("mLanguage", 1, &GpuProgramImportOptionsRTTI::getLanguage, &GpuProgramImportOptionsRTTI::setLanguage);
			addPlainField("mProfile", 2, &GpuProgramImportOptionsRTTI::getProfile, &GpuProgramImportOptionsRTTI::setProfile);
			addPlainField("mType", 3, &GpuProgramImportOptionsRTTI::getType, &GpuProgramImportOptionsRTTI::setType);
			addReflectableArrayField("mIncludes", 4, &GpuProgramImportOptionsRTTI::getInclude, &GpuProgramImportOptionsRTTI::getNumIncludes, 
				&GpuProgramImportOptionsRTTI::setInclude, &GpuProgramImportOptionsRTTI::setNumIncludes);
		}

		virtual const String& getRTTIName()
		{
			static String name = "GpuProgramImportOptions";
			return name;
		}

		virtual UINT32 getRTTIId()
		{
			return TID_GpuProgramImportOptions;
		}

		virtual std::shared_ptr<IReflectable> newRTTIObject()
		{
			return bs_shared_ptr<GpuProgramImportOptions, PoolAlloc>();
		}
	};
}