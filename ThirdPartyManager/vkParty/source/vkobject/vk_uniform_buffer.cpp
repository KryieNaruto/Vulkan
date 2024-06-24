#include "vkobject/vk_uniform_buffer.h"

ThirdParty::UniformBuffer::UniformBuffer(size_t _size, const std::string _label)
	:vkObject(_label)
{
	p_buffers.resize(Core::vkInfo.m_frame_count);

	for (auto& b : p_buffers) {
		b = new Buffer(
			_size,
			VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
		);
	}
}

ThirdParty::UniformBuffer::UniformBuffer(size_t _size)
	:vkObject()
{
	m_label = "Uniform Buffer";
	p_buffers.resize(Core::vkInfo.m_frame_count);

	for (auto& b : p_buffers) {
		b = new Buffer(
			_size,
			VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
		);
	}
}

ThirdParty::UniformBuffer::~UniformBuffer()
{
	for (auto& b : p_buffers)
		delete b;

	removeFromVkObjects();
}

ThirdParty::UniformBuffer* ThirdParty::UniformBuffer::bindData(void* _data)
{
	if (p_buffers[m_vkInfo.m_current_frame])
		p_buffers[m_vkInfo.m_current_frame]->bindData(_data);
	return this;
}

ThirdParty::UniformBuffer* ThirdParty::UniformBuffer::bindData(void* _data, size_t _offset, size_t _size)
{
	if (p_buffers[m_vkInfo.m_current_frame])
		p_buffers[m_vkInfo.m_current_frame]->bindData(_data, _offset, _size);
	return this;
}
