//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsMesh.h"
#include "BsMeshRTTI.h"
#include "BsMeshData.h"
#include "BsDebug.h"
#include "BsHardwareBufferManager.h"
#include "BsMeshManager.h"
#include "BsCoreThread.h"
#include "BsAsyncOp.h"
#include "BsVertexDataDesc.h"
#include "BsResources.h"
#include "BsRenderAPI.h"

namespace BansheeEngine
{
	MESH_DESC MESH_DESC::DEFAULT = MESH_DESC();

	MeshCore::MeshCore(const SPtr<MeshData>& initialMeshData, const MESH_DESC& desc)
		: MeshCoreBase(desc.numVertices, desc.numIndices, desc.subMeshes), mVertexData(nullptr), mIndexBuffer(nullptr)
		, mVertexDesc(desc.vertexDesc), mUsage(desc.usage), mIndexType(desc.indexType)
		, mTempInitialMeshData(initialMeshData), mSkeleton(desc.skeleton), mMorphShapes(desc.morphShapes)
		
	{ }

	MeshCore::~MeshCore()
	{
		THROW_IF_NOT_CORE_THREAD;

		mVertexData = nullptr;
		mIndexBuffer = nullptr;
		mVertexDesc = nullptr;
		mTempInitialMeshData = nullptr;
	}

	void MeshCore::initialize()
	{
		THROW_IF_NOT_CORE_THREAD;

		bool isDynamic = (mUsage & MU_DYNAMIC) != 0;

		mIndexBuffer = HardwareBufferCoreManager::instance().createIndexBuffer(mIndexType,
			mProperties.mNumIndices, isDynamic ? GBU_DYNAMIC : GBU_STATIC);

		mVertexData = SPtr<VertexData>(bs_new<VertexData>());

		mVertexData->vertexCount = mProperties.mNumVertices;
		mVertexData->vertexDeclaration = HardwareBufferCoreManager::instance().createVertexDeclaration(mVertexDesc);

		for (UINT32 i = 0; i <= mVertexDesc->getMaxStreamIdx(); i++)
		{
			if (!mVertexDesc->hasStream(i))
				continue;

			SPtr<VertexBufferCore> vertexBuffer = HardwareBufferCoreManager::instance().createVertexBuffer(
				mVertexData->vertexDeclaration->getProperties().getVertexSize(i),
				mVertexData->vertexCount,
				isDynamic ? GBU_DYNAMIC : GBU_STATIC);

			mVertexData->setBuffer(i, vertexBuffer);
		}

		// TODO Low priority - DX11 (and maybe OpenGL)? allow an optimization that allows you to set
		// buffer data upon buffer construction, instead of setting it in a second step like I do here
		if (mTempInitialMeshData != nullptr)
		{
			writeSubresource(0, *mTempInitialMeshData, isDynamic);
			mTempInitialMeshData = nullptr;
		}

		MeshCoreBase::initialize();
	}

	SPtr<VertexData> MeshCore::getVertexData() const
	{
		THROW_IF_NOT_CORE_THREAD;

		return mVertexData;
	}

	SPtr<IndexBufferCore> MeshCore::getIndexBuffer() const
	{
		THROW_IF_NOT_CORE_THREAD;

		return mIndexBuffer;
	}

	SPtr<VertexDataDesc> MeshCore::getVertexDesc() const
	{
		THROW_IF_NOT_CORE_THREAD;

		return mVertexDesc;
	}

