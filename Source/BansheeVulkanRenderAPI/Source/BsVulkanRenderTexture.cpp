//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsVulkanRenderTexture.h"
#include "BsVulkanFramebuffer.h"
#include "BsVulkanTexture.h"
#include "BsVulkanUtility.h"
#include "BsVulkanRenderAPI.h"
#include "BsVulkanDevice.h"

namespace bs
{
	VulkanRenderTextureCore::VulkanRenderTextureCore(const RENDER_TEXTURE_DESC_CORE& desc, UINT32 deviceIdx)
		:RenderTextureCore(desc, deviceIdx), mProperties(desc, false), mDeviceIdx(deviceIdx), mFramebuffer(nullptr)
	{
		
	}

	VulkanRenderTextureCore::~VulkanRenderTextureCore()
	{
		mFramebuffer->destroy();
	}

	void VulkanRenderTextureCore::initialize()
	{
		RenderTextureCore::initialize();

		VULKAN_FRAMEBUFFER_DESC fbDesc;
		fbDesc.width = mProperties.getWidth();
		fbDesc.height = mProperties.getHeight();
		fbDesc.layers = mProperties.getNumSlices();
		fbDesc.numSamples = mProperties.getMultisampleCount() > 1 ? mProperties.getMultisampleCount() : 1;
		fbDesc.offscreen = true;

		for (UINT32 i = 0; i < BS_MAX_MULTIPLE_RENDER_TARGETS; ++i)
		{
			if (mColorSurfaces[i] == nullptr)
				continue;

			const SPtr<TextureView>& view = mColorSurfaces[i];
			VulkanTextureCore* texture = static_cast<VulkanTextureCore*>(view->getTexture().get());

			TextureSurface surface;
			surface.arraySlice = view->getFirstArraySlice();
			surface.numArraySlices = view->getNumArraySlices();
			surface.mipLevel = view->getMostDetailedMip();
			surface.numMipLevels = view->getNumMips();

			fbDesc.color[i].view = texture->getView(mDeviceIdx, surface);
			fbDesc.color[i].format = VulkanUtility::getPixelFormat(texture->getProperties().getFormat());
			fbDesc.color[i].baseLayer = view->getFirstArraySlice();
		}

		if(mDepthStencilSurface != nullptr)
		{
			const SPtr<TextureView>& view = mDepthStencilSurface;
			VulkanTextureCore* texture = static_cast<VulkanTextureCore*>(view->getTexture().get());

			TextureSurface surface;
			surface.arraySlice = view->getFirstArraySlice();
			surface.numArraySlices = view->getNumArraySlices();
			surface.mipLevel = view->getMostDetailedMip();
			surface.numMipLevels = view->getNumMips();

			fbDesc.depth.view = texture->getView(mDeviceIdx, surface);
			fbDesc.depth.format = VulkanUtility::getPixelFormat(texture->getProperties().getFormat());
			fbDesc.depth.baseLayer = view->getFirstArraySlice();
		}

		VulkanRenderAPI& rapi = static_cast<VulkanRenderAPI&>(RenderAPICore::instance());
		SPtr<VulkanDevice> device = rapi._getDevice(mDeviceIdx);

		mFramebuffer = device->getResourceManager().create<VulkanFramebuffer>(fbDesc);
	}

	void VulkanRenderTextureCore::getCustomAttribute(const String& name, void* data) const
	{
		if (name == "FB")
		{
			VulkanFramebuffer** fb = (VulkanFramebuffer**)data;
			*fb = mFramebuffer;
			return;
		}
	}

	VulkanRenderTexture::VulkanRenderTexture(const RENDER_TEXTURE_DESC& desc)
		:RenderTexture(desc), mProperties(desc, false)
	{ 

	}
}