#include "BsMesh.h"
#include "BsMeshRTTI.h"
#include "BsMeshData.h"
#include "BsVector2.h"
#include "BsVector3.h"
#include "BsDebug.h"
#include "BsHardwareBufferManager.h"
#include "BsMeshManager.h"
#include "BsCoreThread.h"
#include "BsAsyncOp.h"
#include "BsAABox.h"
#include "BsVertexDataDesc.h"
#include "BsResources.h"

namespace BansheeEngine
{
	Mesh::Mesh(UINT32 numVertices, UINT32 numIndices, const VertexDataDescPtr& vertexDesc, 
		MeshBufferType bufferType, DrawOperationType drawOp, IndexBuffer::IndexType indexType)
		:MeshBase(numVertices, numIndices, drawOp), mVertexData(nullptr), mIndexBuffer(nullptr),
		mVertexDesc(vertexDesc), mBufferType(bufferType), mIndexType(indexType)
	{

	}

	Mesh::Mesh(UINT32 numVertices, UINT32 numIndices, const VertexDataDescPtr& vertexDesc,
		const Vector<SubMesh>& subMeshes, MeshBufferType bufferType, IndexBuffer::IndexType indexType)
		:MeshBase(numVertices, numIndices, subMeshes), mVertexData(nullptr), mIndexBuffer(nullptr),
		mVertexDesc(vertexDesc), mBufferType(bufferType), mIndexType(indexType)
	{

	}

	Mesh::Mesh(const MeshDataPtr& initialMeshData, MeshBufferType bufferType, DrawOperationType drawOp)
		:MeshBase(initialMeshData->getNumVertices(), initialMeshData->getNumIndices(), drawOp), 
		mVertexData(nullptr), mIndexBuffer(nullptr), mIndexType(initialMeshData->getIndexType()),
		mVertexDesc(initialMeshData->getVertexDesc()), mTempInitialMeshData(initialMeshData)
	{

	}

	Mesh::Mesh(const MeshDataPtr& initialMeshData, const Vector<SubMesh>& subMeshes, MeshBufferType bufferType)
		:MeshBase(initialMeshData->getNumVertices(), initialMeshData->getNumIndices(), subMeshes),
		mVertexData(nullptr), mIndexBuffer(nullptr), mIndexType(initialMeshData->getIndexType()),
		mVertexDesc(initialMeshData->getVertexDesc()), mTempInitialMeshData(initialMeshData)
	{

	}

	Mesh::Mesh()
		:MeshBase(0, 0, DOT_TRIANGLE_LIST), mVertexData(nullptr), mIndexBuffer(nullptr), 
		mBufferType(MeshBufferType::Static), mIndexType(IndexBuffer::IT_32BIT)
	{

	}

	Mesh::~Mesh()
	{
		
	}

	void Mesh::_writeSubresourceSim(UINT32 subresourceIdx, const GpuResourceData& data, bool discardEntireBuffer)
	{
		const MeshData& meshData = static_cast<const MeshData&>(data);
		updateBounds(meshData);
	}

