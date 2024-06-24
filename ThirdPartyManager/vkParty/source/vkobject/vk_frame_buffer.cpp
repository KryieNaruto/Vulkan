#include "vkobject/vk_frame_buffer.h"

ThirdParty::FrameBuffer::FrameBuffer(const std::string& _label /*= "FrameBuffer"*/)
	:vkObject(_label)
{
	if (!p_createInfo) {
		p_createInfo = new VkFramebufferCreateInfo;
		p_createInfo->sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		p_createInfo->pNext = 0;
		p_createInfo->flags = 0;
	}
}

ThirdParty::FrameBuffer::~FrameBuffer()
{
	if (m_frame_buffer != VK_NULL_HANDLE)
		vkDestroyFramebuffer(m_vkInfo.m_device, m_frame_buffer, m_vkInfo.p_allocate);
	if (p_createInfo) delete p_createInfo;
}

VkFramebuffer ThirdParty::FrameBuffer::createFrameBuffer(const VkExtent2D& _size, VkRenderPass _renderPass, const std::vector<VkImageView>& _pAttachments, uint32_t _layers /*= 1 */)
{
	p_createInfo->width = _size.width;
	p_createInfo->height = _size.height;
	p_createInfo->renderPass = _renderPass;
	p_createInfo->attachmentCount = _pAttachments.size();
	p_createInfo->pAttachments = _pAttachments.data();
	p_createInfo->layers = _layers;

	m_result = vkCreateFramebuffer(m_vkInfo.m_device, p_createInfo, m_vkInfo.p_allocate, &m_frame_buffer);
	if (checkVkResult(m_result, ("Failed to create frame buffer:" + m_label).c_str()))
		return m_frame_buffer;
	return VK_NULL_HANDLE;
}


ThirdParty::FrameBuffer* ThirdParty::FrameBuffer::recreate(const VkExtent2D& _size, const std::vector<VkImageView>& _pAttachments)
{
	if(m_frame_buffer != VK_NULL_HANDLE)
		vkDestroyFramebuffer(m_vkInfo.m_device, m_frame_buffer, m_vkInfo.p_allocate);

	p_createInfo->width = _size.width;
	p_createInfo->height = _size.height;
	p_createInfo->attachmentCount = _pAttachments.size();
	p_createInfo->pAttachments = _pAttachments.data();
	m_result = vkCreateFramebuffer(m_vkInfo.m_device, p_createInfo, m_vkInfo.p_allocate, &m_frame_buffer);
	if (checkVkResult(m_result, ("Failed to create frame buffer:" + m_label).c_str()))
		return this;
	return nullptr;
}
