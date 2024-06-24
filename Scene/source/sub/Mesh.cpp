#include "sub/Mesh.h"

Core::Resource::Mesh::Mesh(const std::vector<RMesh*>& _mesh)
	:RMesh(), SceneObject("Mesh")
{
	if (_mesh.size() == 1) {
		if(!p_property)
			p_property = new RMeshProperty;
		*p_property = *_mesh[0]->getProperty();
	}
	m_scene_type = MESH;
	mergeRMeshes(_mesh);
	setZResourceProperty(p_property);
}

void Core::Resource::Mesh::propertyEditor()
{
	ImGui::DragFloat3("Position", glm::value_ptr(p_property->m_vec3_pos));
	ImGui::DragFloat3("Rotate", glm::value_ptr(p_property->m_rotate_quat_world));
	ImGui::DragFloat3("Scale", glm::value_ptr(p_property->m_scale));
	ImGui::ColorEdit3("BaseColor", glm::value_ptr(p_property->m_vec3_color));
	ImGui::DragInt("Texture", &p_property->m_texture_count, 1, 0, 8);
	ImGui::DragInt("Shinness", &p_property->m_shinness, 1, 4, 256);
}

Core::Resource::Mesh::~Mesh()
{
	if (p_index_buffer) delete p_index_buffer;
	p_index_buffer = nullptr;
}

void Core::Resource::Mesh::draw(VkCommandBuffer _cmd)
{
	if (p_property->m_hidden) return;

	if (p_index_buffer) {
		// 这里的offset是基于buffer
		vkCmdBindIndexBuffer(_cmd, p_index_buffer->getVkBuffer(), 0, VK_INDEX_TYPE_UINT32);
		vkCmdDrawIndexed(_cmd, m_indices.size(), 1, 0, 0, 0);
	}
	else {
		//std::string title = std::format("{0}:[Index Buffer]", m_res_name);
		p_index_buffer = new ThirdParty::IndexBuffer(sizeof(uint32_t) * m_indices.size());
		p_index_buffer->bindData(m_indices.data());
		draw(_cmd);
	}
}

void Core::Resource::Mesh::draw(VkCommandBuffer _cmd, Shader_Data_Binding_Func _func)
{
	draw(_cmd);
}