	void Mesh::writeSubresource(UINT32 subresourceIdx, const GpuResourceData& data, bool discardEntireBuffer)
	{
		THROW_IF_NOT_CORE_THREAD;

		if(data.getTypeId() != TID_MeshData)
			BS_EXCEPT(InvalidParametersException, "Invalid GpuResourceData type. Only MeshData is supported.");

		const MeshData& meshData = static_cast<const MeshData&>(data);

		if (discardEntireBuffer)
		{
			if (mBufferType == MeshBufferType::Static)
			{
				LOGWRN("Buffer discard is enabled but buffer was not created as dynamic. Disabling discard.");
				discardEntireBuffer = false;
			}
		}
		else
		{
			if (mBufferType == MeshBufferType::Dynamic)
			{
				LOGWRN("Buffer discard is not enabled but buffer was created as dynamic. Enabling discard.");
				discardEntireBuffer = true;
			}
		}

		// Indices
		UINT32 indicesSize = meshData.getIndexBufferSize();
		UINT8* srcIdxData = meshData.getIndexData();

		if (meshData.getIndexElementSize() != mIndexBuffer->getIndexSize())
		{
			BS_EXCEPT(InvalidParametersException, "Provided index size doesn't match meshes index size. Needed: " +
				toString(mIndexBuffer->getIndexSize()) + ". Got: " + toString(meshData.getIndexElementSize()));
		}

		if (indicesSize > mIndexBuffer->getSizeInBytes())
			BS_EXCEPT(InvalidParametersException, "Index buffer values are being written out of valid range.");

		mIndexBuffer->writeData(0, indicesSize, srcIdxData, discardEntireBuffer ? BufferWriteType::Discard : BufferWriteType::Normal);

		// Vertices
		for (UINT32 i = 0; i <= mVertexDesc->getMaxStreamIdx(); i++)
		{
			if (!mVertexDesc->hasStream(i))
				continue;

			if (!meshData.getVertexDesc()->hasStream(i))
				continue;

			// Ensure both have the same sized vertices
			UINT32 myVertSize = mVertexDesc->getVertexStride(i);
			UINT32 otherVertSize = meshData.getVertexDesc()->getVertexStride(i);
			if (myVertSize != otherVertSize)
			{
				BS_EXCEPT(InvalidParametersException, "Provided vertex size for stream " + toString(i) + " doesn't match meshes vertex size. Needed: " +
					toString(myVertSize) + ". Got: " + toString(otherVertSize));
			}

			VertexBufferPtr vertexBuffer = mVertexData->getBuffer(i);

			UINT32 bufferSize = meshData.getStreamSize(i);
			UINT8* srcVertBufferData = meshData.getStreamData(i);

			if (bufferSize > vertexBuffer->getSizeInBytes())
				BS_EXCEPT(InvalidParametersException, "Vertex buffer values for stream \"" + toString(i) + "\" are being written out of valid range.");

			if (vertexBuffer->vertexColorReqRGBFlip())
			{
				UINT8* bufferCopy = (UINT8*)bs_alloc(bufferSize);
				memcpy(bufferCopy, srcVertBufferData, bufferSize); // TODO Low priority - Attempt to avoid this copy

				UINT32 vertexStride = meshData.getVertexDesc()->getVertexStride(i);
				for (INT32 semanticIdx = 0; semanticIdx < VertexBuffer::MAX_SEMANTIC_IDX; semanticIdx++)
				{
					if (!meshData.getVertexDesc()->hasElement(VES_COLOR, semanticIdx, i))
						continue;

					UINT8* colorData = bufferCopy + mVertexDesc->getElementOffsetFromStream(VES_COLOR, semanticIdx, i);
					for (UINT32 j = 0; j < mVertexData->vertexCount; j++)
					{
						UINT32* curColor = (UINT32*)colorData;

						(*curColor) = ((*curColor) & 0xFF00FF00) | ((*curColor >> 16) & 0x000000FF) | ((*curColor << 16) & 0x00FF0000);

						colorData += vertexStride;
					}
				}

				vertexBuffer->writeData(0, bufferSize, bufferCopy, discardEntireBuffer ? BufferWriteType::Discard : BufferWriteType::Normal);

				bs_free(bufferCopy);
			}
			else
			{
				vertexBuffer->writeData(0, bufferSize, srcVertBufferData, discardEntireBuffer ? BufferWriteType::Discard : BufferWriteType::Normal);
			}
		}
	}

