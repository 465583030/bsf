#pragma once

#include "BsPrerequisites.h"
#include "BsGUIElement.h"
#include "BsImageSprite.h"
#include "BsEvent.h"

namespace BansheeEngine
{
	/**
	 * @brief	A handle that can be dragged from its predefined minimum and maximum position,
	 *			either horizontally or vertically.
	 */
	class BS_EXPORT GUISliderHandle : public GUIElement
	{
		/**
		 * @brief	Visual state of the handle
		 */
		enum class State
		{
			Normal, Hover, Active
		};

	public:
		/**
		 * Returns type name of the GUI element used for finding GUI element styles. 
		 */
		static const String& getGUITypeName();

		/**
		 * @brief	Creates a new handle.
		 *
		 * @param	horizontal		Should the handle be movable vertically or horizontally.
		 * @param	jumpOnClick		If true clicking on a specific position on the slider will cause
		 *							the slider handle to jump to that position. Otherwise the slider
		 *							will just slightly move towards that direction.
		 * @param	styleName		Optional style to use for the element. Style will be retrieved
		 *							from GUISkin of the GUIWidget the element is used on. If not specified
		 *							default style is used.
		 */
		static GUISliderHandle* create(bool horizontal, bool jumpOnClick, const String& styleName = StringUtil::BLANK);

		/**
		 * @brief	Creates a new handle.
		 *
		 * @param	horizontal		Should the handle be movable vertically or horizontally.
		 * @param	jumpOnClick		If true clicking on a specific position on the slider will cause
		 *							the slider handle to jump to that position. Otherwise the slider
		 *							will just slightly move towards that direction.
		 * @param	options			Options that allow you to control how is the element positioned and sized.
		 *							This will override any similar options set by style.
		 * @param	styleName		Optional style to use for the element. Style will be retrieved
		 *							from GUISkin of the GUIWidget the element is used on. If not specified
		 *							default style is used.
		 */
		static GUISliderHandle* create(bool horizontal, bool jumpOnClick, const GUIOptions& options,
			const String& styleName = StringUtil::BLANK);

		/**
		 * @brief	Size of the handle in pixels, along the handle drag direction.
		 *
		 * @note	Internal method. Does not trigger layout update.
		 */
		void _setHandleSize(UINT32 size);

		/**
		 * @brief	Moves the handle the the specified position in the handle area.
		 *
		 * @param	pct	Position to move the handle to, in percent ranging [0.0f, 1.0f]
		 *
		 * @note	Internal method. Does not trigger layout update.
		 */
		void _setHandlePos(float pct);

		/**
		 * @brief	Gets the current position of the handle, in percent ranging [0.0f, 1.0f].
		 */
		float getHandlePos() const;

		/**
		 * @brief	Returns remaining length of the scrollable area not covered by the handle, in pixels.
		 */
		UINT32 getScrollableSize() const;

		/**
		 * @brief	Returns the total length of the area the handle can move in, in pixels.
		 */
		UINT32 getMaxSize() const;

		/**
		 * @copydoc	GUIElement::setTint
		 */
		virtual void setTint(const Color& color);

		/**
		 * @copydoc	GUIElement::_getOptimalSize
		 */
		virtual Vector2I _getOptimalSize() const;

		Event<void(float newPosition)> onHandleMoved;
	protected:
		~GUISliderHandle();

		/**
		 * @copydoc GUIElement::getNumRenderElements()
		 */
		virtual UINT32 _getNumRenderElements() const;

		/**
		 * @copydoc GUIElement::getMaterial()
		 */
		virtual const GUIMaterialInfo& _getMaterial(UINT32 renderElementIdx) const;

		/**
		 * @copydoc GUIElement::getNumQuads()
		 */
		virtual UINT32 _getNumQuads(UINT32 renderElementIdx) const;

		/**
		 * @copydoc GUIElement::fillBuffer()
		 */
		virtual void _fillBuffer(UINT8* vertices, UINT8* uv, UINT32* indices, UINT32 startingQuad, 
			UINT32 maxNumQuads, UINT32 vertexStride, UINT32 indexStride, UINT32 renderElementIdx) const;

		/**
		 * @copydoc GUIElement::updateRenderElementsInternal()
		 */
		virtual void updateRenderElementsInternal();

		/**
		 * @copydoc GUIElement::updateBounds()
		 */
		virtual void updateClippedBounds();
	private:
		GUISliderHandle(bool horizontal, bool jumpOnClick, const String& styleName, const GUIDimensions& dimensions);

		/**
		 * @brief	Returns the position of the slider handle, in pixels.
		 */
		INT32 getHandlePosPx() const;

		/**
		 * @copydoc	GUIElement::mouseEvent
		 */
		virtual bool _mouseEvent(const GUIMouseEvent& ev);

		/**
		 * @brief	Checks are the specified over the scroll handle. Coordinates are relative
		 *			to the parent widget.
		 */
		bool isOnHandle(const Vector2I& pos) const;

		/**
		 * @brief	Gets the currently active texture, depending on handle state.
		 */
		const HSpriteTexture& getActiveTexture() const;

		ImageSprite* mImageSprite;
		UINT32 mHandleSize;

		bool mHorizontal; // Otherwise its vertical
		bool mJumpOnClick;
		float mPctHandlePos;
		INT32 mDragStartPos;
		bool mMouseOverHandle;
		bool mHandleDragged;
		State mState;
		Color mColor;
	};
}