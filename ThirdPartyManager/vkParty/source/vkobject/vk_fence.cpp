#include "vkobject/vk_fence.h"

ThirdParty::Fence::Fence(const std::string& _label /*= "Fence"*/)
	:vkObject(_label)
{
	if (!p_createInfo) {
		p_createInfo = new VkFenceCreateInfo;
		p_createInfo->sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		p_createInfo->pNext = 0;
		p_createInfo->flags = VK_FENCE_CREATE_SIGNALED_BIT;
		
		m_result = vkCreateFence(m_vkInfo.m_device, p_createInfo, m_vkInfo.p_allocate, &m_fence);
		checkVkResult(m_result, ("Failed to create VkFence:" + m_label).c_str());
	}
}

ThirdParty::Fence::~Fence()
{
	if (p_createInfo) delete p_createInfo;
	if (m_fence != VK_NULL_HANDLE) vkDestroyFence(m_vkInfo.m_device, m_fence, m_vkInfo.p_allocate);
}

ThirdParty::Fence* ThirdParty::Fence::recreate()
{
	vkDestroyFence(m_vkInfo.m_device, m_fence, m_vkInfo.p_allocate);
	m_result = vkCreateFence(m_vkInfo.m_device, p_createInfo, m_vkInfo.p_allocate, &m_fence);
	checkVkResult(m_result, ("Failed to create VkFence:" + m_label).c_str());
	return this;
}
