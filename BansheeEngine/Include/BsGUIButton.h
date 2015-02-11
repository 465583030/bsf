#pragma once

#include "BsPrerequisites.h"
#include "BsGUIButtonBase.h"
#include "BsImageSprite.h"
#include "BsTextSprite.h"
#include "BsGUIContent.h"
#include "BsEvent.h"

namespace BansheeEngine
{
	/**
	 * @brief	GUI button that can be clicked. Has normal, hover and active states
	 *			with an optional label.
	 */
	class BS_EXPORT GUIButton : public GUIButtonBase
	{
	public:
		/**
		 * Returns type name of the GUI element used for finding GUI element styles. 
		 */
		static const String& getGUITypeName();

		/**
		 * Creates a new button with the specified label.
		 *
		 * @param	text		Label to display on the button.
		 * @param	styleName		Optional style to use for the element. Style will be retrieved
		 *							from GUISkin of the GUIWidget the element is used on. If not specified
		 *							default button style is used.
		 */
		static GUIButton* create(const HString& text, const String& styleName = StringUtil::BLANK);

		/**
		 * Creates a new button with the specified label.
		 *
		 * @param	text			Label to display on the button.
		 * @param	layoutOptions	Options that allows you to control how is the element positioned in
		 *							GUI layout. This will override any similar options set by style.
		 * @param	styleName		Optional style to use for the element. Style will be retrieved
		 *							from GUISkin of the GUIWidget the element is used on. If not specified
		 *							default button style is used.
		 */
		static GUIButton* create(const HString& text, const GUIOptions& layoutOptions, const String& styleName = StringUtil::BLANK);

		/**
		 * Creates a new button with the specified label.
		 *
		 * @param	content		Content to display on a button. May include a label, image and a tooltip.
		 * @param	styleName		Optional style to use for the element. Style will be retrieved
		 *							from GUISkin of the GUIWidget the element is used on. If not specified
		 *							default button style is used.
		 */
		static GUIButton* create(const GUIContent& content, const String& styleName = StringUtil::BLANK);

		/**
		 * Creates a new button with the specified label.
		 *
		 * @param	content		Content to display on a button. May include a label, image and a tooltip.
		 * @param	layoutOptions	Options that allows you to control how is the element positioned in
		 *							GUI layout. This will override any similar options set by style.
		 * @param	styleName		Optional style to use for the element. Style will be retrieved
		 *							from GUISkin of the GUIWidget the element is used on. If not specified
		 *							default button style is used.
		 */
		static GUIButton* create(const GUIContent& content, const GUIOptions& layoutOptions, const String& styleName = StringUtil::BLANK);

		/**
		 * @copydoc	GUIElement::getElementType
		 */
		virtual ElementType _getElementType() const { return ElementType::Button; }
	private:
		GUIButton(const String& styleName, const GUIContent& content, const GUILayoutOptions& layoutOptions);
	};
}