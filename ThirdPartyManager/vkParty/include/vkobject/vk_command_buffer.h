#pragma once
#include <ThirdParty/vulkan/vulkan.h>
#include <string>
#include "vk_object.h"

namespace ThirdParty {

	// ��������
	// ��VkCommandBuffer��Ϊ���ñ��棬�з���������VkCommandBuffer���ܣ��ṩbeginInfo
	class ENGINE_API_THIRDPARTY CommandBuffer : public vkObject
	{
	public:
		CommandBuffer(const std::string& _label = "CommandBuffer");	// �޴��������ٹ���
		// ��������������
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

