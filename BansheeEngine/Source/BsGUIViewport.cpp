#include "BsGUIViewport.h"
#include "BsCGUIWidget.h"
#include "BsGUISkin.h"
#include "BsSpriteTexture.h"
#include "BsGUIDimensions.h"
#include "BsCCamera.h"
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
		float aspectRatio, Degree fieldOfView, const GUIDimensions& dimensions)
		:GUIElement(styleName, dimensions), mCamera(camera), mAspectRatio(aspectRatio),
		mFieldOfView(fieldOfView)
	{
		mVerticalFOV = 2.0f * Math::atan(Math::tan(mFieldOfView.valueRadians() * 0.5f) * (1.0f / mAspectRatio));
	}

	GUIViewport::~GUIViewport()
	{

	}

	GUIViewport* GUIViewport::create(const HCamera& camera, float aspectRatio, Degree fieldOfView, const String& styleName)
	{
		return new (bs_alloc<GUIViewport>()) GUIViewport(getStyleName<GUIViewport>(styleName), camera, aspectRatio, fieldOfView, GUIDimensions::create());
	}

	GUIViewport* GUIViewport::create(const GUIOptions& options, const HCamera& camera, 
		float aspectRatio, Degree fieldOfView, const String& styleName)
	{
		return new (bs_alloc<GUIViewport>()) GUIViewport(getStyleName<GUIViewport>(styleName), camera, aspectRatio, fieldOfView, GUIDimensions::create(options));
	}

	UINT32 GUIViewport::_getNumRenderElements() const
	{
		return 0;
	}

	const GUIMaterialInfo& GUIViewport::_getMaterial(UINT32 renderElementIdx) const
	{
		BS_EXCEPT(InternalErrorException, "This element has no render element so no material can be retrieved.");
	}

	UINT32 GUIViewport::_getNumQuads(UINT32 renderElementIdx) const
	{
		return 0;
	}

	void GUIViewport::updateClippedBounds()
	{
		mClippedBounds = mLayoutData.area;
		mClippedBounds.clip(mLayoutData.clipRect);
	}

	Vector2I GUIViewport::_getOptimalSize() const
	{
		return Vector2I(0, 0);
	}

	void GUIViewport::_fillBuffer(UINT8* vertices, UINT8* uv, UINT32* indices, UINT32 startingQuad, UINT32 maxNumQuads, 
		UINT32 vertexStride, UINT32 indexStride, UINT32 renderElementIdx) const
	{

	}

	void GUIViewport::updateRenderElementsInternal()
	{
		// TODO - This doesn't get called if element mesh is dirty!!! and I need to update the viewport when offset changes (in which case mesh is marked as dirty)
		float currentAspect = mLayoutData.area.width / (float)mLayoutData.area.height;
		Radian currentFOV = 2.0f * Math::atan(Math::tan(mVerticalFOV * 0.5f) * currentAspect);

		mCamera->setHorzFOV(currentFOV);

		ViewportPtr viewport = mCamera->getViewport();
		RenderTargetPtr renderTarget = viewport->getTarget();
		const RenderTargetProperties& rtProps = renderTarget->getProperties();

		float x = mLayoutData.area.x / (float)rtProps.getWidth();
		float y = mLayoutData.area.y / (float)rtProps.getHeight();
		float width = mLayoutData.area.width / (float)rtProps.getWidth();
		float height = mLayoutData.area.height / (float)rtProps.getHeight();

		viewport->setArea(x, y, width, height);
	}

	void GUIViewport::_changeParentWidget(CGUIWidget* widget)
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