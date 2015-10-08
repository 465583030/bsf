#pragma once

#include "BsPrerequisites.h"
#include "BsGUIElement.h"
#include "BsTextSprite.h"
#include "BsGUIContent.h"

namespace BansheeEngine
{
	/**
	 * @brief	GUI element that displays text and optionally a content image.
	 */
	class BS_EXPORT GUILabel : public GUIElement
	{
	public:
		/**
		 * Returns type name of the GUI element used for finding GUI element styles. 
		 */
		static const String& getGUITypeName();

		/**
		 * Creates a new label with the specified text.
		 *
		 * @param	text			Text to display.
		 * @param	styleName		Optional style to use for the element. Style will be retrieved
		 *							from GUISkin of the GUIWidget the element is used on. If not specified
		 *							default button style is used.
		 */
		static GUILabel* create(const HString& text, const String& styleName = StringUtil::BLANK);

		/**
		 * Creates a new label with the specified text.
		 *
		 * @param	text			Text to display.
		 * @param	options			Options that allow you to control how is the element positioned and sized.
		 *							This will override any similar options set by style.
		 * @param	styleName		Optional style to use for the element. Style will be retrieved
		 *							from GUISkin of the GUIWidget the element is used on. If not specified
		 *							default button style is used.
		 */
		static GUILabel* create(const HString& text, const GUIOptions& options, const String& styleName = StringUtil::BLANK);

		/**
		 * Creates a new label with the specified content (text + optional image).
		 *
		 * @param	text			Content to display.
		 * @param	styleName		Optional style to use for the element. Style will be retrieved
		 *							from GUISkin of the GUIWidget the element is used on. If not specified
		 *							default button style is used.
		 */
		static GUILabel* create(const GUIContent& content, const String& styleName = StringUtil::BLANK);

		/**
		 * Creates a new label with the specified content (text + optional image).
		 *
		 * @param	text			Content to display.
		 * @param	options			Options that allow you to control how is the element positioned and sized.
		 *							This will override any similar options set by style.
		 * @param	styleName		Optional style to use for the element. Style will be retrieved
		 *							from GUISkin of the GUIWidget the element is used on. If not specified
		 *							default button style is used.
		 */
		static GUILabel* create(const GUIContent& content, const GUIOptions& options, const String& styleName = StringUtil::BLANK);

		/**
		 * Changes the active content of the label.
		 */
		void setContent(const GUIContent& content);

		/**
		 * @copydoc	GUIElement::_getOptimalSize
		 */
		virtual Vector2I _getOptimalSize() const override;

		/**
		 * @copydoc	GUIElement::getElementType
		 */
		virtual ElementType _getElementType() const override { return ElementType::Label; }
	protected:
		~GUILabel();

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
	private:
		GUILabel(const String& styleName, const GUIContent& content, const GUIDimensions& dimensions);

		TextSprite* mTextSprite;
		GUIContent mContent;

		TEXT_SPRITE_DESC mDesc;
	};
}