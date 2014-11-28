#include "BsViewport.h"
#include "BsViewportRTTI.h"
#include "BsException.h"
#include "BsRenderTarget.h"
#include "BsMath.h"
#include "BsRenderAPI.h"
#include "BsFrameAlloc.h"

namespace BansheeEngine 
{
	const Color Viewport::DEFAULT_CLEAR_COLOR = Color(143.0f / 255.0f, 111.0f / 255.0f, 0);

	ViewportBase::ViewportBase(float x, float y, float width, float height)
         :mNormArea(x, y, width, height), mClearColor(DEFAULT_CLEAR_COLOR), mRequiresColorClear(true), 
		 mRequiresDepthClear(true), mRequiresStencilClear(false), mStencilClearValue(0), mDepthClearValue(1.0f)
    {

    }

	void ViewportBase::setArea(float x, float y, float width, float height)
    {
        mNormArea.x = x;
        mNormArea.y = y;
        mNormArea.width = width;
        mNormArea.height = height;

		_markCoreDirty();
    }

	Rect2I ViewportBase::getArea() const
	{
		float width = (float)getTargetWidth();
		float height = (float)getTargetHeight();
		
		Rect2I area;
		area.x = (int)(mNormArea.x * width);
		area.y = (int)(mNormArea.y * height);
		area.width = (int)(mNormArea.width * width);
		area.height = (int)(mNormArea.height * height);

		return area;
	}

	void ViewportBase::setRequiresClear(bool colorClear, bool depthClear, bool stencilClear)
	{
		mRequiresColorClear = colorClear;
		mRequiresDepthClear = depthClear;
		mRequiresStencilClear = stencilClear;

		_markCoreDirty();
	}

	void ViewportBase::setClearValues(const Color& clearColor, float clearDepth, UINT16 clearStencil)
	{
		mClearColor = clearColor;
		mDepthClearValue = clearDepth;
		mStencilClearValue = clearStencil;

		_markCoreDirty();
	}

	INT32 ViewportBase::getX() const
	{ 
		return (INT32)(mNormArea.x * getTargetWidth());
	}

	INT32 ViewportBase::getY() const
	{ 
		return (INT32)(mNormArea.y * getTargetHeight());
	}

	INT32 ViewportBase::getWidth() const
	{ 
		return (INT32)(mNormArea.width * getTargetWidth());
	}

	INT32 ViewportBase::getHeight() const
	{ 
		return (INT32)(mNormArea.height * getTargetHeight());
	}

	ViewportCore::ViewportCore(const SPtr<RenderTargetCore>& target, float x, float y, float width, float height)
		:ViewportBase(x, y, width, height), mTarget(target)
	{

	}

	SPtr<ViewportCore> ViewportCore::create(const SPtr<RenderTargetCore>& target, float x, float y, float width, float height)
	{
		ViewportCore* viewport = new (bs_alloc<Viewport>()) ViewportCore(target, x, y, width, height);

		SPtr<ViewportCore> viewportPtr = bs_shared_ptr<ViewportCore, GenAlloc>(viewport);
		viewportPtr->_setThisPtr(viewportPtr);
		viewportPtr->initialize();

		return viewportPtr;
	}

	UINT32 ViewportCore::getTargetWidth() const
	{
		return mTarget->getProperties().getWidth();
	}

	UINT32 ViewportCore::getTargetHeight() const
	{
		return mTarget->getProperties().getHeight();
	}

	void ViewportCore::syncToCore(const CoreSyncData& data)
	{
		char* dataPtr = (char*)data.getBuffer();
		dataPtr = rttiReadElem(mNormArea, dataPtr);
		dataPtr = rttiReadElem(mRequiresColorClear, dataPtr);
		dataPtr = rttiReadElem(mRequiresDepthClear, dataPtr);
		dataPtr = rttiReadElem(mRequiresStencilClear, dataPtr);
		dataPtr = rttiReadElem(mClearColor, dataPtr);
		dataPtr = rttiReadElem(mDepthClearValue, dataPtr);
		dataPtr = rttiReadElem(mStencilClearValue, dataPtr);

		SPtr<RenderTargetCore>* rtPtr = (SPtr<RenderTargetCore>*)dataPtr;
		mTarget = *rtPtr;

		rtPtr->~SPtr<RenderTargetCore>();
	}

