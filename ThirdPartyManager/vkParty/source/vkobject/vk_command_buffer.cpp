#include "vkobject/vk_command_buffer.h"
#include <vk_util_function.h>

ThirdParty::CommandBuffer::CommandBuffer(const std::string& _label /*= "CommandBuffer"*/)
	:vkObject(_label)
{
	if (!this->p_begin_info)
	{
		this->p_begin_info = new VkCommandBufferBeginInfo;
		p_begin_info->sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		p_begin_info->flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
		p_begin_info->pInheritanceInfo = nullptr;
		p_begin_info->pNext = nullptr;
	}
	this->m_commandbuffer = VK_NULL_HANDLE;
}

ThirdParty::CommandBuffer::CommandBuffer(const VkCommandPool& _command_pool, const std::string& _label /*= "CommandBuffer"*/)
	:vkObject(_label),m_command_pool(_command_pool),m_commandbuffer(VK_NULL_HANDLE)
{
	if (!this->p_begin_info)
	{
		this->p_begin_info = new VkCommandBufferBeginInfo;
		p_begin_info->sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		p_begin_info->flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
		p_begin_info->pInheritanceInfo = nullptr;
		p_begin_info->pNext = nullptr;
	}
	VkCommandBufferAllocateInfo alloc = { VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO };
	alloc.commandBufferCount = 1;
	alloc.commandPool = m_command_pool;
	alloc.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	vkAllocateCommandBuffers(m_vkInfo.m_device, &alloc, &m_commandbuffer);
}

ThirdParty::CommandBuffer::~CommandBuffer()
{
	if (this->p_begin_info)
		delete p_begin_info;
	if (m_command_pool)
		vkFreeCommandBuffers(m_vkInfo.m_device, m_command_pool, 1, &m_commandbuffer);
}

VkResult ThirdParty::CommandBuffer::beginCommandBuffer(VkCommandBuffer& _commandbuffer)
{
	this->m_commandbuffer = _commandbuffer;
	return beginCommandBuffer();
}

VkResult ThirdParty::CommandBuffer::beginCommandBuffer()
{
	if (this->m_begining) return m_result;
	m_result = vkBeginCommandBuffer(this->m_commandbuffer, this->p_begin_info);
	this->m_begining = checkVkResult(m_result, ("Failed to begin command buffer:" + this->m_label).c_str()) ? true : false;
	return m_result;
}

VkResult ThirdParty::CommandBuffer::endCommandBuffer()
{
	if (!this->m_begining) return m_result;
	m_result = vkEndCommandBuffer(this->m_commandbuffer);
	this->m_begining = checkVkResult(m_result, ("Failed to end command buffer:" + this->m_label).c_str()) ? false : true;
	return m_result;
}

ThirdParty::CommandBuffer* ThirdParty::CommandBuffer::recreate()
{
	if (m_command_pool)
		vkFreeCommandBuffers(m_vkInfo.m_device, m_command_pool, 1, &m_commandbuffer);
	VkCommandBufferAllocateInfo alloc = { VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO };
	alloc.commandBufferCount = 1;
	alloc.commandPool = m_command_pool;
	alloc.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	vkAllocateCommandBuffers(m_vkInfo.m_device, &alloc, &m_commandbuffer);
	return this;
}
