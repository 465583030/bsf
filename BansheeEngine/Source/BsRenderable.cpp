#include "BsRenderable.h"
#include "BsRenderableRTTI.h"
#include "BsSceneObject.h"
#include "BsBuiltinResources.h"
#include "BsMesh.h"
#include "BsMaterial.h"
#include "BsRenderQueue.h"
#include "BsBounds.h"
#include "BsRenderer.h"
#include "BsFrameAlloc.h"
#include "BsDebug.h"

namespace BansheeEngine
{
	template<class T>
	bool isMeshValid(const T& mesh) { return false; }

	template<>
	bool isMeshValid(const HMesh& mesh) { return mesh.isLoaded(); }

	template<>
	bool isMeshValid(const SPtr<MeshCore>& mesh) { return mesh != nullptr; }

	template<bool Core>
	TRenderable<Core>::TRenderable()
		:mLayer(1), mTransform(Matrix4::IDENTITY), mTransformNoScale(Matrix4::IDENTITY), mIsActive(true)
	{
		mMaterials.resize(1);
	}

	template<bool Core>
	TRenderable<Core>::~TRenderable()
	{

	}

	template<bool Core>
	void TRenderable<Core>::setMesh(const MeshType& mesh)
	{
		mMesh = mesh;

		int numSubMeshes = 0;
		if (isMeshValid(mesh))
			numSubMeshes = mesh->getProperties().getNumSubMeshes();

		mMaterials.resize(numSubMeshes);

		_markDependenciesDirty();
		_markResourcesDirty();
		_markCoreDirty();
	}

	template<bool Core>
	void TRenderable<Core>::setMaterial(UINT32 idx, const MaterialType& material)
	{
		if (idx >= (UINT32)mMaterials.size())
			return;

		mMaterials[idx] = material;

		_markDependenciesDirty();
		_markResourcesDirty();
		_markCoreDirty();
	}

	template<bool Core>
	void TRenderable<Core>::setMaterials(const Vector<MaterialType>& materials)
	{
		UINT32 numMaterials = (UINT32)mMaterials.size();
		UINT32 min = std::min(numMaterials, (UINT32)materials.size());

		for (UINT32 i = 0; i < min; i++)
			mMaterials[i] = materials[i];

		for (UINT32 i = min; i < numMaterials; i++)
			mMaterials[i] = nullptr;

		_markDependenciesDirty();
		_markResourcesDirty();
		_markCoreDirty();
	}

	template<bool Core>
	void TRenderable<Core>::setMaterial(const MaterialType& material)
	{
		setMaterial(0, material);
	}

	template<bool Core>
	void TRenderable<Core>::setLayer(UINT64 layer)
	{
		bool isPow2 = layer && !((layer - 1) & layer);

		if (!isPow2)
		{
			LOGWRN("Invalid layer provided. Only one layer bit may be set. Ignoring.");
			return;
		}

		mLayer = layer;
		_markCoreDirty();
	}

	template<bool Core>
	void TRenderable<Core>::setTransform(const Matrix4& transform, const Matrix4& transformNoScale)
	{
		mTransform = transform;
		mTransformNoScale = transformNoScale;
		_markCoreDirty(RenderableDirtyFlag::Transform);
	}

	template<bool Core>
	void TRenderable<Core>::setPosition(const Vector3& position)
	{
		mPosition = position;
		_markCoreDirty(RenderableDirtyFlag::Transform);
	}

	template<bool Core>
	void TRenderable<Core>::setIsActive(bool active)
	{
		mIsActive = active;
		_markCoreDirty();
	}

	template class TRenderable < false >;
	template class TRenderable < true >;

	RenderableCore::RenderableCore() 
		:mRendererId(0)
	{
	}

	RenderableCore::~RenderableCore()
	{
		if (mIsActive)
			gRenderer()->_notifyRenderableRemoved(this);
	}

	void RenderableCore::initialize()
	{
		gRenderer()->_notifyRenderableAdded(this);

		CoreObjectCore::initialize();
	}

	Bounds RenderableCore::getBounds() const
	{
		SPtr<MeshCore> mesh = getMesh();

		if (mesh == nullptr)
		{
			AABox box(mPosition, mPosition);
			Sphere sphere(mPosition, 0.0f);

			return Bounds(box, sphere);
		}
		else
		{
			Bounds bounds = mesh->getProperties().getBounds();
			bounds.transformAffine(mTransform);

			return bounds;
		}
	}

	void RenderableCore::syncToCore(const CoreSyncData& data)
	{
		char* dataPtr = (char*)data.getBuffer();

		mWorldBounds.clear();
		mMaterials.clear();

		UINT32 numMaterials = 0;
		UINT32 dirtyFlags = 0;
		bool oldIsActive = mIsActive;

		dataPtr = rttiReadElem(mLayer, dataPtr);
		dataPtr = rttiReadElem(mWorldBounds, dataPtr);
		dataPtr = rttiReadElem(numMaterials, dataPtr);
		dataPtr = rttiReadElem(mTransform, dataPtr);
		dataPtr = rttiReadElem(mTransformNoScale, dataPtr);
		dataPtr = rttiReadElem(mPosition, dataPtr);
		dataPtr = rttiReadElem(mIsActive, dataPtr);
		dataPtr = rttiReadElem(dirtyFlags, dataPtr);

		SPtr<MeshCore>* mesh = (SPtr<MeshCore>*)dataPtr;
		mMesh = *mesh;
		mesh->~SPtr<MeshCore>();
		dataPtr += sizeof(SPtr<MeshCore>);

		for (UINT32 i = 0; i < numMaterials; i++)
		{
			SPtr<MaterialCore>* material = (SPtr<MaterialCore>*)dataPtr;
			mMaterials.push_back(*material);
			material->~SPtr<MaterialCore>();
			dataPtr += sizeof(SPtr<MaterialCore>);
		}

		if (dirtyFlags == (UINT32)RenderableDirtyFlag::Transform)
		{
			if (mIsActive)
				gRenderer()->_notifyRenderableUpdated(this);
		}
		else
		{
			if (oldIsActive != mIsActive)
			{
				if (mIsActive)
					gRenderer()->_notifyRenderableAdded(this);
				else
					gRenderer()->_notifyRenderableRemoved(this);
			}
			else
			{
				gRenderer()->_notifyRenderableRemoved(this);
				gRenderer()->_notifyRenderableAdded(this);
			}
		}
	}

