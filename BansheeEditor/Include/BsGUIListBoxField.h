#pragma once

#include "BsEditorPrerequisites.h"
#include "BsGUIElementContainer.h"

namespace BansheeEngine
{
	/**
	 * @brief	A composite GUI object representing an editor field. Editor fields are a combination
	 *			of a label and an input field. Label is optional. This specific implementation
	 *			displays a list box field.
	 */
	class BS_ED_EXPORT GUIListBoxField : public GUIElementContainer
	{
		struct PrivatelyConstruct {};

	public:
		/**
		 * Returns type name of the GUI element used for finding GUI element styles. 
		 */
		static const String& getGUITypeName();

		/**
		 * Returns style type name of the internal GUILabel element.
		 */
		static const String& getLabelStyleType();

		/**
		 * Style type name for the internal list box.
		 */
		static const String& getListBoxStyleType();

		/**
		 * @brief	Creates a new GUI list box field with a label.
		 *
		 * @param	elements		Elements to display in the list box.
		 * @param	multiselect		Determines should the listbox allow multiple elements to be selected or just one.
		 * @param	labelContent	Content to display in the editor field label.
		 * @param	labelWidth		Width of the label in pixels.
		 * @param	options			Options that allow you to control how is the element positioned and sized.
		 *							This will override any similar options set by style.
		 * @param	styleName		Optional style to use for the element. Style will be retrieved
		 *							from GUISkin of the GUIWidget the element is used on. If not specified
		 *							default style is used.
		 */
		static GUIListBoxField* create(const Vector<HString>& elements, bool multiselect, const GUIContent& labelContent, 
			UINT32 labelWidth, const GUIOptions& options, const String& style = StringUtil::BLANK);

		/**
		 * @brief	Creates a new GUI list box field with a label.
		 *
		 * @param	elements		Elements to display in the list box.
		 * @param	multiselect		Determines should the listbox allow multiple elements to be selected or just one.
		 * @param	labelContent	Content to display in the editor field label.
		 * @param	options			Options that allow you to control how is the element positioned and sized.
		 *							This will override any similar options set by style.
		 * @param	styleName		Optional style to use for the element. Style will be retrieved
		 *							from GUISkin of the GUIWidget the element is used on. If not specified
		 *							default style is used.
		 */
		static GUIListBoxField* create(const Vector<HString>& elements, bool multiselect, const GUIContent& labelContent,
			const GUIOptions& options, const String& style = StringUtil::BLANK);

		/**
		 * @brief	Creates a new GUI list box field with a label.
		 *
		 * @param	elements		Elements to display in the list box.
		 * @param	multiselect		Determines should the listbox allow multiple elements to be selected or just one.
		 * @param	labelText		String to display in the editor field label.
		 * @param	labelWidth		Width of the label in pixels.
		 * @param	options			Options that allow you to control how is the element positioned and sized.
		 *							This will override any similar options set by style.
		 * @param	styleName		Optional style to use for the element. Style will be retrieved
		 *							from GUISkin of the GUIWidget the element is used on. If not specified
		 *							default style is used.
		 */
		static GUIListBoxField* create(const Vector<HString>& elements, bool multiselect, const HString& labelText, 
			UINT32 labelWidth, const GUIOptions& options, const String& style = StringUtil::BLANK);

		/**
		 * @brief	Creates a new GUI list box field with a label.
		 *
		 * @param	elements		Elements to display in the list box.
		 * @param	multiselect		Determines should the listbox allow multiple elements to be selected or just one.
		 * @param	labelText		String to display in the editor field label.
		 * @param	options			Options that allow you to control how is the element positioned and sized.
		 *							This will override any similar options set by style.
		 * @param	styleName		Optional style to use for the element. Style will be retrieved
		 *							from GUISkin of the GUIWidget the element is used on. If not specified
		 *							default style is used.
		 */
		static GUIListBoxField* create(const Vector<HString>& elements, bool multiselect, const HString& labelText, 
			const GUIOptions& options, const String& style = StringUtil::BLANK);

		/**
		 * @brief	Creates a new GUI list box field without a label.
		 *
		 * @param	elements		Elements to display in the list box.
		 * @param	multiselect		Determines should the listbox allow multiple elements to be selected or just one.
		 * @param	options			Options that allow you to control how is the element positioned and sized.
		 *							This will override any similar options set by style.
		 * @param	styleName		Optional style to use for the element. Style will be retrieved
		 *							from GUISkin of the GUIWidget the element is used on. If not specified
		 *							default style is used.
		 */
		static GUIListBoxField* create(const Vector<HString>& elements, bool multiselect, const GUIOptions& options,
			const String& style = StringUtil::BLANK);

