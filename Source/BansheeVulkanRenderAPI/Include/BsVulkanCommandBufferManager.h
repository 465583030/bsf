//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsVulkanPrerequisites.h"
#include "BsCommandBufferManager.h"

namespace BansheeEngine
{
	/** @addtogroup Vulkan
	 *  @{
	 */

	/** 
	 * Handles creation of Vulkan command buffers. See CommandBuffer. 
	 *
	 * @note Core thread only.
	 */
	class VulkanCommandBufferManager : public CommandBufferManager
	{
	public:
		VulkanCommandBufferManager(const VulkanRenderAPI& rapi);
		~VulkanCommandBufferManager();

		/** @copydoc CommandBufferManager::createInternal() */
		SPtr<CommandBuffer> createInternal(UINT32 id, CommandBufferType type, UINT32 deviceIdx = 0, UINT32 queueIdx = 0,
			bool secondary = false) override;

	private:
		const VulkanRenderAPI& mRapi;
	};

	/** @} */
}