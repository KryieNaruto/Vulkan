#include "sub/shape/Shape.h"

void Core::Resource::Shape::draw(VkCommandBuffer _cmd, Shader_Data_Binding_Func _func)
{
	auto _vertex_buffer = getVertexBuffer();
	auto _mesh = p_merge_meshes_vec[0];
	if (_vertex_buffer && _mesh) {
		VkBuffer _v[] = { _vertex_buffer->getVkBuffer() };
		VkDeviceSize _o[] = { 0 };
		vkCmdBindVertexBuffers(_cmd, 0, 1, _v, _o);

		// 如果设置了render_camera
		if (p_render_camera) {
			if (p_materials_copy_camera_use.contains(p_render_camera)) {
				if (p_materials_copy_camera_use[p_render_camera].contains(p_shader)) {
					p_materials_copy = p_materials_copy_camera_use[p_render_camera][p_shader];
					m_temp_draw_in_material = true;
				}
			}
		}

		auto _mat = getMaterials()[0];
		if (_mat) {
			if (!_mat->getShader()->isReloadInMaterialEditor()) {
				p_property->update();
				_mesh->getProperty()->update(p_property);
				if (_func)
					_func(p_parent_scene, _mat, this, 0, _mesh, p_render_camera);
				_mat->bind(_cmd);
				_mesh->draw(_cmd);
			}
		}
	}
	else if(_vertex_buffer == nullptr) {
		auto _mat = getMaterials()[0];
		if (_mat->getVertexInputStride() != 0)
			setShader(_mat->getShader());
		else {
			// 如果设置了render_camera
			if (p_render_camera) {
				if (p_materials_copy_camera_use.contains(p_render_camera)) {
					if (p_materials_copy_camera_use[p_render_camera].contains(p_shader)) {
						p_materials_copy = p_materials_copy_camera_use[p_render_camera][p_shader];
						m_temp_draw_in_material = true;
					}
				}
			}
			p_property->update();
			_mesh->getProperty()->update(p_property);
			if (_func)
				_func(p_parent_scene, _mat, this, 0, _mesh, p_render_camera);
			if (_mat->bind(_cmd))
				vkCmdDraw(_cmd, 2, 1, 0, 0);
		}
	}

	// 如果是临时Material
	if (m_temp_draw_in_material) {
		m_temp_draw_in_material = false;
		p_vertex_buffer = p_vertex_buffer_last;
		p_shader = p_shader_last;
		p_materials_copy = p_materials_copy_loaded[p_shader];
		p_render_camera = nullptr;
	}
}

void Core::Resource::Shape::draw(SceneObject* _scene, Material* _mat, VkCommandBuffer _cmd, Shader_Data_Binding_Func _func, void* _camera)
{
	auto _vertex_buffer = getVertexBuffer();
	_vertex_buffer = p_vertex_buffers[_mat->getVertexInputFlag()][this];
	auto _mesh = p_merge_meshes_vec[0];
	if (_mat->getVertexInputStride() == 0) {
		_func(_scene, _mat, this, 0, _mesh, _camera);
		_mat->bind(_cmd);
		vkCmdDraw(_cmd, 6, 1, 0, 0);
	}
	else if (_scene && _mat && _vertex_buffer && _func) {
		_func(_scene, _mat, this, 0, _mesh, _camera);
		_mat->bind(_cmd);
		VkBuffer _v[] = { _vertex_buffer->getVkBuffer() };
		VkDeviceSize _o[] = { 0 };
		vkCmdBindVertexBuffers(_cmd, 0, 1, _v, _o);

		_mesh->draw(_cmd);
	}
	else {
		setShader(_mat->getShader());
		draw(_scene, _mat, _cmd, _func, _camera);
	}
}

void Core::Resource::Shape::draw(VkCommandBuffer _cmd)
{
	auto _vertex_buffer = getVertexBuffer();
	auto _mesh = p_merge_meshes_vec[0];
	if (_mesh && _vertex_buffer) {
		VkBuffer _v[] = { _vertex_buffer->getVkBuffer() };
		VkDeviceSize _o[] = { 0 };
		vkCmdBindVertexBuffers(_cmd, 0, 1, _v, _o);

		_mesh->draw(_cmd);
	}

	// 如果是临时Material
	if (m_temp_draw_in_material) {
		m_temp_draw_in_material = false;
		p_vertex_buffer = p_vertex_buffer_last;
		p_shader = p_shader_last;
		p_materials_copy = p_materials_copy_loaded[p_shader];
		p_render_camera = nullptr;
	}
}

void Core::Resource::Shape::propertyEditor()
{
	ImGui::Checkbox("Shadow", (bool*)&p_property->m_has_shadow);
	ImGui::DragFloat3("Position", glm::value_ptr(p_property->m_vec3_pos));
	ImGui::DragFloat3("Rotate", glm::value_ptr(p_property->m_rotate_euler_world_degree));
	ImGui::DragFloat3("Scale", glm::value_ptr(p_property->m_scale));
	ImGui::ColorEdit3("BaseColor", glm::value_ptr(p_property->m_vec3_color));
	ImGui::DragInt("Texture", &p_property->m_texture_count, 1, 0, 8);
	ImGui::DragInt("Shinness", &p_property->m_shinness, 1, 4, 256);
	static std::string _preview = p_shader->getName();
	if (ImGui::BeginCombo("Material Shader", _preview.c_str())) {
		auto _shaders = *Shader::p_shaders;
		for (const auto& _shader : _shaders) {
			if (ImGui::Selectable(_shader.first.c_str())) {
				_preview = _shader.first;
				setShader(_shader.second, false);
			}
		}
		ImGui::EndCombo();
	}
}