	void MeshCore::writeSubresource(UINT32 subresourceIdx, const MeshData& meshData, bool discardEntireBuffer, bool performUpdateBounds)
	{
		THROW_IF_NOT_CORE_THREAD;

		if (discardEntireBuffer)
		{
			if ((mUsage & MU_STATIC) != 0)
			{
				LOGWRN("Buffer discard is enabled but buffer was not created as dynamic. Disabling discard.");
				discardEntireBuffer = false;
			}
		}
		else
		{
			if ((mUsage & MU_DYNAMIC) != 0)
			{
				LOGWRN("Buffer discard is not enabled but buffer was created as dynamic. Enabling discard.");
				discardEntireBuffer = true;
			}
		}

		// Indices
		const IndexBufferProperties& ibProps = mIndexBuffer->getProperties();

		UINT32 indicesSize = meshData.getIndexBufferSize();
		UINT8* srcIdxData = meshData.getIndexData();

		if (meshData.getIndexElementSize() != ibProps.getIndexSize())
		{
			LOGERR("Provided index size doesn't match meshes index size. Needed: " +
				toString(ibProps.getIndexSize()) + ". Got: " + toString(meshData.getIndexElementSize()));

			return;
		}

		if (indicesSize > mIndexBuffer->getSizeInBytes())
		{
			indicesSize = mIndexBuffer->getSizeInBytes();
			LOGERR("Index buffer values are being written out of valid range.");
		}

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
				LOGERR("Provided vertex size for stream " + toString(i) + " doesn't match meshes vertex size. Needed: " +
					toString(myVertSize) + ". Got: " + toString(otherVertSize));
				
				continue;
			}

			SPtr<VertexBufferCore> vertexBuffer = mVertexData->getBuffer(i);

			UINT32 bufferSize = meshData.getStreamSize(i);
			UINT8* srcVertBufferData = meshData.getStreamData(i);

			if (bufferSize > vertexBuffer->getSizeInBytes())
			{
				bufferSize = vertexBuffer->getSizeInBytes();
				LOGERR("Vertex buffer values for stream \"" + toString(i) + "\" are being written out of valid range.");
			}

			if (RenderAPICore::instance().getAPIInfo().getVertexColorFlipRequired())
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