	Renderable::Renderable()
		:mLastUpdateHash(0)
	{
		
	}

	Bounds Renderable::getBounds() const
	{
		HMesh mesh = getMesh();

		if (!mesh.isLoaded())
		{
			AABox box(mPosition, mPosition);
			Sphere sphere(mPosition, 0.0f);

			return Bounds(box, sphere);
		}
		else
		{
			Bounds bounds = mesh->getProperties().getBounds();
			bounds.transformAffine(mTransform);

			return bounds;
		}
	}

	SPtr<RenderableCore> Renderable::getCore() const
	{
		return std::static_pointer_cast<RenderableCore>(mCoreSpecific);
	}

	SPtr<CoreObjectCore> Renderable::createCore() const
	{
		RenderableCore* handler = new (bs_alloc<RenderableCore>()) RenderableCore();
		SPtr<RenderableCore> handlerPtr = bs_shared_ptr<RenderableCore>(handler);
		handlerPtr->_setThisPtr(handlerPtr);

		return handlerPtr;
	}

	void Renderable::_markCoreDirty(RenderableDirtyFlag flag)
	{
		markCoreDirty((UINT32)flag);
	}

	void Renderable::_markDependenciesDirty()
	{
		markDependenciesDirty();
	}

	void Renderable::_markResourcesDirty()
	{
		markListenerResourcesDirty();
	}

	CoreSyncData Renderable::syncToCore(FrameAlloc* allocator)
	{
		UINT32 numMaterials = (UINT32)mMaterials.size();

		UINT32 size = rttiGetElemSize(mLayer) + 
			rttiGetElemSize(mWorldBounds) + 
			rttiGetElemSize(numMaterials) + 
			rttiGetElemSize(mTransform) +
			rttiGetElemSize(mTransformNoScale) +
			rttiGetElemSize(mPosition) +
			rttiGetElemSize(mIsActive) +
			rttiGetElemSize(getCoreDirtyFlags()) +
			sizeof(SPtr<MeshCore>) + 
			numMaterials * sizeof(SPtr<MaterialCore>);

		UINT8* data = allocator->alloc(size);
		char* dataPtr = (char*)data;
		dataPtr = rttiWriteElem(mLayer, dataPtr);
		dataPtr = rttiWriteElem(mWorldBounds, dataPtr);
		dataPtr = rttiWriteElem(numMaterials, dataPtr);
		dataPtr = rttiWriteElem(mTransform, dataPtr);
		dataPtr = rttiWriteElem(mTransformNoScale, dataPtr);
		dataPtr = rttiWriteElem(mPosition, dataPtr);
		dataPtr = rttiWriteElem(mIsActive, dataPtr);
		dataPtr = rttiWriteElem(getCoreDirtyFlags(), dataPtr);

		SPtr<MeshCore>* mesh = new (dataPtr) SPtr<MeshCore>();
		if (mMesh.isLoaded())
			*mesh = mMesh->getCore();

		dataPtr += sizeof(SPtr<MeshCore>);

		for (UINT32 i = 0; i < numMaterials; i++)
		{
			SPtr<MaterialCore>* material = new (dataPtr)SPtr<MaterialCore>();
			if (mMaterials[i].isLoaded())
				*material = mMaterials[i]->getCore();

			dataPtr += sizeof(SPtr<MaterialCore>);
		}

		return CoreSyncData(data, size);
	}

	void Renderable::getCoreDependencies(Vector<CoreObject*>& dependencies)
	{
		if (mMesh.isLoaded())
			dependencies.push_back(mMesh.get());

		for (auto& material : mMaterials)
		{
			if (material.isLoaded())
				dependencies.push_back(material.get());
		}
	}

	void Renderable::getListenerResources(Vector<HResource>& resources)
	{
		if (mMesh != nullptr)
			resources.push_back(mMesh);

		for (auto& material : mMaterials)
		{
			if (material != nullptr)
				resources.push_back(material);
		}
	}

	void Renderable::notifyResourceLoaded(const HResource& resource)
	{
		markDependenciesDirty();
		markCoreDirty();
	}

	void Renderable::notifyResourceChanged(const HResource& resource)
	{
		markDependenciesDirty();
		markCoreDirty();
	}

	RenderablePtr Renderable::create()
	{
		RenderablePtr handlerPtr = createEmpty();
		handlerPtr->initialize();

		return handlerPtr;
	}

	RenderablePtr Renderable::createEmpty()
	{
		Renderable* handler = new (bs_alloc<Renderable>()) Renderable();
		RenderablePtr handlerPtr = bs_core_ptr<Renderable>(handler);
		handlerPtr->_setThisPtr(handlerPtr);

		return handlerPtr;
	}

	RTTITypeBase* Renderable::getRTTIStatic()
	{
		return RenderableRTTI::instance();
	}

	RTTITypeBase* Renderable::getRTTI() const
	{
		return Renderable::getRTTIStatic();
	}
}