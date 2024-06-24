#include "vkobject/vk_semaphore.h"

ThirdParty::Semaphore::Semaphore(const std::string& _label) 
	:vkObject(_label)
{
	if (!p_createInfo) {
		p_createInfo = new VkSemaphoreCreateInfo;
		p_createInfo->sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
		p_createInfo->flags = 0;
		p_createInfo->pNext = 0;
	}
	m_result = vkCreateSemaphore(m_vkInfo.m_device, p_createInfo, m_vkInfo.p_allocate, &m_Semaphore);
	checkVkResult(m_result, ("Failed to create VkSeamphore" + m_label).c_str());
}

ThirdParty::Semaphore* ThirdParty::Semaphore::recreate() {
	vkDestroySemaphore(m_vkInfo.m_device, m_Semaphore, m_vkInfo.p_allocate);
	m_result = vkCreateSemaphore(m_vkInfo.m_device, p_createInfo, m_vkInfo.p_allocate, &m_Semaphore);
	checkVkResult(m_result, ("Failed to create VkSeamphore" + m_label).c_str());
	return this;
}