	void Mesh::readSubresource(UINT32 subresourceIdx, GpuResourceData& data)
	{
		THROW_IF_NOT_CORE_THREAD;

		if(data.getTypeId() != TID_MeshData)
			BS_EXCEPT(InvalidParametersException, "Invalid GpuResourceData type. Only MeshData is supported.");

		IndexBuffer::IndexType indexType = IndexBuffer::IT_32BIT;
		if(mIndexBuffer)
			indexType = mIndexBuffer->getType();

		MeshData& meshData = static_cast<MeshData&>(data);

		if(mIndexBuffer)
		{
			if(meshData.getIndexElementSize() != mIndexBuffer->getIndexSize())
			{
				BS_EXCEPT(InvalidParametersException, "Provided index size doesn't match meshes index size. Needed: " + 
					toString(mIndexBuffer->getIndexSize()) + ". Got: " + toString(meshData.getIndexElementSize()));
			}

			UINT8* idxData = static_cast<UINT8*>(mIndexBuffer->lock(GBL_READ_ONLY));
			UINT32 idxElemSize = mIndexBuffer->getIndexSize();

			UINT8* indices = nullptr;

			if(indexType == IndexBuffer::IT_16BIT)
				indices = (UINT8*)meshData.getIndices16();
			else
				indices = (UINT8*)meshData.getIndices32();

			UINT32 numIndicesToCopy = std::min(mNumIndices, meshData.getNumIndices());

			UINT32 indicesSize = numIndicesToCopy * idxElemSize;
			if(indicesSize > meshData.getIndexBufferSize())
				BS_EXCEPT(InvalidParametersException, "Provided buffer doesn't have enough space to store mesh indices.");

			memcpy(indices, idxData, numIndicesToCopy * idxElemSize);

			mIndexBuffer->unlock();
		}

		if(mVertexData)
		{
			auto vertexBuffers = mVertexData->getBuffers();
			
			UINT32 streamIdx = 0;
			for(auto iter = vertexBuffers.begin(); iter != vertexBuffers.end() ; ++iter)
			{
				if(!meshData.getVertexDesc()->hasStream(streamIdx))
					continue;

				// Ensure both have the same sized vertices
				UINT32 myVertSize = mVertexDesc->getVertexStride(streamIdx);
				UINT32 otherVertSize = meshData.getVertexDesc()->getVertexStride(streamIdx);
				if(myVertSize != otherVertSize)
				{
					BS_EXCEPT(InvalidParametersException, "Provided vertex size for stream " + toString(streamIdx) + " doesn't match meshes vertex size. Needed: " + 
						toString(myVertSize) + ". Got: " + toString(otherVertSize));
				}

				UINT32 numVerticesToCopy = meshData.getNumVertices();

				VertexBufferPtr vertexBuffer = iter->second;
				UINT32 bufferSize = vertexBuffer->getVertexSize() * numVerticesToCopy;
				
				if(bufferSize > vertexBuffer->getSizeInBytes())
					BS_EXCEPT(InvalidParametersException, "Vertex buffer values for stream \"" + toString(streamIdx) + "\" are being read out of valid range.");

				UINT8* vertDataPtr = static_cast<UINT8*>(vertexBuffer->lock(GBL_READ_ONLY));

				UINT8* dest = meshData.getStreamData(streamIdx);
				memcpy(dest, vertDataPtr, bufferSize);

				vertexBuffer->unlock();

				streamIdx++;
			}
		}
	}

	MeshDataPtr Mesh::allocateSubresourceBuffer(UINT32 subresourceIdx) const
	{
		IndexBuffer::IndexType indexType = IndexBuffer::IT_32BIT;
		if(mIndexBuffer)
			indexType = mIndexBuffer->getType();

		MeshDataPtr meshData = bs_shared_ptr<MeshData>(mVertexData->vertexCount, mNumIndices, mVertexDesc, indexType);

		return meshData;
	}

	std::shared_ptr<VertexData> Mesh::_getVertexData() const
	{
		THROW_IF_NOT_CORE_THREAD;

		return mVertexData;
	}

	IndexBufferPtr Mesh::_getIndexBuffer() const
	{
		THROW_IF_NOT_CORE_THREAD;

		return mIndexBuffer;
	}

	void Mesh::initialize()
	{
		if (mTempInitialMeshData != nullptr)
		{
			updateBounds(*mTempInitialMeshData);
		}

		MeshBase::initialize();
	}

