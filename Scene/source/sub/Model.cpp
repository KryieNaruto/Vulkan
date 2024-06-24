#include "sub/Model.h"
#include "sub/Material.h"

Core::Resource::Model::Model(const std::string& _path, const std::string& _name /*= "Model"*/)
	:RModel(), SceneObject(_name)
{
	m_scene_type = MODEL;
	m_path = _path;
	auto _loaded = (Model*)getModel(m_path);
	if (!_loaded) {
		loadFromFile(_path);
		p_loaded_models[m_path] = this;
	}
	else {
		loadFromModel(_loaded);
		p_mesh_no_texture = _loaded->p_mesh_no_texture;
		p_merge_meshes = _loaded->p_merge_meshes;
		p_merge_meshes_vec = _loaded->p_merge_meshes_vec;
		m_merge_finish = _loaded->m_merge_finish;
		// 将loaded 已有的顶点类型以this保存到p_vertex_buffers中
		for (const auto& [_input, _buffers_ite] : p_vertex_buffers) {
			p_vertex_buffers[_input][this] = p_vertex_buffers[_input][_loaded];
		}
	}
	// 转化Material
	getMaterials();
	addModel(this);
	setZResourceProperty(p_property);
}

Core::Resource::Model::Model()
	:RModel(), SceneObject("Model")
{
	addModel(this);
	setZResourceProperty(p_property);
}

Core::Resource::Model::~Model()
{
	if (!m_is_copy) {
		if (p_mesh_no_texture) delete p_mesh_no_texture;
		for (const auto& _merge_mesh : p_merge_meshes_vec) {
			// 剔除mesh中重复的
			for (auto _m = p_meshes.begin(); _m < p_meshes.end(); _m++)
				if (*_m == _merge_mesh) {
					p_meshes.erase(_m);
					break;
				}
			delete _merge_mesh;
		}
		p_merge_meshes_vec.clear();
	}
}

void Core::Resource::Model::propertyEditor()
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
				setShader(_shader.second);
			}
		}
		ImGui::EndCombo();
	}
}

void Core::Resource::Model::merge()
{
	p_property->update();
	if (isFinished() && !m_merge_finish) {
		m_merge_finish = true;
		// 根据Matrial Index 区分meshes
		std::unordered_map<uint32_t, std::vector<RMesh*>> p_meshes_temp;
		for (const auto& _mesh : p_meshes) {
			uint32_t _m_index = _mesh->getMaterialIndex();
			p_meshes_temp[_m_index].push_back(_mesh);
		}

		// 将meshes 合并为一个大的Mesh
		std::unordered_map<uint32_t, Mesh*> _p_meshes;
		for (const auto& [_mat_index, _meshes] : p_meshes_temp) {
			Mesh* p_mesh = new Mesh(_meshes);
			p_mesh->setParent(this);
			_p_meshes[_mat_index] = p_mesh;
		}
		// 销毁原有meshes
		for (const auto& _mesh : p_meshes) {
			delete _mesh;
		}
		p_meshes.clear();
		p_meshes_temp.clear();
		p_merge_meshes.clear();
		// 重新构建merge后的mesh
		uint32_t _mesh_index = 0;
		for (const auto& [_index, _p_mesh] : _p_meshes) {
			_p_mesh->setIndex(_mesh_index++);
			p_meshes.push_back(_p_mesh);
			p_merge_meshes[_index].push_back(_p_mesh);
			p_merge_meshes_vec.push_back(_p_mesh);
		}
		// 白模
		p_mesh_no_texture = new Mesh(p_meshes);
	}
}

