#include "vkobject/vk_texture.h"
#include <vkParty/include/vk_party_manager.h>
ThirdParty::Texture::~Texture()
{

}

VkImage& ThirdParty::Texture::transitionImageLayout(VkImageLayout _newLayout, VkImageSubresourceRange _res, VkCommandBuffer _cmd)
{
	m_layout_old = m_layout_current;
	m_layout_current = _newLayout;

	const auto source = getAccessMask_PipelineStage(m_layout_old);
	const auto destination = getAccessMask_PipelineStage(m_layout_current);

	VkImageMemoryBarrier barrier = { VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER };
	barrier.image = m_image;
	barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.oldLayout = m_layout_old;
	barrier.newLayout = m_layout_current;
	barrier.subresourceRange = _res;
	barrier.srcAccessMask = source.first;
	barrier.dstAccessMask = destination.first;

	const auto& cmd = _cmd == VK_NULL_HANDLE ? ThirdParty::vkManager.BeginSingleCommandBuffer() : _cmd;
	vkCmdPipelineBarrier(cmd, source.second, destination.second, 0, 0, nullptr, 0, nullptr, 1, &barrier);
	if (_cmd == VK_NULL_HANDLE)
		ThirdParty::vkManager.endSingleCommandBuffer();

	return m_image;
}

VkImage& ThirdParty::Texture::transitionImageLayout(VkImageLayout _oldLayout, VkImageLayout _newLayout, VkImageSubresourceRange _res /*= { VK_IMAGE_ASPECT_COLOR_BIT,0,1,0,1 }*/, VkCommandBuffer _cmd)
{
	m_layout_old = _oldLayout;
	m_layout_current = _newLayout;

	const auto source = getAccessMask_PipelineStage(m_layout_old);
	const auto destination = getAccessMask_PipelineStage(m_layout_current);

	VkImageMemoryBarrier barrier = { VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER };
	barrier.image = m_image;
	barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.oldLayout = m_layout_old;
	barrier.newLayout = m_layout_current;
	barrier.subresourceRange = _res;
	barrier.srcAccessMask = source.first;
	barrier.dstAccessMask = destination.first;

	const auto& cmd = _cmd == VK_NULL_HANDLE ? ThirdParty::vkManager.BeginSingleCommandBuffer() : _cmd;
	vkCmdPipelineBarrier(cmd, source.second, destination.second, 0, 0, nullptr, 0, nullptr, 1, &barrier);
	if (_cmd == VK_NULL_HANDLE)
		ThirdParty::vkManager.endSingleCommandBuffer();

	return m_image;
}

void ThirdParty::Texture::recreate(VkImage _image, VkImageView _imageView)
{
	m_layout_old = VK_IMAGE_LAYOUT_UNDEFINED;
	m_layout_current = VK_IMAGE_LAYOUT_UNDEFINED;
	m_image = _image;
	m_image_view = _imageView;
}