	void Mesh::initialize_internal()
	{
		THROW_IF_NOT_CORE_THREAD;
		
		mIndexBuffer = HardwareBufferManager::instance().createIndexBuffer(mIndexType,
			mNumIndices, mBufferType == MeshBufferType::Dynamic ? GBU_DYNAMIC : GBU_STATIC);

		mVertexData = std::shared_ptr<VertexData>(bs_new<VertexData, PoolAlloc>());

		mVertexData->vertexCount = mNumVertices;
		mVertexData->vertexDeclaration = mVertexDesc->createDeclaration();

		for(UINT32 i = 0; i <= mVertexDesc->getMaxStreamIdx(); i++)
		{
			if(!mVertexDesc->hasStream(i))
				continue;

			VertexBufferPtr vertexBuffer = HardwareBufferManager::instance().createVertexBuffer(
				mVertexData->vertexDeclaration->getVertexSize(i),
				mVertexData->vertexCount,
				mBufferType == MeshBufferType::Dynamic ? GBU_DYNAMIC : GBU_STATIC);

			mVertexData->setBuffer(i, vertexBuffer);
		}

		// TODO Low priority - DX11 (and maybe OpenGL)? allow an optimization that allows you to set
		// buffer data upon buffer construction, instead of setting it in a second step like I do here
		if(mTempInitialMeshData != nullptr)
		{
			writeSubresource(0, *mTempInitialMeshData, mBufferType == MeshBufferType::Dynamic);
			mTempInitialMeshData = nullptr;
		}

		MeshBase::initialize_internal();
	}

	void Mesh::destroy_internal()
	{
		THROW_IF_NOT_CORE_THREAD;

		mVertexData = nullptr;
		mIndexBuffer = nullptr;
		mVertexDesc = nullptr;
		mTempInitialMeshData = nullptr;

		MeshBase::destroy_internal();
	}

	void Mesh::updateBounds(const MeshData& meshData)
	{
		VertexDataDescPtr vertexDesc = meshData.getVertexDesc();
		for (UINT32 i = 0; i < vertexDesc->getNumElements(); i++)
		{
			const VertexElement& curElement = vertexDesc->getElement(i);

			if (curElement.getSemantic() != VES_POSITION || (curElement.getType() != VET_FLOAT3 && curElement.getType() != VET_FLOAT4))
				continue;

			UINT8* data = meshData.getElementData(curElement.getSemantic(), curElement.getSemanticIdx(), curElement.getStreamIdx());
			UINT32 stride = vertexDesc->getVertexStride(curElement.getStreamIdx());

			mBounds = calculateBounds((UINT8*)data, mTempInitialMeshData->getNumVertices(), stride);
			markCoreDirty();

			break;
		}
	}

	Bounds Mesh::calculateBounds(UINT8* verticesPtr, UINT32 numVertices, UINT32 stride) const
	{
		Bounds bounds;

		if (mNumVertices > 0)
		{
			Vector3 accum;
			Vector3 min;
			Vector3 max;

			Vector3 curPosition = *(Vector3*)verticesPtr;
			accum = curPosition;
			min = curPosition;
			max = curPosition;

			for (UINT32 i = 1; i < mNumVertices; i++)
			{
				curPosition = *(Vector3*)(verticesPtr + stride * i);
				accum += curPosition;
				min = Vector3::min(min, curPosition);
				max = Vector3::max(max, curPosition);
			}

			Vector3 center = accum / (float)mNumVertices;
			float radiusSqrd = 0.0f;

			for (UINT32 i = 0; i < mNumVertices; i++)
			{
				curPosition = *(Vector3*)(verticesPtr + stride * i);
				float dist = center.squaredDistance(curPosition);

				if (dist > radiusSqrd)
					radiusSqrd = dist;
			}

			float radius = Math::sqrt(radiusSqrd);

			bounds = Bounds(AABox(min, max), Sphere(center, radius));
		}

		return bounds;
	}

