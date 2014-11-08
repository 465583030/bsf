#include "BsRenderSystem.h"

#include "BsCoreThread.h"
#include "BsViewport.h"
#include "BsException.h"
#include "BsRenderTarget.h"
#include "BsRenderWindow.h"
#include "BsPixelBuffer.h"
#include "BsOcclusionQuery.h"
#include "BsGpuResource.h"
#include "BsCoreThread.h"
#include "BsMesh.h"
#include "BsProfilerCPU.h"
#include "BsRenderStats.h"

using namespace std::placeholders;

namespace BansheeEngine 
{
    static const TexturePtr sNullTexPtr;

    RenderSystem::RenderSystem()
        : mCullingMode(CULL_COUNTERCLOCKWISE)
        , mDisabledTexUnitsFrom(0)
        , mVertexProgramBound(false)
		, mGeometryProgramBound(false)
        , mFragmentProgramBound(false)
		, mDomainProgramBound(false)
		, mHullProgramBound(false)
		, mComputeProgramBound(false)
		, mClipPlanesDirty(true)
		, mCurrentCapabilities(nullptr)
    {
    }

    RenderSystem::~RenderSystem()
    {
		// Base classes need to call virtual destroy_internal method instead of a destructor

		bs_delete(mCurrentCapabilities);
		mCurrentCapabilities = nullptr;
    }

	RenderWindowPtr RenderSystem::initialize(const RENDER_WINDOW_DESC& primaryWindowDesc)
	{
		mPrimaryWindowDesc = primaryWindowDesc;

		AsyncOp op = gCoreThread().queueReturnCommand(std::bind(&RenderSystem::initialize_internal, this, _1), true);

		return op.getReturnValue<RenderWindowPtr>();
	}

	void RenderSystem::initialize_internal(AsyncOp& asyncOp)
	{
		THROW_IF_NOT_CORE_THREAD;

		mVertexProgramBound = false;
		mGeometryProgramBound = false;
		mFragmentProgramBound = false;
		mDomainProgramBound = false;
		mHullProgramBound = false;
		mComputeProgramBound = false;
	}

	void RenderSystem::destroy()
	{
		gCoreAccessor().queueCommand(std::bind(&RenderSystem::destroy_internal, this));
		gCoreThread().submitAccessors(true);
	}

	void RenderSystem::destroy_internal()
	{
		mActiveRenderTarget = nullptr;
	}

	const RenderSystemCapabilities* RenderSystem::getCapabilities(void) const 
	{ 
		return mCurrentCapabilities; 
	}

	const DriverVersion& RenderSystem::getDriverVersion(void) const 
	{ 
		THROW_IF_NOT_CORE_THREAD;

		return mDriverVersion; 
	}

    void RenderSystem::disableTextureUnit(GpuProgramType gptype, UINT16 texUnit)
    {
		THROW_IF_NOT_CORE_THREAD;

        setTexture(gptype, texUnit, false, sNullTexPtr);
    }

	void RenderSystem::addClipPlane (const Plane &p)
	{
		THROW_IF_NOT_CORE_THREAD;

		mClipPlanes.push_back(p);
		mClipPlanesDirty = true;
	}

	void RenderSystem::setClipPlanes(const PlaneList& clipPlanes)
	{
		THROW_IF_NOT_CORE_THREAD;

		if (clipPlanes != mClipPlanes)
		{
			mClipPlanes = clipPlanes;
			mClipPlanesDirty = true;
		}
	}

	void RenderSystem::resetClipPlanes()
	{
		THROW_IF_NOT_CORE_THREAD;

		if (!mClipPlanes.empty())
		{
			mClipPlanes.clear();
			mClipPlanesDirty = true;
		}
	}

	void RenderSystem::bindGpuProgram(HGpuProgram prg)
	{
		THROW_IF_NOT_CORE_THREAD;

		switch(prg->getType())
		{
		case GPT_VERTEX_PROGRAM:
			if (!mVertexProgramBound && !mClipPlanes.empty())
				mClipPlanesDirty = true;

			mVertexProgramBound = true;
			break;
		case GPT_GEOMETRY_PROGRAM:
			mGeometryProgramBound = true;
			break;
		case GPT_FRAGMENT_PROGRAM:
			mFragmentProgramBound = true;
			break;
		case GPT_DOMAIN_PROGRAM:
			mDomainProgramBound = true;
			break;
		case GPT_HULL_PROGRAM:
			mHullProgramBound = true;
			break;
		case GPT_COMPUTE_PROGRAM:
			mComputeProgramBound = true;
			break;
		}
	}