void Core::Resource::Model::draw(VkCommandBuffer _cmd, Shader_Data_Binding_Func _func)
{
	if (isFinished()) {
		merge();
		if (m_is_hidden) return;
		// 绑定vertex buffer
		VkBuffer _vertex_buffer[] = { p_vertex_buffer->getVkBuffer() };
		VkDeviceSize _vertex_size[] = { 0 };
		vkCmdBindVertexBuffers(_cmd, 0, 1, _vertex_buffer, _vertex_size);

		// 如果设置了render_camera
		if (p_render_camera) {
			if (p_materials_copy_camera_use.contains(p_render_camera)) {
				if (p_materials_copy_camera_use[p_render_camera].contains(p_shader)) {
					p_materials_copy = p_materials_copy_camera_use[p_render_camera][p_shader];
					m_temp_draw_in_material = true;
				}
			}
		}

		// 白模
		if (p_property->m_texture_count == 0) {
			auto _mat = p_materials_copy[0];		// 使用Material 而非RMaterial类型
			if (!_mat->getShader()->isReloadInMaterialEditor()) {
				_mat->bind(_cmd);
				p_mesh_no_texture->update(p_property);
				_func(p_parent_scene, _mat, this, 0, p_mesh_no_texture, p_render_camera);
				p_mesh_no_texture->draw(_cmd);
			}
		}
		else {
			for (const auto& [_mat_index, _meshes] : p_merge_meshes) {
				auto _mat = p_materials_copy[_mat_index];
				if (_mat && p_parent_scene) {
					if (!_mat->getShader()->isReloadInMaterialEditor()) {
						// 此处的material 必须是对应一个 mesh,
						auto _mesh = _meshes[0];
						_mesh->update(p_property);
						_func(p_parent_scene, _mat, this, 0, _mesh, p_render_camera);
						_mat->bind(_cmd);
						_mesh->draw(_cmd);
					}
				}
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
}

void Core::Resource::Model::draw(VkCommandBuffer _cmd)
{
	if (isFinished()) {
		merge();
		if (m_is_hidden) return;
		// 绑定vertex buffer
		VkBuffer _vertex_buffer[] = { p_vertex_buffer->getVkBuffer() };
		VkDeviceSize _vertex_size[] = { 0 };
		vkCmdBindVertexBuffers(_cmd, 0, 1, _vertex_buffer, _vertex_size);
		// 白模
		if (p_property->m_texture_count == 0) {
			p_mesh_no_texture->update(p_property);
			p_mesh_no_texture->draw(_cmd);
		}
		else {
			for (const auto& [_mat_index, _meshes] : p_merge_meshes) {
				auto _mat = p_materials_copy[_mat_index];
				if (_mat && p_parent_scene) {
					for (const auto& _mesh : _meshes) {
						_mesh->draw(_cmd);
					}
				}
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
}

const std::vector<Core::Resource::Material*>& Core::Resource::Model::getMaterials()
{
	if (p_materials.size() != p_materials_copy.size()) {
		for (const auto& _rmat : p_materials) {
			Material* _mat = new Material(_rmat);
			p_materials_copy.push_back(_mat);
			Core::Core_Pool::removeResource(_rmat->getUUID(), _rmat);
			delete _rmat;
		}
		// 转换完成后，销毁旧RMaterial
		p_materials.clear();
		// 从all_resource剔除

	}
	return p_materials_copy;
}

const std::unordered_map<uint32_t, std::vector<Core::Resource::Mesh*>> Core::Resource::Model::getMeshes()
{
	return p_merge_meshes;
}

Core::Resource::Mesh* Core::Resource::Model::getMesh(uint32_t _index)
{
	return p_merge_meshes_vec[_index];
}

Core::Resource::Model* Core::Resource::Model::setShader(Shader* _shader, bool _temp)
{
	if (_shader == nullptr) return this;
	// 保存旧顶点
	if (p_vertex_buffer) {
		p_vertex_buffer_last = p_vertex_buffer;
		if (!p_materials_copy.empty()) {
			auto _input = p_materials_copy[0]->getVertexInputFlag();
			p_vertex_buffers[_input][this] = p_vertex_buffer;
		}
	}
	// 保存旧Materials
	if (!p_materials_copy.empty()) {
		p_materials_copy_loaded[p_shader] = p_materials_copy;
		p_materials_copy.clear();
	}

	p_shader_last = p_shader;
	p_shader = _shader;
	if (!_temp) p_shader_last = p_shader;
	m_temp_draw_in_material = _temp;
	
	// 查找Material是否被创建
	if (p_materials_copy_loaded.contains(_shader)) {
		p_materials_copy = p_materials_copy_loaded[_shader];
	}
	// 创建Material
	else {
		RModel::setShader(_shader);
		getMaterials();
		p_shader_loaded.insert(_shader);
		p_materials_copy_loaded[_shader] = p_materials_copy;
	}

	// 将Material_copy 复制到camera_use
	if (p_render_camera) {
		// 如果是新摄像
		if (!p_materials_copy_camera_use.contains(p_render_camera)) {
			std::vector<Material*> _mtas;
			for (const auto& _copy : p_materials_copy) {
				Material* _mat = new Material(_copy);
				_mtas.push_back(_mat);
			}
			p_materials_copy_camera_use[p_render_camera][_shader] = _mtas;
		}
		else {
			// 如果是新Shader
			if (!p_materials_copy_camera_use[p_render_camera].contains(p_shader)) {
				std::vector<Material*> _mtas;
				for (const auto& _copy : p_materials_copy) {
					Material* _mat = new Material(_copy);
					_mtas.push_back(_mat);
				}
				p_materials_copy_camera_use[p_render_camera][_shader] = _mtas;
			}
		}
	}


	// 获取Vertex buffer
	auto _mat = p_materials_copy[0];
	auto _input = _mat->getVertexInputFlag();
	auto _stride = _mat->getVertexInputStride();
	if (p_vertex_buffers.contains(_input)) {
		if (!p_vertex_buffers[_input].contains(this)) {
			setupVkData(true, _input, _stride);
			p_vertex_buffers[_input][this] = p_vertex_buffer;
		}
	}
	else {
		setupVkData(true, _input, _stride);
		p_vertex_buffers[_input][this] = p_vertex_buffer;
	}
	p_vertex_buffer = p_vertex_buffers[_input][this];
	if (!_temp)p_vertex_buffer_last = p_vertex_buffer;


	return this;
}

Core::Resource::Model* Core::Resource::Model::setMaterial(Material* _material)
{
	// 保存当前vertex 格式
	p_vertex_buffer_last = p_vertex_buffer;
	p_shader_last = p_shader;
	p_shader = _material->getShader();
	// 设置临时标志位
	m_temp_draw_in_material = true;
	auto _input = _material->getVertexInputFlag();
	auto _stride = _material->getVertexInputStride();
	// 生成顶点
	setupVkData(true, _input, _stride);
	if (p_materials_copy.size() == 0) p_materials_copy.push_back(_material);
	else if (p_materials_copy.size() == 1) p_materials_copy[0] = _material;
	return this;
}

bool Core::Resource::Model::isReady()
{
	return isFinished() && m_merge_finish && m_initialized && !m_is_hidden;
}

Core::Resource::Model* Core::Resource::Model::setRenderCamera(void* _cam)
{
	p_render_camera = _cam;
	return this;
}

Core::Resource::Model* Core::Resource::Model::setCurrentMaterial(Material* _material)
{
	setShader(_material->getShader(), false);
	return this;
}