	MeshProxyPtr Mesh::_createProxy(UINT32 subMeshIdx)
	{
		MeshProxyPtr coreProxy = bs_shared_ptr<MeshProxy>();
		coreProxy->mesh = std::static_pointer_cast<MeshBase>(getThisPtr());
		coreProxy->bounds = mBounds;
		coreProxy->subMesh = getSubMesh(subMeshIdx);
		coreProxy->submeshIdx = subMeshIdx;

		return coreProxy;
	}

	HMesh Mesh::dummy()
	{
		return MeshManager::instance().getDummyMesh();
	}

	/************************************************************************/
	/* 								SERIALIZATION                      		*/
	/************************************************************************/

	RTTITypeBase* Mesh::getRTTIStatic()
	{
		return MeshRTTI::instance();
	}

	RTTITypeBase* Mesh::getRTTI() const
	{
		return Mesh::getRTTIStatic();
	}

	/************************************************************************/
	/* 								STATICS		                     		*/
	/************************************************************************/

	HMesh Mesh::create(UINT32 numVertices, UINT32 numIndices, const VertexDataDescPtr& vertexDesc, 
		MeshBufferType bufferType, DrawOperationType drawOp, IndexBuffer::IndexType indexType)
	{
		MeshPtr meshPtr = _createPtr(numVertices, numIndices, vertexDesc, bufferType, drawOp, indexType);

		return static_resource_cast<Mesh>(gResources()._createResourceHandle(meshPtr));
	}

	HMesh Mesh::create(UINT32 numVertices, UINT32 numIndices, const VertexDataDescPtr& vertexDesc,
		const Vector<SubMesh>& subMeshes, MeshBufferType bufferType, IndexBuffer::IndexType indexType)
	{
		MeshPtr meshPtr = _createPtr(numVertices, numIndices, vertexDesc, subMeshes, bufferType, indexType);

		return static_resource_cast<Mesh>(gResources()._createResourceHandle(meshPtr));
	}

	HMesh Mesh::create(const MeshDataPtr& initialMeshData, MeshBufferType bufferType, DrawOperationType drawOp)
	{
		MeshPtr meshPtr = _createPtr(initialMeshData, bufferType, drawOp);

		return static_resource_cast<Mesh>(gResources()._createResourceHandle(meshPtr));
	}

	HMesh Mesh::create(const MeshDataPtr& initialMeshData, const Vector<SubMesh>& subMeshes, MeshBufferType bufferType)
	{
		MeshPtr meshPtr = _createPtr(initialMeshData, subMeshes, bufferType);

		return static_resource_cast<Mesh>(gResources()._createResourceHandle(meshPtr));
	}

	MeshPtr Mesh::_createPtr(UINT32 numVertices, UINT32 numIndices, const VertexDataDescPtr& vertexDesc, 
		MeshBufferType bufferType, DrawOperationType drawOp, IndexBuffer::IndexType indexType)
	{
		return MeshManager::instance().create(numVertices, numIndices, vertexDesc, bufferType, drawOp, indexType);
	}

	MeshPtr Mesh::_createPtr(UINT32 numVertices, UINT32 numIndices, const VertexDataDescPtr& vertexDesc,
		const Vector<SubMesh>& subMeshes, MeshBufferType bufferType, IndexBuffer::IndexType indexType)
	{
		return MeshManager::instance().create(numVertices, numIndices, vertexDesc, subMeshes, bufferType, indexType);
	}

	MeshPtr Mesh::_createPtr(const MeshDataPtr& initialMeshData, MeshBufferType bufferType, DrawOperationType drawOp)
	{
		return MeshManager::instance().create(initialMeshData, bufferType, drawOp);
	}

	MeshPtr Mesh::_createPtr(const MeshDataPtr& initialMeshData, const Vector<SubMesh>& subMeshes, MeshBufferType bufferType)
	{
		return MeshManager::instance().create(initialMeshData, subMeshes, bufferType);
	}
}