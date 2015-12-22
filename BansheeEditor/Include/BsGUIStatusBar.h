#pragma once

#include "BsEditorPrerequisites.h"
#include "BsGUIElementContainer.h"

namespace BansheeEngine
{
	/**
	 * @brief  Editor window status bar that displays log messages and various other information.
	 */
	class BS_ED_EXPORT GUIStatusBar : public GUIElementContainer
	{
		struct PrivatelyConstruct {};

	public:
		/**
		 * Returns type name of the GUI element used for finding GUI element styles. 
		 */
		static const String& getGUITypeName();

		/**
		 * Returns type name of the internal background GUI element. 
		 */
		static const String& getGUIBackgroundTypeName();

		/**
		 * Returns type name of the internal message button GUI element. 
		 */
		static const String& getGUIMessageTypeName();

		/**
		 * @brief	Creates a new GUI status bar.
		 *
		 * @param	options			Options that allow you to control how is the element positioned and sized.
		 *							This will override any similar options set by style.
		 * @param	styleName		Optional style to use for the element. Style will be retrieved
		 *							from GUISkin of the GUIWidget the element is used on. If not specified
		 *							default style is used.
		 */
		static GUIStatusBar* create(const GUIOptions& options, const String& style = StringUtil::BLANK);

		/**
		 * @brief	Creates a new GUI status bar.
		 *
		 * @param	options			Options that allow you to control how is the element positioned and sized.
		 *							This will override any similar options set by style.
		 * @param	styleName		Optional style to use for the element. Style will be retrieved
		 *							from GUISkin of the GUIWidget the element is used on. If not specified
		 *							default style is used.
		 */
		static GUIStatusBar* create(const String& style = StringUtil::BLANK);

		GUIStatusBar(const PrivatelyConstruct& dummy, const String& style, const GUIDimensions& dimensions);

		/**
		 * @brief	Updates the active project displayed on the status bar.
		 * 	
		 * @param	name		Name of the project.
		 * @param	modified	Should the project be displayed as modified (i.e. needs saving).
		 */
		void setProject(const WString& name, bool modified);

		/**
		 * @brief	Updates the active scene displayed on the status bar.
		 * 	
		 * @param	name		Name of the scene.
		 * @param	modified	Should the scene be displayed as modified (i.e. needs saving).
		 */
		void setScene(const WString& name, bool modified);

		/**
		 * @brief	Activates or deactivates the "compilation in progress" visuals on the status bar.
		 */
		void setIsCompiling(bool compiling);

		/**
		 * @copydoc	GUIElement::setTint
		 */
		virtual void setTint(const Color& color) override;

		/**
		 * @copydoc	GUIElement::_updateLayoutInternal
		 */
		void _updateLayoutInternal(const GUILayoutData& data) override;

		/**
		 * @copydoc	GUIElement::_getOptimalSize
		 */
		Vector2I _getOptimalSize() const override;

		Event<void()> onMessageClicked; /**< Triggered when the user clicks on the console message. */
	private:
		virtual ~GUIStatusBar();

		/**
		 * @copydoc	GUIElement::styleUpdated
		 */
		void styleUpdated() override;

		/**
		 * @brief	Triggered when the debug Log was modified.
		 */
		void logModified();

		/**
		 * @brief	Triggered when the user clicks on the message display.
		 */
		void messageBtnClicked();

	private:
		static const Color COLOR_INFO;
		static const Color COLOR_WARNING;
		static const Color COLOR_ERROR;

		GUIPanel* mPanel;
		GUIPanel* mBgPanel;
		GUIButton* mMessage;
		GUILabel* mScene;
		GUILabel* mProject;
		GUILabel* mCompiling;
		GUITexture* mBackground;

		HEvent mLogEntryAddedConn;
		HEvent mMessageBtnPressedConn;
	};
}