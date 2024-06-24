#include "vkobject/vk_image.h"
#include "vkParty/include/vk_party_manager.h"
ThirdParty::Image::~Image()
{
	vkDeviceWaitIdle(m_vkInfo.m_device);
	if (p_create_info) delete p_create_info;
	if (m_image != VK_NULL_HANDLE) {
		vkDestroyImage(m_vkInfo.m_device, m_image, m_vkInfo.p_allocate);
		m_image = VK_NULL_HANDLE;
	}
	if (m_image_memory != VK_NULL_HANDLE) {
		vkFreeMemory(m_vkInfo.m_device, m_image_memory, m_vkInfo.p_allocate);
		m_image_memory = VK_NULL_HANDLE;
	}
}

ThirdParty::Image* ThirdParty::Image::createImage(VkImageUsageFlags _usage, VkExtent3D _extent, VkFormat _format /*= VK_FORMAT_R8G8B8A8_UNORM*/, /*----------------------------------- */ uint32_t _mipLevels /*= 1*/, VkSampleCountFlagBits _samples /*= VK_SAMPLE_COUNT_1_BIT*/, uint32_t _queueFamilyIndexCount /*= 0*/, const uint32_t* _pQueueFamilyIndices /*= nullptr*/, VkSharingMode _sharingMode /*= VK_SHARING_MODE_EXCLUSIVE*/, VkImageTiling _tiling /*= VK_IMAGE_TILING_OPTIMAL*/, /*----------------------------------- */ VkImageType _imageType /*= VK_IMAGE_TYPE_2D*/, uint32_t _arrayLayers /*= 1*/, VkImageLayout _initialLayout /*= VK_IMAGE_LAYOUT_UNDEFINED */)
{
	if (!p_create_info) {
		p_create_info = new VkImageCreateInfo;
		p_create_info->sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		p_create_info->flags = 0;
		p_create_info->pNext = 0;
	}
	p_create_info->usage = _usage;
	p_create_info->extent = _extent;
	p_create_info->format = _format;
	p_create_info->mipLevels = _mipLevels;
	p_create_info->samples = _samples;
	p_create_info->queueFamilyIndexCount = _queueFamilyIndexCount;
	p_create_info->pQueueFamilyIndices = _pQueueFamilyIndices;
	p_create_info->sharingMode = _sharingMode;
	p_create_info->tiling = _tiling;
	p_create_info->imageType = _imageType;
	p_create_info->arrayLayers = _arrayLayers;
	p_create_info->initialLayout = _initialLayout;

	m_result = vkCreateImage(m_vkInfo.m_device, p_create_info, m_vkInfo.p_allocate,&m_image);
	if (checkVkResult(m_result, ("Failed to create image:" + m_label).c_str())) {
		// 绑定内存
		ThirdParty::vkManager.bindImageMemory(m_image, m_size, m_offset);
	}

	return this;
}

ThirdParty::Image* ThirdParty::Image::recreate(VkExtent3D _extent)
{
	p_create_info->extent = _extent;
	// 取消绑定内存区间
	vkDeviceWaitIdle(m_vkInfo.m_device);
	ThirdParty::vkManager.unbindImageMemory(m_size, m_offset);
	vkDestroyImage(m_vkInfo.m_device, m_image, m_vkInfo.p_allocate);
	m_result = vkCreateImage(m_vkInfo.m_device, p_create_info, m_vkInfo.p_allocate, &m_image);
	if (checkVkResult(m_result, ("Failed to create image:" + m_label).c_str())) {
		ThirdParty::vkManager.bindImageMemory(m_image, m_size, m_offset);
	}
	return this;
}

VkImage& ThirdParty::Image::getVkImage()
{
	return m_image;
}
