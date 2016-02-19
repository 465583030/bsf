//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsPhysXPrerequisites.h"
#include "BsPhysicsMesh.h"
#include "PxMaterial.h"

namespace BansheeEngine
{
	/** @addtogroup PhysX
	 *  @{
	 */

	/** PhysX implementation of a PhysicsMesh. */
	class PhysXMesh : public PhysicsMesh
	{
	public:
		PhysXMesh(const MeshDataPtr& meshData, PhysicsMeshType type);

		/** @copydoc PhysicsMesh::getMeshData */
		MeshDataPtr getMeshData() const override;

		/** 
		 * Returns the internal PhysX representation of a triangle mesh. Caller must ensure the physics mesh type is 
		 * triangle. 
		 */
		physx::PxTriangleMesh* _getTriangle() const { assert(mType == PhysicsMeshType::Triangle); return mTriangleMesh; }

		/** 
		 * Returns the internal PhysX representation of a convex mesh. Caller must ensure the physics mesh type is 
		 * convex. 
		 */
		physx::PxConvexMesh* _getConvex() const { assert(mType == PhysicsMeshType::Convex); return mConvexMesh; }

	private:
		/** @copydoc PhysicsMesh::initialize() */
		void initialize() override;

		/** @copydoc PhysicsMesh::initialize() */
		void destroy() override;

		physx::PxTriangleMesh* mTriangleMesh = nullptr;
		physx::PxConvexMesh* mConvexMesh = nullptr;

		UINT8* mCookedData = nullptr;
		UINT32 mCookedDataSize = 0;

		/************************************************************************/
		/* 								SERIALIZATION                      		*/
		/************************************************************************/
	public:
		friend class PhysXMeshRTTI;
		static RTTITypeBase* getRTTIStatic();
		RTTITypeBase* getRTTI() const override;
	};

	/** @} */
}