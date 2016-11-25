//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsVulkanPrerequisites.h"
#include "BsVulkanResource.h"
#include "BsTexture.h"

namespace bs
{
	/** @addtogroup Vulkan
	 *  @{
	 */

	/** Wrapper around a Vulkan image object that manages its usage and lifetime. */
	class VulkanImage : public VulkanResource
	{
	public:
		VulkanImage(VulkanResourceManager* owner, VkImage image, VkDeviceMemory memory, VkImageLayout layout,
					const TextureProperties& props);
		~VulkanImage();

		/** Returns the internal handle to the Vulkan object. */
		VkImage getHandle() const { return mImage; }

		/** Returns the layout the image is currently in. */
		VkImageLayout getLayout() const { return mLayout; }

		/** Notifies the resource that the current image layout has changed. */
		void setLayout(VkImageLayout layout) { mLayout = layout; }

		/** Returns an image view that covers all faces and mip maps of the texture. */
		VkImageView getView() const { return mMainView; };

		/** Returns an image view that covers the specified faces and mip maps of the texture. */
		VkImageView getView(const TextureSurface& surface) const;

		/** 
		 * Returns a pointer to internal image memory for the specified sub-resource. Must be followed by unmap(). Caller
		 * must ensure the image was created in CPU readable memory, and that image isn't currently being written to by the
		 * GPU.
		 * 
		 * @param[in]	face		Index of the face to map.
		 * @param[in]	mipLevel	Index of the mip level to map.
		 * @param[in]	output		Output object containing the pointer to the sub-resource data.
		 */
		void map(UINT32 face, UINT32 mipLevel, PixelData& output) const;

		/** Unmaps a buffer previously mapped with map(). */
		void unmap();

		/** 
		 * Queues a command on the provided command buffer. The command copies the contents of the current image
		 * subresource to the destination buffer. 
		 */
		void copy(VulkanTransferBuffer* cb, VulkanBuffer* destination, const VkExtent3D& extent, 
			const VkImageSubresourceLayers& range, VkImageLayout layout);

	private:
		/** Creates a new view of the provided part (or entirety) of surface. */
		VkImageView createView(const TextureSurface& surface) const;

		/** Contains information about view for a specific surface(s) of this image. */
		struct ImageViewInfo
		{
			TextureSurface surface;
			VkImageView view;
		};

		VkImage mImage;
		VkDeviceMemory mMemory;
		VkImageLayout mLayout;
		VkImageView mMainView;

		mutable VkImageViewCreateInfo mImageViewCI;
		mutable Vector<ImageViewInfo> mImageInfos;
	};

	/**	Vulkan implementation of a texture. */
	class VulkanTextureCore : public TextureCore
	{
	public:
		~VulkanTextureCore();

		/** 
		 * Gets the resource wrapping the Vulkan image object, on the specified device. If texture device mask doesn't 
		 * include the provided device, null is returned. 
		 */
		VulkanImage* getResource(UINT32 deviceIdx) const { return mImages[deviceIdx]; }

		/** 
		 * Returns an image view that covers all faces and mip maps of the texture. Usable only on the specified device. 
		 * If texture device mask doesn't include the provided device, null is returned. 
		 */
		VkImageView getView(UINT32 deviceIdx) const;

		/** 
		 * Returns an image view that covers the specified faces and mip maps of the texture. Usable only on the specified 
		 * device. If texture device mask doesn't include the provided device, null is returned. 
		 */
		VkImageView getView(UINT32 deviceIdx, const TextureSurface& surface) const;

	protected:
		friend class VulkanTextureCoreManager;

		VulkanTextureCore(const TEXTURE_DESC& desc, const SPtr<PixelData>& initialData, GpuDeviceFlags deviceMask);

		/** @copydoc CoreObjectCore::initialize() */
		void initialize() override;

		/** @copydoc TextureCore::lockImpl */
		PixelData lockImpl(GpuLockOptions options, UINT32 mipLevel = 0, UINT32 face = 0, UINT32 deviceIdx = 0,
						   UINT32 queueIdx = 0) override;

		/** @copydoc TextureCore::unlockImpl */
		void unlockImpl() override;

		/** @copydoc TextureCore::copyImpl */
		void copyImpl(UINT32 srcFace, UINT32 srcMipLevel, UINT32 destFace, UINT32 destMipLevel,
					  const SPtr<TextureCore>& target, UINT32 queueIdx = 0) override;

		/** @copydoc TextureCore::readData */
		void readDataImpl(PixelData& dest, UINT32 mipLevel = 0, UINT32 face = 0, UINT32 deviceIdx = 0,
					  UINT32 queueIdx = 0) override;

		/** @copydoc TextureCore::writeData */
		void writeDataImpl(const PixelData& src, UINT32 mipLevel = 0, UINT32 face = 0, bool discardWholeBuffer = false,
					   UINT32 queueIdx = 0) override;

	private:
		/** Creates a new image for the specified device, matching the current properties. */
		VulkanImage* createImage(VulkanDevice& device);

		/** 
		 * Creates a staging buffer that can be used for texture transfer operations.
		 * 
		 * @param[in]	device		Device to create the buffer on.
		 * @param[in]	pixelData	Object that describes the image sub-resource that will be in the buffer.
		 * @param[in]	needsRead	True if we will be copying data from the buffer, false if just reading. True if both.
		 * @return					Newly allocated buffer.
		 */
		VulkanBuffer* createStaging(VulkanDevice& device, const PixelData& pixelData, bool needsRead);

		VulkanImage* mImages[BS_MAX_DEVICES];
		GpuDeviceFlags mDeviceMask;
		VkAccessFlags mAccessFlags;

		VulkanBuffer* mStagingBuffer;
		UINT32 mMappedDeviceIdx;
		UINT32 mMappedGlobalQueueIdx;
		UINT32 mMappedMip;
		UINT32 mMappedFace;
		UINT32 mMappedRowPitch;
		UINT32 mMappedSlicePitch;
		GpuLockOptions mMappedLockOptions;

		VkImageCreateInfo mImageCI;
		bool mDirectlyMappable : 1;
		bool mSupportsGPUWrites : 1;
		bool mIsMapped : 1;
	};

	/** @} */
}