		/**
		 * @brief	Creates a new GUI list box field with a label.
		 *
		 * @param	elements		Elements to display in the list box.
		 * @param	multiselect		Determines should the listbox allow multiple elements to be selected or just one.
		 * @param	labelContent	Content to display in the editor field label.
		 * @param	labelWidth		Width of the label in pixels.
		 * @param	styleName		Optional style to use for the element. Style will be retrieved
		 *							from GUISkin of the GUIWidget the element is used on. If not specified
		 *							default style is used.
		 */
		static GUIListBoxField* create(const Vector<HString>& elements, bool multiselect, const GUIContent& labelContent,
			UINT32 labelWidth, const String& style = StringUtil::BLANK);

		/**
		 * @brief	Creates a new GUI list box field with a label.
		 *
		 * @param	elements		Elements to display in the list box.
		 * @param	multiselect		Determines should the listbox allow multiple elements to be selected or just one.
		 * @param	labelContent	Content to display in the editor field label.
		 * @param	styleName		Optional style to use for the element. Style will be retrieved
		 *							from GUISkin of the GUIWidget the element is used on. If not specified
		 *							default style is used.
		 */
		static GUIListBoxField* create(const Vector<HString>& elements, bool multiselect, const GUIContent& labelContent,
			const String& style = StringUtil::BLANK);

		/**
		 * @brief	Creates a new GUI list box field with a label.
		 *
		 * @param	elements		Elements to display in the list box.
		 * @param	multiselect		Determines should the listbox allow multiple elements to be selected or just one.
		 * @param	labelText		String to display in the editor field label.
		 * @param	labelWidth		Width of the label in pixels.
		 * @param	styleName		Optional style to use for the element. Style will be retrieved
		 *							from GUISkin of the GUIWidget the element is used on. If not specified
		 *							default style is used.
		 */
		static GUIListBoxField* create(const Vector<HString>& elements, bool multiselect, const HString& labelText, 
			UINT32 labelWidth, const String& style = StringUtil::BLANK);

		/**
		 * @brief	Creates a new GUI list box field with a label.
		 *
		 * @param	elements		Elements to display in the list box.
		 * @param	multiselect		Determines should the listbox allow multiple elements to be selected or just one.
		 * @param	labelText		String to display in the editor field label.
		 * @param	styleName		Optional style to use for the element. Style will be retrieved
		 *							from GUISkin of the GUIWidget the element is used on. If not specified
		 *							default style is used.
		 */
		static GUIListBoxField* create(const Vector<HString>& elements, bool multiselect, const HString& labelText,
			const String& style = StringUtil::BLANK);

		/**
		 * @brief	Creates a new GUI list box field without a label.
		 *
		 * @param	elements		Elements to display in the list box.
		 * @param	multiselect		Determines should the listbox allow multiple elements to be selected or just one.
		 * @param	styleName		Optional style to use for the element. Style will be retrieved
		 *							from GUISkin of the GUIWidget the element is used on. If not specified
		 *							default style is used.
		 */
		static GUIListBoxField* create(const Vector<HString>& elements, bool multiselect, 
			const String& style = StringUtil::BLANK);

		GUIListBoxField(const PrivatelyConstruct& dummy, const Vector<HString>& elements, bool multiselect, 
			const GUIContent& labelContent, UINT32 labelWidth, const String& style, const GUIDimensions& dimensions, bool withLabel);

		/**
		 * @brief	Checks whether the listbox supports multiple selected elements at once.
		 */
		bool isMultiselect() const;

		/**
		 * @brief	Changes the list box elements.
		 */
		void setElements(const Vector<HString>& elements);

		/**
		 * @brief	Selects an element with the specified index.
		 */
		void selectElement(UINT32 value);

		/**
		 * @brief	Deselect element the element with the specified index. Only relevant for multi-select list boxes.
		 */
		void deselectElement(UINT32 idx);

		/**
		 * @brief	Returns states of all element in the list box (enabled or disabled).
		 */
		const Vector<bool>& getElementStates() const;

		/**
		 * @brief	Sets states for all list box elements. Only valid for multi-select list boxes. Number of states
		 * 			must match number of list box elements.
		 */
		void setElementStates(const Vector<bool>& states);

		/**
		 * @copydoc	GUIElement::setTint
		 */
		virtual void setTint(const Color& color) override;

		Event<void(UINT32, bool)> onSelectionChanged; /**< Triggers when a new element is selected. Provides index to the element. */
	protected:
		static const UINT32 DEFAULT_LABEL_WIDTH;

		virtual ~GUIListBoxField();

		/**
		 * @copydoc	GUIElement::_updateLayoutInternal
		 */
		void _updateLayoutInternal(const GUILayoutData& data) override;

		/**
		 * @copydoc	GUIElement::_getOptimalSize
		 */
		Vector2I _getOptimalSize() const override;

		/**
		 * @copydoc	GUIElement::styleUpdated
		 */
		void styleUpdated() override;

		/**
		 * @brief	Triggered when the selected list box element changes.
		 */
		void selectionChanged(UINT32 newIndex, bool enabled);

		GUIListBox* mListBox;
		GUILayout* mLayout;
		GUILabel* mLabel;
	};
}