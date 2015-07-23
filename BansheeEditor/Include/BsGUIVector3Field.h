#pragma once

#include "BsEditorPrerequisites.h"
#include "BsGUIFieldBase.h"
#include "BsVector3.h"

namespace BansheeEngine
{
	/**
	 * @brief	A composite GUI object representing an editor field. Editor fields are a combination
	 *			of a label and an input field. Label is optional. This specific implementation
	 *			displays a Vector3 input field.
	 */
	class BS_ED_EXPORT GUIVector3Field : public TGUIField<GUIVector3Field>
	{
	public:
		/**
		 * Returns type name of the GUI element used for finding GUI element styles. 
		 */
		static const String& getGUITypeName();

		/**
		 * Style type name for the internal input boxes.
		 */
		static const String& getFloatFieldStyleType();

		GUIVector3Field(const PrivatelyConstruct& dummy, const GUIContent& labelContent, UINT32 labelWidth,
			const String& style, const GUIDimensions& dimensions, bool withLabel);

		/**
		 * @brief	Returns the value of the input field.
		 */
		Vector3 getValue() const;

		/**
		 * @brief	Sets a new value in the input field.
		 */
		void setValue(const Vector3& value);

		/**
		 * @brief	Checks is the input field currently active.
		 */
		bool hasInputFocus() const;

		/**
		 * @copydoc	GUIElement::setTint
		 */
		virtual void setTint(const Color& color) override;

		Event<void(const Vector3&)> onValueChanged; /**< Triggers when the field value changes. */
	protected:
		virtual ~GUIVector3Field() { }

		/**
		 * @copydoc	GUIElement::styleUpdated
		 */
		void styleUpdated() override;

		/**
		 * @brief	Triggered when the values in any of the input boxes change.
		 */
		void valueChanged(float newValue);

		static const UINT32 ELEMENT_LABEL_WIDTH;

		GUIFloatField* mFieldX;
		GUIFloatField* mFieldY;
		GUIFloatField* mFieldZ;
	};
}