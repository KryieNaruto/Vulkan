#include "RMesh.h"

Core::Resource::RMesh::RMesh(const std::string& _name /*= "RMesh"*/)
{
	this->m_res_name = _name;
	this->m_json_title = "RMesh";
	this->p_id_generate->init(RESOURCE_TYPE::RMESH);
	this->m_uuid = this->p_id_generate->generateUUID();
	p_property = new RMeshProperty;
}

Core::Resource::RMesh::RMesh(const std::vector<uint32_t>& _indices, const int& _texture_index, const std::string& _name /*= "RMesh"*/)
{
	this->m_indices = _indices;
	this->m_texture_index = _texture_index;
	this->m_res_name = _name;
	this->m_json_title = "RMesh";
	this->p_id_generate->init(RESOURCE_TYPE::RMESH);
	this->m_uuid = this->p_id_generate->generateUUID();
	p_property = new RMeshProperty;
}

Core::Resource::RMesh::RMesh(const int& _texture_index, const std::string& _name /*= "RMesh"*/)
{
	this->m_texture_index = _texture_index;
	this->m_res_name = _name;
	this->m_json_title = "RMesh";
	this->p_id_generate->init(RESOURCE_TYPE::RMESH);
	this->m_uuid = this->p_id_generate->generateUUID();
	p_property = new RMeshProperty;
}

Core::Resource::RMesh::~RMesh()
{
	if (p_index_buffer) delete p_index_buffer;
	if (p_property) delete p_property;
}

Json::Value Core::Resource::RMesh::serializeToJSON()
{
	return Json::Value();
}

void Core::Resource::RMesh::deserializeToObj(Json::Value& _root)
{

}

void Core::Resource::RMesh::draw(VkCommandBuffer _cmd)
{
	if (p_property->m_hidden) return;
	// 绑定vertex
	//std::vector<VkBuffer> buffers = { p_vertex_buffer->getVkBuffer() };
	//// 基于buffer的offset而非memory
	//std::vector<VkDeviceSize> buffer_offsets = { 0 };
	//vkCmdBindVertexBuffers(_cmd, 0, buffers.size(), buffers.data(), buffer_offsets.data());

	if (p_index_buffer) {
		// 这里的offset是基于buffer
		vkCmdBindIndexBuffer(_cmd, p_index_buffer->getVkBuffer(), 0, VK_INDEX_TYPE_UINT32);
		vkCmdDrawIndexed(_cmd, m_indices.size(), 1, 0, 0, 0);
	}
	else {
		//std::string title = std::format("%s:[Index Buffer]", m_res_name);
		p_index_buffer = new ThirdParty::IndexBuffer(sizeof(uint32_t) * m_indices.size());
		p_index_buffer->bindData(m_indices.data());
	}
}

void Core::Resource::RMesh::update(RMeshProperty* _parent)
{
	if (p_property)
		p_property->update(_parent);
}

void Core::Resource::RMesh::setup()
{
	/*size_t _stride = p_material->getVertexInputStride();
	VERTEX_INPUT_FLAG _input = p_material->getVertexInputFlag();*/
	//size_t _size = p_vertices.size() * _stride;

	/*p_vertices_in_shader = malloc(_size);
	size_t _vertex_offset = 0;
	for (const auto& vertex : p_vertices) {
		vertex->pushData(_input, p_vertices_in_shader, _vertex_offset);
	}*/
	// 创建buffer
	//std::string title = std::format("%s:[Vertex Buffer]", m_res_name);
	//p_vertex_buffer = new ThirdParty::VertexBuffer(_size, title);
	// 绑定数据
	//p_vertex_buffer->bindData(p_vertices_in_shader);
}

void Core::Resource::RMesh::initTemplate()
{
}

Core::Resource::RMesh* Core::Resource::RMesh::mergeRMeshes(const std::vector<RMesh*>& _meshes)
{
	for (const auto& _mesh : _meshes) m_indices.insert(m_indices.end(), _mesh->m_indices.begin(), _mesh->m_indices.end());
	return this;
}
