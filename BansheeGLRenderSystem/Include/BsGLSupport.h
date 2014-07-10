//__________________________ Banshee Project - A modern game development toolkit _________________________________//
//_____________________________________ www.banshee-project.com __________________________________________________//
//________________________ Copyright (c) 2014 Marko Pintera. All rights reserved. ________________________________//
#pragma once

#include "BsGLPrerequisites.h"
#include "BsGLRenderSystem.h"
#include "BsRenderWindow.h"

namespace BansheeEngine
{
	/**
	 * @brief	Helper class dealing mostly with platform specific OpenGL functionality,
	 *			initialization, extensions and window creation.
	 */
	class BS_RSGL_EXPORT GLSupport
	{
	public:
		GLSupport() { }
		virtual ~GLSupport() { }

		/**
		 * @brief	Creates a new render window using the specified descriptor.
		 *
		 * @param	desc			Description of a render window to create.
		 * @param	parentWindow	Optional parent window if the window shouldn't be a main window. First
		 *							created window cannot have a parent.
		 *
		 * @param	Returns newly created window.
		 */
		virtual RenderWindowPtr newWindow(RENDER_WINDOW_DESC& desc, RenderWindowPtr parentWindow) = 0;

		/**
		 * @brief	Called when OpenGL is being initialized.
		 */
		virtual void start() = 0;

		/**
		 * @brief	Called when OpenGL is being shut down.
		 */
		virtual void stop() = 0;

		/**
		 * @brief	Gets OpenGL vendor name.
		 */
		const String& getGLVendor() const
		{
			return mVendor;
		}

		/**
		 * @brief	Gets OpenGL version string.
		 */
		const String& getGLVersion() const
		{
			return mVersion;
		}

		/**
		 * @brief	Checks is the specified extension available.
		 */
		virtual bool checkExtension(const String& ext) const;

		/**
		 * @brief	Gets an address of an OpenGL procedure with the specified name.
		 */
		virtual void* getProcAddress(const String& procname) = 0;

		/**
		 * @brief	Initializes OpenGL extensions. Must be called after we have a valid and active
		 *			OpenGL context.
		 */
		virtual void initializeExtensions();

		/**
		 * @brief	Gets a structure describing all available video modes.
		 */
		virtual VideoModeInfoPtr getVideoModeInfo() const = 0;

	protected:
		Set<String> extensionList;

		String mVersion;
		String mVendor;
	};
};