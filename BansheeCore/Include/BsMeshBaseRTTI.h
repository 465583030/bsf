//__________________________ Banshee Project - A modern game development toolkit _________________________________//
//_____________________________________ www.banshee-project.com __________________________________________________//
//________________________ Copyright (c) 2014 Marko Pintera. All rights reserved. ________________________________//
#pragma once

#include "BsCorePrerequisites.h"
#include "BsRTTIType.h"
#include "BsCoreApplication.h"
#include "BsMeshBase.h"
#include "BsException.h"

namespace BansheeEngine
{
	BS_ALLOW_MEMCPY_SERIALIZATION(SubMesh);

	class MeshBaseRTTI : public RTTIType<MeshBase, GpuResource, MeshBaseRTTI>
	{
		SubMesh& getSubMesh(MeshBase* obj, UINT32 arrayIdx) { return obj->mSubMeshes[arrayIdx]; }
		void setSubMesh(MeshBase* obj, UINT32 arrayIdx, SubMesh& value) { obj->mSubMeshes[arrayIdx] = value; }
		UINT32 getNumSubmeshes(MeshBase* obj) { return (UINT32)obj->mSubMeshes.size(); }
		void setNumSubmeshes(MeshBase* obj, UINT32 numElements) { obj->mSubMeshes.resize(numElements); obj->mActiveProxies.resize(numElements); }

		UINT32& getNumVertices(MeshBase* obj) { return obj->mNumVertices; }
		void setNumVertices(MeshBase* obj, UINT32& value) { obj->mNumVertices = value; }

		UINT32& getNumIndices(MeshBase* obj) { return obj->mNumIndices; }
		void setNumIndices(MeshBase* obj, UINT32& value) { obj->mNumIndices = value; }

	public:
		MeshBaseRTTI()
		{
			addPlainField("mNumVertices", 0, &MeshBaseRTTI::getNumVertices, &MeshBaseRTTI::setNumVertices);
			addPlainField("mNumIndices", 1, &MeshBaseRTTI::getNumIndices, &MeshBaseRTTI::setNumIndices);

			addPlainArrayField("mSubMeshes", 2, &MeshBaseRTTI::getSubMesh, 
				&MeshBaseRTTI::getNumSubmeshes, &MeshBaseRTTI::setSubMesh, &MeshBaseRTTI::setNumSubmeshes);
		}

		virtual std::shared_ptr<IReflectable> newRTTIObject() 
		{
			BS_EXCEPT(InternalErrorException, "Cannot instantiate an abstract class.");
		}

		virtual const String& getRTTIName() 
		{
			static String name = "MeshBase";
			throw name;
		}

		virtual UINT32 getRTTIId() 
		{
			return TID_MeshBase;
		}
	};
}