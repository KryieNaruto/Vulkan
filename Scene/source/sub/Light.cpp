#include "sub/Light.h"

Core::Resource::Light::Light(const std::string& _name)
	:RLight(Core::Resource::getName(_name)), SceneObject(_name)
{
	m_scene_type = LIGHT;
	setZResourceProperty(p_property);
}

Core::Resource::Light::Light(LIGHT_TYPE _type, const glm::vec4& _light_color, const std::string& _name /*= "Light"*/)
	:RLight(_type, { 0,0,0 }, _light_color), SceneObject(_name)
{
	m_scene_type = LIGHT;
	setZResourceProperty(p_property);
}

Core::Resource::Light::Light(LIGHT_TYPE _type, const glm::vec3& _dir, const glm::vec4& _light_color, const std::string& _name /*= "Light"*/)
	: RLight(_type, _dir, _light_color, _dir), SceneObject(_name)
{
	m_scene_type = LIGHT;
	setZResourceProperty(p_property);
}

Core::Resource::Light::Light(LIGHT_TYPE _type, const glm::vec3& _pos, const glm::vec3& _dir, const glm::vec4& _light_color, const std::string& _name /*= "Light"*/)
	:RLight(_type, _pos, _light_color, _dir), SceneObject(_name)
{
	m_scene_type = LIGHT;
	setZResourceProperty(p_property);
}

Core::Resource::Light::~Light()
{
	if (p_shape) {
		Model::removeModel(p_shape);
		delete p_shape;
	}
}

Core::Resource::RMesh* Core::Resource::Light::getMesh()
{
	return this->p_shape->getRMeshes()[0];
}

void Core::Resource::Light::propertyEditor()
{
	static const char* items[] = {
		"Ambient",
		"Point",
		"Parallel",
		"Spot"
	};
	auto _pro = getProperty();
	auto& type = _pro->m_type;
	int _type_index = getLightTypeIndex(type);
	
	if (ImGui::GetCurrentContext() == nullptr) {
		ImGui::SetCurrentContext(ThirdParty::imgui_manager.getCurrentContext());
	}

	// 颜色
	ImGui::ColorEdit3("Color", glm::value_ptr(_pro->m_color));
	// 强度
	ImGui::DragFloat("Strength", &_pro->m_strength, 0.01, 0.0, 1.0);
	// 类型
	if (ImGui::Combo("Type", &_type_index, items, IM_ARRAYSIZE(items))) {
		type = Core::Resource::getLightType(_type_index);
	}

	switch (type)
	{
	case LIGHT_TYPE::AMBIENT:
		break;
	case LIGHT_TYPE::PARALLEL:
		// 方向
		ImGui::DragFloat3("Direction", glm::value_ptr(_pro->m_rotate_euler_world_degree));
		ImGui::DragFloat("Ortho Distance", &_pro->m_parallel_R);
		break;
	case LIGHT_TYPE::SPOT: 
		// 位置
		ImGui::DragFloat3("Pos", glm::value_ptr(_pro->m_vec3_pos));
		// 方向
		ImGui::DragFloat3("Direction", glm::value_ptr(_pro->m_rotate_euler_world_degree));
		// 衰减
		ImGui::DragFloat3("Attenuation", glm::value_ptr(_pro->m_attenuation), 0.01f, 0.0f);
		ImGui::SameLine();
		ThirdParty::imgui::help("衰减因子:C/L/Q. 计算公式: A = 1 / (D+L*D+Q*D*D)");
		// 角度
		ImGui::DragFloat("Range(degree)", &_pro->m_cut_off_deg, 1.0f, 0.0f, 179.0f);
		ImGui::SameLine();
		ThirdParty::imgui::help("聚光灯内圈范围(超出范围则不会进行光照计算)");
		ImGui::DragFloat("Outer Range(degree)", &_pro->m_cut_off_outer_deg, 1.0f, 0.0f, 179.0f);
		ImGui::SameLine();
		ThirdParty::imgui::help("聚光灯外圈范围(超出范围则不会进行光照计算)");
		break;
	case LIGHT_TYPE::POINT: 
		// 位置
		ImGui::DragFloat3("Pos", glm::value_ptr(_pro->m_vec3_pos));
		// 衰减
		ImGui::DragFloat3("Attenuation", glm::value_ptr(_pro->m_attenuation), 0.01f, 0.0f);
		ImGui::SameLine();
		ThirdParty::imgui::help("衰减因子:C/L/Q. 计算公式: A = 1 / (D+L*D+Q*D*D)");
		// 范围
		ImGui::DragFloat("Radians", &_pro->m_radians, 1.0f, 0.0f);
		ImGui::SameLine();
		ThirdParty::imgui::help("光照范围(超出范围则不会进行光照计算)");
		break;
	default: 
		break;
	}

}

void Core::Resource::Light::draw(VkCommandBuffer _cmd, Shader_Data_Binding_Func _func)
{
	if (p_shape) {
		auto _mat = p_shape->getMaterials()[0];
		if (_mat) {
			_func(p_parent_scene, _mat, this, 0, nullptr, nullptr);
			_mat->bind(_cmd);
			// 绑定vertex buffer
			VkBuffer _vertex_buffer[] = { p_shape->getVertexBuffer()->getVkBuffer() };
			VkDeviceSize _vertex_size[] = { 0 };
			vkCmdBindVertexBuffers(_cmd, 0, 1, _vertex_buffer, _vertex_size);
			getMesh()->draw(_cmd);
		}
	}
}

Core::Resource::Light* Core::Resource::Light::setShader(Shader* _shader)
{
	if (p_shape) {
		p_shape->setShader(_shader);
	}
	return this;
}

Core::Resource::Light* Core::Resource::Light::update()
{
	if (p_property) {
		p_property->update();
		m_is_pos_change = p_property->m_is_change;
	}
	if (p_shape) {
		auto _mesh_data = p_shape->getProperty();
		_mesh_data->m_vec3_pos = p_property->m_vec3_pos;
		_mesh_data->m_front = p_property->m_front;
		_mesh_data->update();
	}
	return this;
}
