//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsVulkanPrerequisites.h"

namespace BansheeEngine
{
	/** @addtogroup Vulkan
	 *  @{
	 */

	/** Represents a single attachment in a Vulkan frame-buffer. */
	struct VULKAN_ATTACHMENT_DESC
	{
		/** View of the image to attach or VK_NULL_HANDLE if none. */
		VkImageView view;

		/** Format of the attached image. */
		VkFormat format;
	};

	/** Contains parameters used for initializing VulkanFrameBuffer object. */
	struct VULKAN_FRAMEBUFFER_DESC
	{
		/** Images describing the color attachments. */
		VULKAN_ATTACHMENT_DESC color[BS_MAX_MULTIPLE_RENDER_TARGETS];

		/** Image describing the depth attachment. */
		VULKAN_ATTACHMENT_DESC depth;

		/** Width of the images, in pixels. All images must be the same size. */
		UINT32 width;

		/** Height of the images, in pixels. All images must be the same size. */
		UINT32 height;

		/** Number of image layers to render to. */
		UINT32 layers;

		/** Number of samples in the attached images. All attachments must have the same number of samples. */
		UINT32 numSamples;

		/** Set to true if framebuffer represents an offscreen surface that will not be presented. */
		bool offscreen;
	};

	/** Vulkan frame buffer containing one or multiple color surfaces, and an optional depth surface. */
	class VulkanFramebuffer : INonCopyable
	{
	public:
		/** Creates a new frame buffer with the specified image views attached. 
		 *
		 * @param[in]	device	Device to create the frame buffer on. All attachment images provided in the @p desc
		 *						parameter must also belong to this device.
		 * @param[in]	desc	Description of the frame buffer.
		 */
		VulkanFramebuffer(const SPtr<VulkanDevice>& device, const VULKAN_FRAMEBUFFER_DESC& desc);
		~VulkanFramebuffer();

		/** Gets internal Vulkan render pass object. */
		VkRenderPass getRenderPass() const { return mRenderPass; }

		/** Gets internal Vulkan framebuffer object. */
		VkFramebuffer getFramebuffer() const { return mFramebuffer; }

		/** Gets the total number of frame-buffer attachments, including both color and depth. */
		UINT32 getNumAttachments() const { return mNumAttachments; }

		/** Gets the number of color frame-buffer attachments. */
		UINT32 getNumColorAttachments() const { return mNumColorAttachments; }

		/** Returns true if the framebuffer has a depth attachment. */
		bool hasDepthAttachment() const { return mHasDepth; }

		/** Returns sample flags that determine if the framebuffer supports multi-sampling, and for how many samples. */
		VkSampleCountFlagBits getSampleFlags() const { return mSampleFlags; }
	private:
		VkRenderPass mRenderPass;
		VkFramebuffer mFramebuffer;
		VkDevice mDevice;

		UINT32 mNumAttachments;
		UINT32 mNumColorAttachments;
		bool mHasDepth;
		VkSampleCountFlagBits mSampleFlags;
	};

	/** @} */
}