	void RenderSystem::unbindGpuProgram(GpuProgramType gptype)
	{
		THROW_IF_NOT_CORE_THREAD;

		switch(gptype)
		{
		case GPT_VERTEX_PROGRAM:
			if (mVertexProgramBound && !mClipPlanes.empty())
				mClipPlanesDirty = true;

			mVertexProgramBound = false;
			break;
		case GPT_GEOMETRY_PROGRAM:
			mGeometryProgramBound = false;
			break;
		case GPT_FRAGMENT_PROGRAM:
			mFragmentProgramBound = false;
			break;
		case GPT_DOMAIN_PROGRAM:
			mDomainProgramBound = false;
			break;
		case GPT_HULL_PROGRAM:
			mHullProgramBound = false;
			break;
		case GPT_COMPUTE_PROGRAM:
			mComputeProgramBound = false;
			break;
		}
	}

	bool RenderSystem::isGpuProgramBound(GpuProgramType gptype)
	{
		THROW_IF_NOT_CORE_THREAD;

	    switch(gptype)
	    {
        case GPT_VERTEX_PROGRAM:
            return mVertexProgramBound;
        case GPT_GEOMETRY_PROGRAM:
            return mGeometryProgramBound;
        case GPT_FRAGMENT_PROGRAM:
            return mFragmentProgramBound;
		case GPT_DOMAIN_PROGRAM:
			return mDomainProgramBound;
		case GPT_HULL_PROGRAM:
			return mHullProgramBound;
		case GPT_COMPUTE_PROGRAM:
			return mComputeProgramBound;
	    }

        return false;
	}

	void RenderSystem::render(const MeshBasePtr& mesh, UINT32 indexOffset, UINT32 indexCount, bool useIndices, DrawOperationType drawOp)
	{
		THROW_IF_NOT_CORE_THREAD;

		gProfilerCPU().beginSample("render");

		if (mClipPlanesDirty)
		{
			setClipPlanesImpl(mClipPlanes);
			mClipPlanesDirty = false;
		}

		std::shared_ptr<VertexData> vertexData = mesh->_getVertexData();

		setVertexDeclaration(vertexData->vertexDeclaration);
		auto vertexBuffers = vertexData->getBuffers();

		if(vertexBuffers.size() > 0)
		{
			VertexBufferPtr buffers[MAX_BOUND_VERTEX_BUFFERS];

			UINT32 endSlot = 0;
			UINT32 startSlot = MAX_BOUND_VERTEX_BUFFERS;
			for(auto iter = vertexBuffers.begin(); iter != vertexBuffers.end() ; ++iter)
			{
				if(iter->first >= MAX_BOUND_VERTEX_BUFFERS)
					BS_EXCEPT(InvalidParametersException, "Buffer index out of range");

				startSlot = std::min(iter->first, startSlot);
				endSlot = std::max(iter->first, endSlot);
			}

			for(auto iter = vertexBuffers.begin(); iter != vertexBuffers.end() ; ++iter)
			{
				buffers[iter->first - startSlot] = iter->second;
			}

			setVertexBuffers(startSlot, buffers, endSlot - startSlot + 1);
		}

		setDrawOperation(drawOp);

		if (useIndices)
		{
			SPtr<IndexBufferCore> indexBuffer = mesh->_getIndexBuffer()->getCore();
			const IndexBufferProperties& ibProps = indexBuffer->getProperties();

			if(indexCount == 0)
				indexCount = ibProps.getNumIndices();

			setIndexBuffer(mesh->_getIndexBuffer());
			drawIndexed(indexOffset + mesh->_getIndexOffset(), indexCount, mesh->_getVertexOffset(), vertexData->vertexCount);
		}
		else
			draw(mesh->_getVertexOffset(), vertexData->vertexCount);

		mesh->_notifyUsedOnGPU();

		gProfilerCPU().endSample("render");
	}

	void RenderSystem::swapBuffers(RenderTargetPtr target)
	{
		THROW_IF_NOT_CORE_THREAD;

		if (target->isInitialized())
		{
			target->getCore()->swapBuffers();

			BS_INC_RENDER_STAT(NumPresents);
		}
	}

	void RenderSystem::writeSubresource(GpuResourcePtr resource, UINT32 subresourceIdx, const GpuResourceDataPtr& data, bool discardEntireBuffer, AsyncOp& asyncOp)
	{
		THROW_IF_NOT_CORE_THREAD;

		gProfilerCPU().beginSample("writeSubresource");

		resource->writeSubresource(subresourceIdx, *data, discardEntireBuffer);

		gProfilerCPU().endSample("writeSubresource");

		gProfilerCPU().beginSample("writeSubresourceB");

		data->_unlock();
		asyncOp._completeOperation();

		gProfilerCPU().endSample("writeSubresourceB");
	}

	void RenderSystem::readSubresource(GpuResourcePtr resource, UINT32 subresourceIdx, GpuResourceDataPtr& data, AsyncOp& asyncOp)
	{
		THROW_IF_NOT_CORE_THREAD;

		gProfilerCPU().beginSample("readSubresource");

		resource->readSubresource(subresourceIdx, *data);
		data->_unlock();
		asyncOp._completeOperation();

		gProfilerCPU().endSample("readSubresource");
	}
}