#pragma once

#include "BsCorePrerequisites.h"
#include "BsColor.h"
#include "BsRectI.h"
#include "BsRectF.h"
#include "BsEvent.h"

namespace BansheeEngine 
{
	/**
	 * @brief	Viewport provides you with a way to render to only a part of a 
	 * 			RenderTarget. It also allows you to set up color/depth/stencil
	 * 			clear values for that specific region.
	 */
	class BS_CORE_EXPORT Viewport
    {
    public:       
		Viewport();
        
        /**
         * @brief	Constructs a new viewport.
         *
         * @note	Viewport coordinates are normalized in [0, 1] range.
         */
        Viewport(const RenderTargetPtr& target, float x = 0.0f, float y = 0.0f, float width = 1.0f, float height = 1.0f);
        virtual ~Viewport();

        /**
         * @brief	Returns the render target the viewport is associated with.
         */
        RenderTargetPtr getTarget() const { return mTarget; }

        /**
         * @brief	Gets the normalized x coordinate of the viewport, in [0, 1] range.
         */
        float getNormalizedX() const { return mNormArea.x; }

        /**
         * @brief	Gets the normalized y coordinate of the viewport, in [0, 1] range.
         */
        float getNormalizedY() const { return mNormArea.y; }

        /**
         * @brief	Gets the normalized width of the viewport, in [0, 1] range.
         */
        float getNormalizedWidth() const { return mNormArea.width; }

		/**
         * @brief	Gets the normalized height of the viewport, in [0, 1] range.
         */
        float getNormalizedHeight() const { return mNormArea.height; }

        /**
         * @brief	Gets the actual x coordinate of the viewport in pixels, in [0, RenderTargetWidth] range.
         */
        INT32 getX() const { return mArea.x; }

        /**
         * @brief	Gets the actual y coordinate of the viewport in pixels, in [0, RenderTargetHeight] range.
         */
        INT32 getY() const { return mArea.y; }

		/**
         * @brief	Gets the actual width coordinate of the viewport in pixels, in [0, RenderTargetWidth] range.
         */
        INT32 getWidth() const { return mArea.width; }

		/**
         * @brief	Gets the actual height coordinate of the viewport in pixels, in [0, RenderTargetHeight] range.
         */
        INT32 getHeight() const { return mArea.height; }
               
        /**
         * @brief	Changes the area that the viewport covers.
         *
         * @note	Viewport coordinates are normalized in [0, 1] range.
         */
        void setArea(float x, float y, float width, float height);

		/**
		 * @brief	Returns actual area of the viewport, in pixels.
		 */
		const RectI& getArea() const { return mArea; }

		/**
		 * @brief	Activates or deactivates clears for color, depth or stencil buffers.
		 *			Buffers will be cleared before rendering to this viewport is performed.
		 */
		void setRequiresClear(bool colorClear, bool depthClear, bool stencilClear);

		/**
		 * @brief	Sets values to clear color, depth and stencil buffers to.
		 */
		void setClearValues(const Color& clearColor, float clearDepth = 0.0f, UINT16 clearStencil = 0);

		/**
		 * @brief	Returns the color to clear the viewport color buffers to.
		 */
		const Color& getClearColor() const { return mClearColor; }

		/**
		 * @brief	Returns the value to clear the viewport depth buffers to.
		 */
		float getClearDepthValue() const { return mDepthClearValue; }


		/**
		 * @brief	Returns the value to clear the viewport stencil buffer to.
		 */
		UINT16 getClearStencilValue() const { return mStencilClearValue; }

		/**
		 * @brief	Returns true if viewport requires color clear before rendering.
		 */
		bool getRequiresColorClear() const { return mRequiresColorClear; }

		/**
		 * @brief	Returns true if viewport requires depth clear before rendering.
		 */
		bool getRequiresDepthClear() const { return mRequiresDepthClear; }

		/**
		 * @brief	Returns true if viewport requires stencil clear before rendering.
		 */
		bool getRequiresStencilClear() const { return mRequiresStencilClear; }

		/**
		 * @brief	Makes an exact copy of this viewport.
		 */
		Viewport clone();
    protected:
        RenderTargetPtr mTarget;

		RectF mNormArea;
		RectI mArea;

		bool mRequiresColorClear;
		bool mRequiresDepthClear;
		bool mRequiresStencilClear;

		Color mClearColor;
		float mDepthClearValue;
		UINT16 mStencilClearValue;

		static const Color DEFAULT_CLEAR_COLOR;

        void updateArea();
		void targetResized();
    };
}