//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsCorePrerequisites.h"
#include "BsRTTIType.h"
#include "BsCoreApplication.h"
#include "BsMesh.h"
#include "BsMeshManager.h"
#include "BsCoreThread.h"

namespace BansheeEngine
{
	/** @cond RTTI */
	/** @addtogroup RTTI-Impl-Core
	 *  @{
	 */

	class MeshRTTI : public RTTIType<Mesh, MeshBase, MeshRTTI>
	{
		SPtr<VertexDataDesc> getVertexDesc(Mesh* obj) { return obj->mVertexDesc; }
		void setVertexDesc(Mesh* obj, SPtr<VertexDataDesc> value) { obj->mVertexDesc = value; }

		IndexType& getIndexType(Mesh* obj) { return obj->mIndexType; }
		void setIndexType(Mesh* obj, IndexType& value) { obj->mIndexType = value; }

		int& getBufferType(Mesh* obj) { return (int&)obj->mUsage; }
		void setBufferType(Mesh* obj, int& value) { obj->mUsage = value; }

		SPtr<MeshData> getMeshData(Mesh* obj) 
		{ 
			SPtr<MeshData> meshData = obj->allocateSubresourceBuffer(0);

			obj->readSubresource(gCoreAccessor(), 0, meshData);
			gCoreAccessor().submitToCoreThread(true);

			return meshData;
		}

		void setMeshData(Mesh* obj, SPtr<MeshData> meshData) 
		{ 
			obj->mCPUData = meshData;
		}

	public:
		MeshRTTI()
		{
			addReflectablePtrField("mVertexDesc", 0, &MeshRTTI::getVertexDesc, &MeshRTTI::setVertexDesc);

			addPlainField("mIndexType", 1, &MeshRTTI::getIndexType, &MeshRTTI::setIndexType);
			addPlainField("mUsage", 2, &MeshRTTI::getBufferType, &MeshRTTI::setBufferType);

			addReflectablePtrField("mMeshData", 3, &MeshRTTI::getMeshData, &MeshRTTI::setMeshData);
		}

		void onDeserializationEnded(IReflectable* obj, const UnorderedMap<String, UINT64>& params) override
		{
			Mesh* mesh = static_cast<Mesh*>(obj);
			mesh->initialize();
		}

		SPtr<IReflectable> newRTTIObject() override
		{
			return MeshManager::instance().createEmpty();
		}

		const String& getRTTIName() override
		{
			static String name = "Mesh";
			return name;
		}

		UINT32 getRTTIId() override
		{
			return TID_Mesh;
		}
	};

	/** @} */
	/** @endcond */
}