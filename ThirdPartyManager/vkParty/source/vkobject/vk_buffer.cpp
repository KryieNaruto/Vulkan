#include "vkobject/vk_buffer.h"
#include "vk_party_manager.h"
#include "third_party_manager_global.h"

ThirdParty::Buffer::Buffer(const std::string& _label /*= "Buffer"*/)
	:vkObject(_label)
{
}

ThirdParty::Buffer::Buffer(size_t _size, VkBufferUsageFlags _usage, VkMemoryPropertyFlags _prop)
	:vkObject()
{
	m_buffer_usage = _usage;
	m_memory_property = _prop;

	m_buffer = vkManager.createBuffer(_size, m_buffer_usage, m_memory_property);
	m_buffer_memory = vkManager.bindBufferMemory(m_buffer, m_memory_property, m_buffer_size, m_buffer_offset);
}

ThirdParty::Buffer::~Buffer()
{
	vkDeviceWaitIdle(m_vkInfo.m_device);
	if (m_buffer_memory != VK_NULL_HANDLE)
	{
		vkManager.unbindBufferMemory(m_buffer_memory, m_buffer_size, m_buffer_offset);
	}
	if (m_buffer != VK_NULL_HANDLE) {
		vkDestroyBuffer(m_vkInfo.m_device, m_buffer, m_vkInfo.p_allocate);
	}
}

ThirdParty::Buffer* ThirdParty::Buffer::bindData(void* _data)
{
	vkManager.bindBufferData(m_buffer_memory, _data, m_buffer_offset, m_buffer_size);
	return this;
}

ThirdParty::Buffer* ThirdParty::Buffer::bindData(void* _data, size_t _offset, size_t _size)
{
	vkManager.bindBufferData(m_buffer_memory, _data, m_buffer_offset + _offset, _size);
	return this;
}

ThirdParty::Buffer* ThirdParty::Buffer::copyData(VkBuffer _dstBuffer)
{
	auto cmd = vkManager.BeginNewSigleCommandBuffer();
	// 此处的offset 是相对于buffer
	VkBufferCopy copyRegion;
	copyRegion.srcOffset = 0;
	copyRegion.size = m_buffer_size;
	copyRegion.dstOffset = 0;
	vkCmdCopyBuffer(cmd, m_buffer, _dstBuffer, 1, &copyRegion);
	vkManager.endSingleCommandBuffer(cmd);
	return this;
}

ThirdParty::Buffer* ThirdParty::Buffer::copyDataToVkImage(VkImage _image, uint32_t _width, uint32_t _height, VkImageAspectFlags _aspect, uint32_t _mipLevel, uint32_t _baseLayer)
{
	VkBufferImageCopy _region = {};
	_region.bufferOffset = 0;
	_region.bufferRowLength = 0;
	_region.bufferImageHeight = 0;

	_region.imageSubresource = { _aspect,_mipLevel,_baseLayer,1};

	_region.imageOffset = { 0,0,0 };
	_region.imageExtent = { _width,_height,1 };

	auto cmd = vkManager.BeginSingleCommandBuffer();
	vkCmdCopyBufferToImage(cmd, m_buffer, _image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &_region);
	vkManager.endSingleCommandBuffer();
	return this;
}

ThirdParty::Buffer* ThirdParty::Buffer::copyData(Buffer* _buffer)
{
	return copyData(_buffer->getVkBuffer());
}
