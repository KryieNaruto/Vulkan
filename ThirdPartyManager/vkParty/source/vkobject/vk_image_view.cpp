#include "vkobject/vk_image_view.h"

ThirdParty::ImageView::~ImageView()
{
	vkDeviceWaitIdle(m_vkInfo.m_device);
	if (p_create_info) delete p_create_info;
	if (m_image_view != VK_NULL_HANDLE) {
		vkDestroyImageView(m_vkInfo.m_device, m_image_view, m_vkInfo.p_allocate);
		m_image_view = VK_NULL_HANDLE;
	}
}

ThirdParty::ImageView* ThirdParty::ImageView::createImageView(VkImage _image, VkFormat _format /*= VK_FORMAT_R8G8B8A8_UNORM*/, VkImageSubresourceRange _subresourceRange /*= { VK_IMAGE_ASPECT_COLOR_BIT,0,1,0,1 }*/, VkImageViewType _viewType /*= VK_IMAGE_VIEW_TYPE_2D*/, VkComponentMapping _components /*= { VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY , VK_COMPONENT_SWIZZLE_IDENTITY , VK_COMPONENT_SWIZZLE_IDENTITY } */)
{
	if (!p_create_info) {
		p_create_info = new VkImageViewCreateInfo;
		p_create_info->sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		p_create_info->flags = 0;
		p_create_info->pNext = 0;
	}
	p_create_info->image = _image;
	p_create_info->format = _format;
	p_create_info->subresourceRange = _subresourceRange;
	p_create_info->viewType = _viewType;
	p_create_info->components = _components;

	m_result = vkCreateImageView(m_vkInfo.m_device, p_create_info, m_vkInfo.p_allocate,&m_image_view);
	checkVkResult(m_result, ("Failed to create image view:" + m_label).c_str());
	return this;
}

ThirdParty::ImageView* ThirdParty::ImageView::recreate(VkImage _image)
{
	vkDestroyImageView(m_vkInfo.m_device, m_image_view, m_vkInfo.p_allocate);
	p_create_info->image = _image;
	m_result = vkCreateImageView(m_vkInfo.m_device, p_create_info, m_vkInfo.p_allocate, &m_image_view);
	checkVkResult(m_result, ("Failed to create image view:" + m_label).c_str());
	return this;
}

