#include "BsGUIViewport.h"
#include "BsGUIWidget.h"
#include "BsGUISkin.h"
#include "BsSpriteTexture.h"
#include "BsGUILayoutOptions.h"
#include "BsCamera.h"
#include "BsViewport.h"
#include "BsRenderTarget.h"
#include "BsException.h"

namespace BansheeEngine
{
	const String& GUIViewport::getGUITypeName()
	{
		static String name = "Viewport";
		return name;
	}

	GUIViewport::GUIViewport(const String& styleName, const HCamera& camera, 
		float aspectRatio, Degree fieldOfView, const GUILayoutOptions& layoutOptions)
		:GUIElement(styleName, layoutOptions), mCamera(camera), mAspectRatio(aspectRatio),
		mFieldOfView(fieldOfView)
	{
		mVerticalFOV = 2.0f * Math::atan(Math::tan(mFieldOfView.valueRadians() * 0.5f) * (1.0f / mAspectRatio));
	}

	GUIViewport::~GUIViewport()
	{

	}

	GUIViewport* GUIViewport::create(const HCamera& camera, float aspectRatio, Degree fieldOfView, const String& styleName)
	{
		return new (bs_alloc<GUIViewport, PoolAlloc>()) GUIViewport(getStyleName<GUIViewport>(styleName), camera, aspectRatio, fieldOfView, GUILayoutOptions::create());
	}

	GUIViewport* GUIViewport::create(const GUIOptions& layoutOptions, const HCamera& camera, 
		float aspectRatio, Degree fieldOfView, const String& styleName)
	{
		return new (bs_alloc<GUIViewport, PoolAlloc>()) GUIViewport(getStyleName<GUIViewport>(styleName), camera, aspectRatio, fieldOfView, GUILayoutOptions::create(layoutOptions));
	}

	UINT32 GUIViewport::getNumRenderElements() const
	{
		return 0;
	}

	const GUIMaterialInfo& GUIViewport::getMaterial(UINT32 renderElementIdx) const
	{
		BS_EXCEPT(InternalErrorException, "This element has no render element so no material can be retrieved.");
	}

	UINT32 GUIViewport::getNumQuads(UINT32 renderElementIdx) const
	{
		return 0;
	}

	void GUIViewport::updateClippedBounds()
	{
		RectI mBounds = RectI(0, 0, mWidth, mHeight);
		mBounds.clip(mClipRect);
		mBounds.x += mOffset.x;
		mBounds.y += mOffset.y;
	}

	Vector2I GUIViewport::_getOptimalSize() const
	{
		return Vector2I(0, 0);
	}

	void GUIViewport::fillBuffer(UINT8* vertices, UINT8* uv, UINT32* indices, UINT32 startingQuad, UINT32 maxNumQuads, 
		UINT32 vertexStride, UINT32 indexStride, UINT32 renderElementIdx) const
	{

	}

	void GUIViewport::updateRenderElementsInternal()
	{
		// TODO - This doesn't get called if element mesh is dirty!!! and I need to update the viewport when offset changes (in which case mesh is marked as dirty)
		float currentAspect = mWidth / (float)mHeight;
		Radian currentFOV = 2.0f * Math::atan(Math::tan(mVerticalFOV * 0.5f) * currentAspect);

		mCamera->setHorzFOV(currentFOV);

		ViewportPtr viewport = mCamera->getViewport();
		RenderTargetPtr renderTarget = viewport->getTarget();

		float x = mOffset.x / (float)renderTarget->getWidth();
		float y = mOffset.y / (float)renderTarget->getHeight();
		float width = mWidth / (float)renderTarget->getWidth();
		float height = mHeight / (float)renderTarget->getHeight();

		viewport->setArea(x, y, width, height);
	}

	void GUIViewport::_changeParentWidget(GUIWidget* widget)
	{
		GUIElement::_changeParentWidget(widget);

		if(widget != nullptr)
		{
			RenderTargetPtr guiRenderTarget = widget->getTarget()->getTarget();
			RenderTargetPtr cameraRenderTarget = mCamera->getViewport()->getTarget();

			if(guiRenderTarget != cameraRenderTarget)
				BS_EXCEPT(InvalidParametersException, "Camera provided to GUIViewport must use the same render target as the GUIWidget this element is located on.")
		}
	}
}