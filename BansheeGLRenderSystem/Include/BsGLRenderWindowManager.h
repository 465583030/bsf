#pragma once

#include "BsGLPrerequisites.h"
#include "BsRenderWindowManager.h"

namespace BansheeEngine
{
	/**
	 * @brief	Manager that handles window creation for OpenGL.
	 */
	class BS_RSGL_EXPORT GLRenderWindowManager : public RenderWindowManager
	{
	public:
		GLRenderWindowManager(GLRenderAPI* renderSystem);

	protected:
		/**
		 * @copydoc RenderWindowManager::createImpl()
		 */
		RenderWindowPtr createImpl(RENDER_WINDOW_DESC& desc, const RenderWindowPtr& parentWindow);

	private:
		GLRenderAPI* mRenderSystem;
	};

	/**
	 * @brief	Manager that handles window creation for OpenGL.
	 */
	class BS_RSGL_EXPORT GLRenderWindowCoreManager : public RenderWindowCoreManager
	{
	public:
		GLRenderWindowCoreManager(GLRenderAPI* renderSystem);

	protected:
		/**
		 * @copydoc RenderWindowCoreManager::createInternal
		 */
		SPtr<RenderWindowCore> createInternal(RENDER_WINDOW_DESC& desc);

	private:
		GLRenderAPI* mRenderSystem;
	};
}