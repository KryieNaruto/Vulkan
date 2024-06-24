#include "vkobject/vk_vertex_buffer.h"
#include "core/info/include/global_info.h"
#include "vk_party_manager.h"
ThirdParty::VertexBuffer::VertexBuffer(size_t _size, const std::string _label)
	:vkObject(_label)
{

	p_buffer = new Buffer(
		_size,
		VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
	);

	p_stage_buffer = new Buffer(
		_size,
		VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
	);
}

ThirdParty::VertexBuffer::VertexBuffer(size_t _size)
	:vkObject()
{
	m_label = "Vertex Buffer";
	p_buffer = new Buffer(
		_size,
		VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
	);

	p_stage_buffer = new Buffer(
		_size,
		VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
	);
}

ThirdParty::VertexBuffer::~VertexBuffer()
{
	if (p_buffer) delete p_buffer;
	if (p_stage_buffer) delete p_stage_buffer;
}

void ThirdParty::VertexBuffer::bindData(void* data)
{
	if (p_stage_buffer) {
		p_stage_buffer
			->bindData(data)
			->copyData(p_buffer);
		delete p_stage_buffer;
		p_stage_buffer = nullptr;
	}

}