Core::Resource::Shape::~Shape()
{
	g_shape_count--;
	p_vertices.clear();
	// 从vkobject中移除
	if (p_vertex_buffer) {
		p_vertex_buffer->removeFromVkObjects();
		delete p_vertex_buffer;
		p_vertex_buffer = nullptr;
	}
	if (g_shape_count == 0) {
		auto& _v = RShape::getVertices();
		auto& _i = RShape::g_indices;
		for (const auto& v : _v) 
			for (const auto& __v : v.second) 
				delete __v;
		_v.clear();
		_i.clear();
	}
}

void Core::Resource::Shape::generateModel(uint32_t _quad)
{

}

void Core::Resource::Shape::setup()
{
	auto p_indices = getIndex(m_type);
	p_vertices = getVertex(m_type);
	// 将p_vertices p_indices 封装成RMesh
	RMesh* p_mesh = new RMesh(p_indices, 0, "RMesh");
	p_meshes.push_back(p_mesh);
	p_textures[0] = {};
	m_initialized = true;
	merge();
}

void Core::Resource::Shape::saveVertex(const std::string& _type, const std::vector<RVertex*>& _v)
{
	RShape::saveVertex(_type, _v);
}


void Core::Resource::Shape::saveIndex(const std::string& _type, const std::vector<uint32_t>& _v)
{
	RShape::saveIndex(_type, _v);
}

const std::vector<Core::Resource::RVertex*>& Core::Resource::Shape::getVertex(const std::string& _type)
{
	return RShape::getVertex(_type);
}

const std::vector<uint32_t>& Core::Resource::Shape::getIndex(const std::string& _type)
{
	return RShape::getIndex(_type);
}

Core::Resource::Shape::Shape(const std::string& _name)
	:Model() 
{
	saveSceneObject(_name, this);
	g_shape_count++;
	m_scene_type = SHAPE;
	p_vertex_buffer = nullptr;
}

void Core::Resource::Shape::initTemplate()
{

}

auto Core::Resource::Shape::calculateTangentAndBitangent(const glm::vec3 _pos[3], const glm::vec2 _uv[3])
{
	glm::vec3 edge1 = _pos[1] - _pos[0];
	glm::vec3 edge2 = _pos[2] - _pos[0];
	glm::vec2 deltaUV1 = _uv[1] - _uv[0];
	glm::vec2 deltaUV2 = _uv[2] - _uv[0];

	glm::vec3 tangent;
	float f = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y);
	tangent.x = f * (deltaUV2.y * edge1.x - deltaUV1.y * edge2.x);
	tangent.y = f * (deltaUV2.y * edge1.y - deltaUV1.y * edge2.y);
	tangent.z = f * (deltaUV2.y * edge1.z - deltaUV1.y * edge2.z);

	glm::vec3 bitTangent;
	bitTangent.x = f * (-deltaUV2.x * edge1.x + deltaUV1.x * edge2.x);
	bitTangent.y = f * (-deltaUV2.x * edge1.y + deltaUV1.x * edge2.y);
	bitTangent.z = f * (-deltaUV2.x * edge1.z + deltaUV1.x * edge2.z);
	if (tangent.x != 0.0f || tangent.y != 0.0f || tangent.z != 0.0f)
		tangent = glm::normalize(tangent);
	else
	{
		glm::vec3 _normal = glm::vec3(0);
		if (glm::length(edge2) == 0) {
			_normal = glm::normalize(_pos[1] + _pos[2]);
		}
		tangent = calculateTangent(_normal, bitTangent);
	}
	if (bitTangent.x != 0.0f || bitTangent.y != 0.0f || bitTangent.z != 0.0f)
		bitTangent = glm::normalize(bitTangent);
	else
		bitTangent = calculateTangent(glm::cross(edge1,edge2), tangent);

	return std::vector<glm::vec3>({ tangent,bitTangent });
}

void Core::Resource::Shape::generateTangentAndBitangent(const std::vector<uint32_t>& _indices)
{
	auto pos1 = p_vertices[_indices[0]];
	auto pos2 = p_vertices[_indices[1]];
	auto pos3 = p_vertices[_indices[2]];
	// 计算切线
	const glm::vec3 _pos[] = {
		pos1->m_position,
		pos2->m_position,
		pos3->m_position,
	};
	const glm::vec2 _uv[] = {
		pos1->m_texCoord,
		pos2->m_texCoord,
		pos3->m_texCoord,
	};
	auto _TB1 = calculateTangentAndBitangent(_pos, _uv);
	{
		pos1->m_tangent = _TB1[0];
		pos1->m_bitangent = _TB1[1];
		pos2->m_tangent = _TB1[0];
		pos2->m_bitangent = _TB1[1];
		pos3->m_tangent = _TB1[0];
		pos3->m_bitangent = _TB1[1];
	}
}

void Core::Resource::Shape::generateTangentAndBitangent(const std::vector<uint32_t>& _indices, int _indices_start)
{
	generateTangentAndBitangent({ 
		_indices[_indices_start],
		_indices[_indices_start + 1],
		_indices[_indices_start + 2], });
}

glm::vec3 Core::Resource::Shape::calculateTangent(const glm::vec3& _normal, const glm::vec3& _bitangent)
{
	return glm::normalize(glm::cross(_normal, _bitangent));
}