		if (performUpdateBounds)
			updateBounds(meshData);
	}

	void MeshCore::readSubresource(UINT32 subresourceIdx, MeshData& meshData)
	{
		THROW_IF_NOT_CORE_THREAD;

		IndexType indexType = IT_32BIT;
		if (mIndexBuffer)
			indexType = mIndexBuffer->getProperties().getType();

		if (mIndexBuffer)
		{
			const IndexBufferProperties& ibProps = mIndexBuffer->getProperties();

			if (meshData.getIndexElementSize() != ibProps.getIndexSize())
			{
				LOGERR("Provided index size doesn't match meshes index size. Needed: " +
					toString(ibProps.getIndexSize()) + ". Got: " + toString(meshData.getIndexElementSize()));
				return;
			}

			UINT8* idxData = static_cast<UINT8*>(mIndexBuffer->lock(GBL_READ_ONLY));
			UINT32 idxElemSize = ibProps.getIndexSize();

			UINT8* indices = nullptr;

			if (indexType == IT_16BIT)
				indices = (UINT8*)meshData.getIndices16();
			else
				indices = (UINT8*)meshData.getIndices32();

			UINT32 numIndicesToCopy = std::min(mProperties.mNumIndices, meshData.getNumIndices());

			UINT32 indicesSize = numIndicesToCopy * idxElemSize;
			if (indicesSize > meshData.getIndexBufferSize())
			{
				LOGERR("Provided buffer doesn't have enough space to store mesh indices.");
				return;
			}

			memcpy(indices, idxData, numIndicesToCopy * idxElemSize);

			mIndexBuffer->unlock();
		}

		if (mVertexData)
		{
			auto vertexBuffers = mVertexData->getBuffers();

			UINT32 streamIdx = 0;
			for (auto iter = vertexBuffers.begin(); iter != vertexBuffers.end(); ++iter)
			{
				if (!meshData.getVertexDesc()->hasStream(streamIdx))
					continue;

				SPtr<VertexBufferCore> vertexBuffer = iter->second;
				const VertexBufferProperties& vbProps = vertexBuffer->getProperties();

				// Ensure both have the same sized vertices
				UINT32 myVertSize = mVertexDesc->getVertexStride(streamIdx);
				UINT32 otherVertSize = meshData.getVertexDesc()->getVertexStride(streamIdx);
				if (myVertSize != otherVertSize)
				{
					LOGERR("Provided vertex size for stream " + toString(streamIdx) + " doesn't match meshes vertex size. Needed: " +
						toString(myVertSize) + ". Got: " + toString(otherVertSize));

					continue;
				}

				UINT32 numVerticesToCopy = meshData.getNumVertices();
				UINT32 bufferSize = vbProps.getVertexSize() * numVerticesToCopy;

				if (bufferSize > vertexBuffer->getSizeInBytes())
				{
					LOGERR("Vertex buffer values for stream \"" + toString(streamIdx) + "\" are being read out of valid range.");
					continue;
				}

				UINT8* vertDataPtr = static_cast<UINT8*>(vertexBuffer->lock(GBL_READ_ONLY));

				UINT8* dest = meshData.getStreamData(streamIdx);
				memcpy(dest, vertDataPtr, bufferSize);

				vertexBuffer->unlock();

				streamIdx++;
			}
		}
	}

	void MeshCore::updateBounds(const MeshData& meshData)
	{
		mProperties.mBounds = meshData.calculateBounds();
		
		// TODO - Sync this to sim-thread possibly?
	}

	SPtr<MeshCore> MeshCore::create(UINT32 numVertices, UINT32 numIndices, const SPtr<VertexDataDesc>& vertexDesc,
		int usage, DrawOperationType drawOp, IndexType indexType)
	{
		MESH_DESC desc;
		desc.numVertices = numVertices;
		desc.numIndices = numIndices;
		desc.vertexDesc = vertexDesc;
		desc.subMeshes.push_back(SubMesh(0, numIndices, drawOp));
		desc.usage = usage;
		desc.indexType = indexType;

		SPtr<MeshCore> mesh = bs_shared_ptr<MeshCore>(new (bs_alloc<MeshCore>()) MeshCore(nullptr, desc));
		mesh->_setThisPtr(mesh);
		mesh->initialize();

		return mesh;
	}

	SPtr<MeshCore> MeshCore::create(const MESH_DESC& desc)
	{
		SPtr<MeshCore> mesh = bs_shared_ptr<MeshCore>(new (bs_alloc<MeshCore>()) MeshCore(nullptr, desc));

		mesh->_setThisPtr(mesh);
		mesh->initialize();

		return mesh;
	}

	SPtr<MeshCore> MeshCore::create(const SPtr<MeshData>& initialMeshData, const MESH_DESC& desc)
	{
		MESH_DESC descCopy = desc;
		descCopy.numVertices = initialMeshData->getNumVertices();
		descCopy.numIndices = initialMeshData->getNumIndices();
		descCopy.vertexDesc = initialMeshData->getVertexDesc();
		descCopy.indexType = initialMeshData->getIndexType();

		SPtr<MeshCore> mesh = bs_shared_ptr<MeshCore>(new (bs_alloc<MeshCore>()) MeshCore(initialMeshData, descCopy));

		mesh->_setThisPtr(mesh);
		mesh->initialize();

		return mesh;
	}

	Mesh::Mesh(const MESH_DESC& desc)
		:MeshBase(desc.numVertices, desc.numIndices, desc.subMeshes), mVertexDesc(desc.vertexDesc), mUsage(desc.usage),
		mIndexType(desc.indexType), mSkeleton(desc.skeleton), mMorphShapes(desc.morphShapes)
	{

	}

	Mesh::Mesh(const SPtr<MeshData>& initialMeshData, const MESH_DESC& desc)
		:MeshBase(initialMeshData->getNumVertices(), initialMeshData->getNumIndices(), desc.subMeshes),
		mCPUData(initialMeshData), mVertexDesc(initialMeshData->getVertexDesc()),
		mUsage(desc.usage), mIndexType(initialMeshData->getIndexType()), mSkeleton(desc.skeleton), 
		mMorphShapes(desc.morphShapes)
	{

	}

	Mesh::Mesh()
		:MeshBase(0, 0, DOT_TRIANGLE_LIST), mUsage(MU_STATIC), mIndexType(IT_32BIT)
	{

	}

	Mesh::~Mesh()
	{
		
	}

	AsyncOp Mesh::writeSubresource(CoreAccessor& accessor, UINT32 subresourceIdx, const SPtr<MeshData>& data, bool discardEntireBuffer)
	{
		updateBounds(*data);
		updateCPUBuffer(subresourceIdx, *data);

		data->_lock();

		std::function<void(const SPtr<MeshCore>&, UINT32, const SPtr<MeshData>&, bool, AsyncOp&)> func =
			[&](const SPtr<MeshCore>& mesh, UINT32 _subresourceIdx, const SPtr<MeshData>& _meshData, bool _discardEntireBuffer, AsyncOp& asyncOp)
		{
			mesh->writeSubresource(_subresourceIdx, *_meshData, _discardEntireBuffer, false);
			_meshData->_unlock();
			asyncOp._completeOperation();

		};

		return accessor.queueReturnCommand(std::bind(func, getCore(), subresourceIdx,
			 data, discardEntireBuffer, std::placeholders::_1));
	}

	AsyncOp Mesh::readSubresource(CoreAccessor& accessor, UINT32 subresourceIdx, const SPtr<MeshData>& data)
	{
		data->_lock();

		std::function<void(const SPtr<MeshCore>&, UINT32, const SPtr<MeshData>&, AsyncOp&)> func =
			[&](const SPtr<MeshCore>& mesh, UINT32 _subresourceIdx, const SPtr<MeshData>& _meshData, AsyncOp& asyncOp)
		{
			mesh->readSubresource(_subresourceIdx, *_meshData);
			_meshData->_unlock();
			asyncOp._completeOperation();

		};

		return accessor.queueReturnCommand(std::bind(func, getCore(), subresourceIdx,
			data, std::placeholders::_1));
	}

	SPtr<MeshData> Mesh::allocateSubresourceBuffer(UINT32 subresourceIdx) const
	{
		SPtr<MeshData> meshData = bs_shared_ptr_new<MeshData>(mProperties.mNumVertices, mProperties.mNumIndices, mVertexDesc, mIndexType);

		return meshData;
	}

	void Mesh::initialize()
	{
		if (mCPUData != nullptr)
			updateBounds(*mCPUData);

		MeshBase::initialize();

		if ((mUsage & MU_CPUCACHED) != 0 && mCPUData == nullptr)
			createCPUBuffer();
	}

	void Mesh::updateBounds(const MeshData& meshData)
	{
		mProperties.mBounds = meshData.calculateBounds();
		markCoreDirty();
	}

	SPtr<MeshCore> Mesh::getCore() const
	{
		return std::static_pointer_cast<MeshCore>(mCoreSpecific);
	}

	SPtr<CoreObjectCore> Mesh::createCore() const
	{
		MESH_DESC desc;
		desc.numVertices = mProperties.mNumVertices;
		desc.numIndices = mProperties.mNumIndices;
		desc.vertexDesc = mVertexDesc;
		desc.subMeshes = mProperties.mSubMeshes;
		desc.usage = mUsage;
		desc.indexType = mIndexType;
		desc.skeleton = mSkeleton;
		desc.morphShapes = mMorphShapes;

		MeshCore* obj = new (bs_alloc<MeshCore>()) MeshCore(mCPUData, desc);

		SPtr<CoreObjectCore> meshCore = bs_shared_ptr<MeshCore>(obj);
		meshCore->_setThisPtr(meshCore);

		if ((mUsage & MU_CPUCACHED) == 0)
			mCPUData = nullptr;

		return meshCore;
	}

	void Mesh::updateCPUBuffer(UINT32 subresourceIdx, const MeshData& pixelData)
	{
		if ((mUsage & MU_CPUCACHED) == 0)
			return;

		if (subresourceIdx > 0)
		{
			LOGERR("Invalid subresource index: " + toString(subresourceIdx) + ". Supported range: 0 .. 1.");
			return;
		}

		if (pixelData.getNumIndices() != mProperties.getNumIndices() ||
			pixelData.getNumVertices() != mProperties.getNumVertices() ||
			pixelData.getIndexType() != mIndexType ||
			pixelData.getVertexDesc()->getVertexStride() != mVertexDesc->getVertexStride())
		{
			LOGERR("Provided buffer is not of valid dimensions or format in order to update this mesh.");
			return;
		}

		if (mCPUData->getSize() != pixelData.getSize())
			BS_EXCEPT(InternalErrorException, "Buffer sizes don't match.");

		UINT8* dest = mCPUData->getData();
		UINT8* src = pixelData.getData();

		memcpy(dest, src, pixelData.getSize());
	}

	void Mesh::readData(MeshData& dest)
	{
		if ((mUsage & MU_CPUCACHED) == 0)
		{
			LOGERR("Attempting to read CPU data from a mesh that is created without CPU caching.");
			return;
		}

		if (dest.getNumIndices() != mProperties.getNumIndices() ||
			dest.getNumVertices() != mProperties.getNumVertices() ||
			dest.getIndexType() != mIndexType ||
			dest.getVertexDesc()->getVertexStride() != mVertexDesc->getVertexStride())
		{
			LOGERR("Provided buffer is not of valid dimensions or format in order to read from this mesh.");
			return;
		}
		
		if (mCPUData->getSize() != dest.getSize())
			BS_EXCEPT(InternalErrorException, "Buffer sizes don't match.");

		UINT8* srcPtr = mCPUData->getData();
		UINT8* destPtr = dest.getData();

		memcpy(destPtr, srcPtr, dest.getSize());
	}

	void Mesh::createCPUBuffer()
	{
		mCPUData = allocateSubresourceBuffer(0);
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

	HMesh Mesh::create(UINT32 numVertices, UINT32 numIndices, const SPtr<VertexDataDesc>& vertexDesc, 
		int usage, DrawOperationType drawOp, IndexType indexType)
	{
		MESH_DESC desc;
		desc.numVertices = numVertices;
		desc.numIndices = numIndices;
		desc.vertexDesc = vertexDesc;
		desc.usage = usage;
		desc.subMeshes.push_back(SubMesh(0, numIndices, drawOp));
		desc.indexType = indexType;

		SPtr<Mesh> meshPtr = _createPtr(desc);
		return static_resource_cast<Mesh>(gResources()._createResourceHandle(meshPtr));
	}

	HMesh Mesh::create(const MESH_DESC& desc)
	{
		SPtr<Mesh> meshPtr = _createPtr(desc);
		return static_resource_cast<Mesh>(gResources()._createResourceHandle(meshPtr));
	}

	HMesh Mesh::create(const SPtr<MeshData>& initialMeshData, const MESH_DESC& desc)
	{
		SPtr<Mesh> meshPtr = _createPtr(initialMeshData, desc);
		return static_resource_cast<Mesh>(gResources()._createResourceHandle(meshPtr));
	}

	SPtr<Mesh> Mesh::_createPtr(const MESH_DESC& desc)
	{
		SPtr<Mesh> mesh = bs_core_ptr<Mesh>(new (bs_alloc<Mesh>()) Mesh(desc));
		mesh->_setThisPtr(mesh);
		mesh->initialize();

		return mesh;
	}

	SPtr<Mesh> Mesh::_createPtr(const SPtr<MeshData>& initialMeshData, const MESH_DESC& desc)
	{
		SPtr<Mesh> mesh = bs_core_ptr<Mesh>(new (bs_alloc<Mesh>()) Mesh(initialMeshData, desc));
		mesh->_setThisPtr(mesh);
		mesh->initialize();

		return mesh;
	}

	SPtr<Mesh> Mesh::createEmpty()
	{
		SPtr<Mesh> mesh = bs_core_ptr<Mesh>(new (bs_alloc<Mesh>()) Mesh());
		mesh->_setThisPtr(mesh);

		return mesh;
	}
}