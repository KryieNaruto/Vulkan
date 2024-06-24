#include "vkobject/vk_index_buffer.h"

ThirdParty::IndexBuffer::IndexBuffer(size_t _size, const std::string _label /*= "IndexBuffer"*/)
	:vkObject(_label)
{

	p_buffer = new Buffer(
		_size,
		VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT|VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
	);
}

ThirdParty::IndexBuffer::IndexBuffer(size_t _size)
	:vkObject()
{
	m_label = "Index Buffer";
	p_buffer = new Buffer(
		_size,
		VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
	);
}

ThirdParty::IndexBuffer::~IndexBuffer()
{
	if (p_buffer) delete p_buffer;
	removeFromVkObjects();
}

ThirdParty::IndexBuffer* ThirdParty::IndexBuffer::bindData(void* _data)
{
	if (p_buffer) {
		p_buffer->bindData(_data);
	}
	return this;
}
