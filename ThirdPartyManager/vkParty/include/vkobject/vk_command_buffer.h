#pragma once
#include <ThirdParty/vulkan/vulkan.h>
#include <string>
#include "vk_object.h"

namespace ThirdParty {

	// 命令缓冲对象
	// 将VkCommandBuffer作为引用保存，有分配与销毁VkCommandBuffer功能，提供beginInfo
	class ENGINE_API_THIRDPARTY CommandBuffer : public vkObject
	{
	public:
		CommandBuffer(const std::string& _label = "CommandBuffer");	// 无创建与销毁功能
		// 包含分配与销毁
		CommandBuffer(const VkCommandPool& _command_pool, const std::string& _label = "CommandBuffer");
		virtual ~CommandBuffer();
		VkResult beginCommandBuffer();
		VkResult beginCommandBuffer(VkCommandBuffer& _commandbuffer);
		VkResult endCommandBuffer();
		inline VkCommandBuffer& getVkCommandBuffer() { return m_commandbuffer; }
		CommandBuffer* recreate();
	protected:
	private:
		bool m_begining = false;
		VkCommandBuffer m_commandbuffer;
		VkCommandBufferBeginInfo* p_begin_info = nullptr;
		VkCommandPool m_command_pool = VK_NULL_HANDLE;
	};

}

