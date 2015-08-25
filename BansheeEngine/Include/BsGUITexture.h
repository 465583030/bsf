#pragma once

#include "BsPrerequisites.h"
#include "BsGUIElement.h"
#include "BsImageSprite.h"

namespace BansheeEngine
{
	/**
	 * @brief	Type of scaling modes for GUI images.
	 */
	enum class GUIImageScaleMode
	{
		StretchToFit, /**< Image will stretch non-uniformly in all dimensions in order to cover the assigned area fully. */
		ScaleToFit, /**< Image will scale uniformly until one dimension is aligned with the assigned area. Remaining dimension might have empty space. */
		CropToFit, /**< Image will scale uniformly until both dimensions are larger or aligned with the assigned area. Remaining dimension might be cropped. */
		RepeatToFit /**< Image will keep its original size, but will repeat in order to fill the assigned area. */
	};

	/**
	 * @brief	A GUI element that displays a texture.
	 */
	class BS_EXPORT GUITexture : public GUIElement
	{
	public:
		/**
		 * Returns type name of the GUI element used for finding GUI element styles. 
		 */
		static const String& getGUITypeName();

		/**
		 * @brief	Creates a new GUI texture element.
		 *
		 * @param	texture			Texture element to display. If this is null then the texture specified
		 *							by the style will be used.
		 * @param	scale			Scale mode to use when sizing the texture.
		 * @param	transparent		Determines should the texture be rendered with transparency active.
		 * @param	options			Options that allow you to control how is the element positioned and sized.
		 *							This will override any similar options set by style.
		 * @param	styleName		Optional style to use for the element. Style will be retrieved
		 *							from GUISkin of the GUIWidget the element is used on. If not specified
		 *							default style is used.
		 */
		static GUITexture* create(const HSpriteTexture& texture, GUIImageScaleMode scale, bool transparent,
			const GUIOptions& options, const String& styleName = StringUtil::BLANK);

		/**
		 * @brief	Creates a new GUI texture element.
		 *
		 * @param	texture			Texture element to display. If this is null then the texture specified
		 *							by the style will be used.
		 * @param	scale			Scale mode to use when sizing the texture.
		 * @param	transparent		Determines should the texture be rendered with transparency active.
		 * @param	styleName		Optional style to use for the element. Style will be retrieved
		 *							from GUISkin of the GUIWidget the element is used on. If not specified
		 *							default style is used.
		 */
		static GUITexture* create(const HSpriteTexture& texture, GUIImageScaleMode scale, bool transparent,
			const String& styleName = StringUtil::BLANK);

		/**
		 * @brief	Creates a new GUI texture element.
		 *
		 * @param	texture			Texture element to display. If this is null then the texture specified
		 *							by the style will be used.
		 * @param	scale			Scale mode to use when sizing the texture.
		 * @param	options			Options that allow you to control how is the element positioned and sized.
		 *							This will override any similar options set by style.
		 * @param	styleName		Optional style to use for the element. Style will be retrieved
		 *							from GUISkin of the GUIWidget the element is used on. If not specified
		 *							default style is used.
		 */
		static GUITexture* create(const HSpriteTexture& texture, GUIImageScaleMode scale, 
			const GUIOptions& options, const String& styleName = StringUtil::BLANK);

		/**
		 * @brief	Creates a new GUI texture element.
		 *
		 * @param	texture			Texture element to display. If this is null then the texture specified
		 *							by the style will be used.
		 * @param	scale			Scale mode to use when sizing the texture.
		 * @param	styleName		Optional style to use for the element. Style will be retrieved
		 *							from GUISkin of the GUIWidget the element is used on. If not specified
		 *							default style is used.
		 */
		static GUITexture* create(const HSpriteTexture& texture, GUIImageScaleMode scale, 
			const String& styleName = StringUtil::BLANK);

		/**
		 * @brief	Creates a new GUI texture element. Uses the default StretchToFit scale mode.
		 *
		 * @param	texture			Texture element to display. If this is null then the texture specified
		 *							by the style will be used.
		 * @param	options			Options that allow you to control how is the element positioned and sized.
		 *							This will override any similar options set by style.
		 * @param	styleName		Optional style to use for the element. Style will be retrieved
		 *							from GUISkin of the GUIWidget the element is used on. If not specified
		 *							default style is used.
		 */
		static GUITexture* create(const HSpriteTexture& texture, 
			const GUIOptions& options, const String& styleName = StringUtil::BLANK);