	Viewport::Viewport()
		:ViewportBase()
	{ }

	Viewport::Viewport(const RenderTargetPtr& target, float x, float y, float width, float height)
		:ViewportBase(x, y, width, height), mTarget(target)
	{

	}

	SPtr<ViewportCore> Viewport::getCore() const
	{
		return std::static_pointer_cast<ViewportCore>(mCoreSpecific);
	}

	void Viewport::_markCoreDirty()
	{
		markCoreDirty();
	}

	UINT32 Viewport::getTargetWidth() const
	{
		return mTarget->getProperties().getWidth();
	}

	UINT32 Viewport::getTargetHeight() const
	{
		return mTarget->getProperties().getHeight();
	}

	SPtr<CoreObjectCore> Viewport::createCore() const
	{
		ViewportCore* viewport = new (bs_alloc<Viewport>()) 
			ViewportCore(mTarget->getCore(), mNormArea.x, mNormArea.y, mNormArea.width, mNormArea.height);

		SPtr<ViewportCore> viewportPtr = bs_shared_ptr<ViewportCore, GenAlloc>(viewport);
		viewportPtr->_setThisPtr(viewportPtr);

		return viewportPtr;
	}

	CoreSyncData Viewport::syncToCore(FrameAlloc* allocator)
	{
		UINT32 size = 0;
		size += rttiGetElemSize(mNormArea);
		size += rttiGetElemSize(mRequiresColorClear);
		size += rttiGetElemSize(mRequiresDepthClear);
		size += rttiGetElemSize(mRequiresStencilClear);
		size += rttiGetElemSize(mClearColor);
		size += rttiGetElemSize(mDepthClearValue);
		size += rttiGetElemSize(mStencilClearValue);
		size += sizeof(SPtr<RenderTargetCore>);

		UINT8* buffer = allocator->alloc(size);

		char* dataPtr = (char*)buffer;
		dataPtr = rttiWriteElem(mNormArea, dataPtr);
		dataPtr = rttiWriteElem(mRequiresColorClear, dataPtr);
		dataPtr = rttiWriteElem(mRequiresDepthClear, dataPtr);
		dataPtr = rttiWriteElem(mRequiresStencilClear, dataPtr);
		dataPtr = rttiWriteElem(mClearColor, dataPtr);
		dataPtr = rttiWriteElem(mDepthClearValue, dataPtr);
		dataPtr = rttiWriteElem(mStencilClearValue, dataPtr);

		SPtr<RenderTargetCore>* rtPtr = new (dataPtr) SPtr<RenderTargetCore>();
		if (mTarget != nullptr)
			*rtPtr = mTarget->getCore();
		else
			*rtPtr = nullptr;

		return CoreSyncData(buffer, size);
	}

	ViewportPtr Viewport::create(const RenderTargetPtr& target, float x, float y, float width, float height)
	{
		Viewport* viewport = new (bs_alloc<Viewport>()) Viewport(target, x, y, width, height);
		ViewportPtr viewportPtr = bs_core_ptr<Viewport, GenAlloc>(viewport);
		viewportPtr->_setThisPtr(viewportPtr);
		viewportPtr->initialize();

		return viewportPtr;
	}

	ViewportPtr Viewport::createEmpty()
	{
		Viewport* viewport = new (bs_alloc<Viewport>()) Viewport();
		ViewportPtr viewportPtr = bs_core_ptr<Viewport, GenAlloc>(viewport);
		viewportPtr->_setThisPtr(viewportPtr);

		return viewportPtr;
	}

	RTTITypeBase* Viewport::getRTTIStatic()
	{
		return ViewportRTTI::instance();
	}

	RTTITypeBase* Viewport::getRTTI() const
	{
		return Viewport::getRTTIStatic();
	}
}
