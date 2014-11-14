#include "BsGLRenderWindowManager.h"
#include "BsGLRenderSystem.h"
#include "BsGLSupport.h"
#include "BsAsyncOp.h"

namespace BansheeEngine
{
	GLRenderWindowManager::GLRenderWindowManager(GLRenderSystem* renderSystem)
		:mRenderSystem(renderSystem)
	{
		assert(mRenderSystem != nullptr);
	}

	RenderWindowPtr GLRenderWindowManager::createImpl(RENDER_WINDOW_DESC& desc, const RenderWindowPtr& parentWindow)
	{
		GLSupport* glSupport = mRenderSystem->getGLSupport();

		// Create the window
		return glSupport->newWindow(desc, parentWindow);
	}

	GLRenderWindowCoreManager::GLRenderWindowCoreManager(GLRenderSystem* renderSystem)
		:mRenderSystem(renderSystem)
	{
		assert(mRenderSystem != nullptr);
	}

	SPtr<RenderWindowCore> GLRenderWindowCoreManager::createInternal(RENDER_WINDOW_DESC& desc)
	{
		GLSupport* glSupport = mRenderSystem->getGLSupport();

		// Create the window
		SPtr<RenderWindowCore> window = glSupport->newWindowCore(desc);
		window->_setThisPtr(window);

		windowCreated(window.get());

		return window;
	}
}