		/**
		 * @brief	Creates a new GUI texture element. Uses the default StretchToFit scale mode.
		 *
		 * @param	texture			Texture element to display. If this is null then the texture specified
		 *							by the style will be used.
		 * @param	styleName		Optional style to use for the element. Style will be retrieved
		 *							from GUISkin of the GUIWidget the element is used on. If not specified
		 *							default style is used.
		 */
		static GUITexture* create(const HSpriteTexture& texture, const String& styleName = StringUtil::BLANK);

		/**
		 * @brief	Creates a new GUI texture element. Uses the "normal" texture from the active GUI element style.
		 *
		 * @param	scale			Scale mode to use when sizing the texture.
		 * @param	options			Options that allow you to control how is the element positioned and sized.
		 *							This will override any similar options set by style.
		 * @param	styleName		Optional style to use for the element. Style will be retrieved
		 *							from GUISkin of the GUIWidget the element is used on. If not specified
		 *							default style is used.
		 */
		static GUITexture* create(GUIImageScaleMode scale, const GUIOptions& options, 
			const String& styleName = StringUtil::BLANK);

		/**
		 * @brief	Creates a new GUI texture element. Uses the "normal" texture from the active GUI element style.
		 *
		 * @param	scale			Scale mode to use when sizing the texture.
		 * @param	styleName		Optional style to use for the element. Style will be retrieved
		 *							from GUISkin of the GUIWidget the element is used on. If not specified
		 *							default style is used.
		 */
		static GUITexture* create(GUIImageScaleMode scale, const String& styleName = StringUtil::BLANK);

		/**
		 * @brief	Creates a new GUI texture element. Uses the "normal" texture from the active GUI element style.
		 *			Uses the default StretchToFit scale mode.
		 *
		 * @param	options			Options that allow you to control how is the element positioned and sized.
		 *							This will override any similar options set by style.
		 * @param	styleName		Optional style to use for the element. Style will be retrieved
		 *							from GUISkin of the GUIWidget the element is used on. If not specified
		 *							default style is used.
		 */
		static GUITexture* create(const GUIOptions& options, const String& styleName = StringUtil::BLANK);

		/**
		 * @brief	Creates a new GUI texture element. Uses the "normal" texture from the active GUI element style.
		 *			Uses the default StretchToFit scale mode.
		 *
		 * @param	styleName		Optional style to use for the element. Style will be retrieved
		 *							from GUISkin of the GUIWidget the element is used on. If not specified
		 *							default style is used.
		 */
		static GUITexture* create(const String& styleName = StringUtil::BLANK);

		/**
		 * @brief	Changes the active texture. If the provided texture is null 
		 *			then the texture specified by the style will be used.
		 */
		void setTexture(const HSpriteTexture& texture);

		/**
		 * @copydoc	GUIElement::setTint
		 */
		virtual void setTint(const Color& color) override;
		
		/**
		 * @copydoc	GUIElement::getElementType
		 */
		virtual ElementType _getElementType() const override { return ElementType::Texture; }

		/**
		 * @copydoc	GUIElement::_getOptimalSize
		 */
		virtual Vector2I _getOptimalSize() const override;
	protected:
		GUITexture(const String& styleName, const HSpriteTexture& texture, GUIImageScaleMode scale, 
			bool transparent, const GUIDimensions& dimensions);
		virtual ~GUITexture();

		/**
		 * @copydoc GUIElement::getNumRenderElements
		 */
		virtual UINT32 _getNumRenderElements() const override;

		/**
		 * @copydoc GUIElement::getMaterial
		 */
		virtual const GUIMaterialInfo& _getMaterial(UINT32 renderElementIdx) const override;

		/**
		 * @copydoc GUIElement::getNumQuads
		 */
		virtual UINT32 _getNumQuads(UINT32 renderElementIdx) const override;

		/**
		 * @copydoc GUIElement::fillBuffer
		 */
		virtual void _fillBuffer(UINT8* vertices, UINT8* uv, UINT32* indices, UINT32 startingQuad, 
			UINT32 maxNumQuads, UINT32 vertexStride, UINT32 indexStride, UINT32 renderElementIdx) const override;

		/**
		 * @copydoc GUIElement::updateRenderElementsInternal
		 */
		virtual void updateRenderElementsInternal() override;

		/**
		 * @copydoc GUIElement::updateBounds
		 */
		virtual void updateClippedBounds() override;

		/**
		 * @copydoc GUIElement::styleUpdated
		 */
		virtual void styleUpdated() override;

		ImageSprite* mImageSprite;
		HSpriteTexture mActiveTexture;
		IMAGE_SPRITE_DESC mDesc;
		GUIImageScaleMode mScaleMode;
		bool mTransparent;
		bool mUsingStyleTexture;
		Color mColor;
	};
}