#include "resource_inner_global.h"
#include <Core/include/core_global.h>


void* Core::Resource::Pool::getRRenderPass(const std::string& _name)
{
	return Core::Core_Pool::g_all_renderpass[_name];
}

ThirdParty::Pipeline* Core::Resource::Pool::getThirdParty_Pipeline(const std::string& _name)
{
	return (ThirdParty::Pipeline*)Core::Core_Pool::g_all_pipelines[_name];
}


void Core::Resource::Pool::addRRenderPassIntoPool(const std::string& _name, void* _r)
{
	Core::Core_Pool::g_all_renderpass.insert(std::make_pair(_name, _r));
}

void Core::Resource::Pool::addThirdParty_PipelineInfoPool(const std::string& _name, ThirdParty::Pipeline* _r)
{
	Core::Core_Pool::g_all_pipelines.insert(std::make_pair(_name, _r));
}

void Core::Resource::Pool::addThirdParty_SamplerInfoPool(const std::string& _name, ThirdParty::Sampler* _r)
{
	Core::Core_Pool::g_all_samplers[_name] = _r;
}

void Core::Resource::CameraProperty::update()
{
	m_is_camera = true;
	m_canvas_vec2 = { m_canvas_2d.width ,m_canvas_2d.height };
	if (m_window_size.x == 0 || m_window_size.y == 0)
		m_window_size = m_canvas_vec2;
	m_aspect = m_canvas_2d.height != 0 ? (float)m_canvas_2d.width / (float)m_canvas_2d.height : 1.0f;
	m_fov_rad = glm::radians(m_fov_deg);
	// 如果是FPS， 则需要通过鼠标移动距离来计算增加角度
	if (m_controller_type == FIRST_PERSON_VIEW) {
		auto _move = m_axis_rotate;
		glm::vec2 _ndc_size = { m_canvas_2d.width,m_canvas_2d.height };
		float _x = m_fov_deg * _move.x / _ndc_size.x;
		float _y = m_fov_deg * _move.y / _ndc_size.y;
		m_axis_rotate = glm::vec3(_x, _y, 0);

	}
	// 仅计算坐标轴
	ObjectProperty::update();
	if (m_input) {
		m_mouse_pos_relative = ThirdParty::sdl_manager.getMousePos_vec2() - m_window_pos;
		m_last_mouse_pos_relative = ThirdParty::sdl_manager.getLastMousePos_vec2() - m_window_pos;
		// 绕点旋转, Target 不动， Pos动
		if (m_controller_type == ROTATION_AROUND_POINT) {
			// m_move 控制Target的移动
			if (m_move.x != 0 || m_move.y != 0 || m_move.z != 0) {
				auto _last = ThirdParty::calcScreenToWorld(m_last_mouse_pos_relative, m_window_pos, m_mat4_proj_inverse, m_mat4_view_inverse);
				auto _current = ThirdParty::calcScreenToWorld(m_mouse_pos_relative, m_window_pos, m_mat4_proj_inverse, m_mat4_view_inverse);
				m_move = _current - _last;

				m_vec3_target += m_move;
				/*m_vec3_target += m_move.x * m_X;
				m_vec3_target += m_move.y * m_Y;
				m_vec3_target += m_move.z * m_Z;*/
				m_move = glm::vec3(0);
			}
			// 滚轮后划为负，前划为正，后划R增大
			m_rotate_R -= m_wheel * m_move_speed;
			if (m_rotate_R < 0.0f)
				m_rotate_R = 0.1f;
			m_front *= m_rotate_R;
			// 通过m_Rotate_R 计算Pos
			m_vec3_pos = m_vec3_target - m_front;
		}
		// 第一人称，Pos不动， Target动
		else if (m_controller_type == FIRST_PERSON_VIEW) {
			if (m_move.x != 0 || m_move.y != 0 || m_move.z != 0) {
				m_vec3_pos += m_move.x * m_X;
				m_vec3_pos += m_move.y * m_Y;
				m_vec3_pos += m_move.z * m_Z;
				m_move = glm::vec3(0);
			}
			m_vec3_pos -= m_wheel * m_Z * m_move_speed;
		}
		m_wheel = 0;
	}

	m_vec3_target = m_vec3_pos + m_front;
	// 计算view
	m_mat4_view = glm::lookAt(m_vec3_pos, m_vec3_target, m_up);
	// 计算proj
	if (m_project_type == PERSPECTIVE) {
		m_mat4_proj = glm::perspective(m_fov_rad, m_aspect, m_zNear, m_zFar);
		m_mat4_proj[1][1] *= -1;
		// 计算_VP
		m_mat4_view_proj = m_mat4_proj * m_mat4_view;
		// 计算_VP_INVERSE
		m_mat4_view_inverse = glm::inverse(m_mat4_view);
		m_mat4_proj_inverse = glm::inverse(m_mat4_proj);
		m_mat4_view_proj_inverse = m_mat4_view_inverse * m_mat4_proj_inverse;
		// 统一输出
		m_mat4_ortho = m_mat4_proj;
		m_mat4_view_ortho = m_mat4_view_proj;
	}
	if (m_project_type == ORTHO) {
		m_project_width = m_parallel_R;
		m_project_height = m_parallel_R / m_aspect;
		m_mat4_view = glm::lookAt(m_vec3_pos, m_vec3_target, m_up);
		m_mat4_ortho = glm::ortho(-m_project_width, m_project_width, -m_project_height, m_project_height, m_zNear, m_zFar);
		m_mat4_ortho[1][1] *= -1;
		m_mat4_view_ortho = m_mat4_ortho * m_mat4_view;
		// 统一输出
		m_mat4_proj = m_mat4_ortho;
		m_mat4_view_proj = m_mat4_view_ortho;
		// 计算_VP_INVERSE
		m_mat4_view_inverse = glm::inverse(m_mat4_view);
		m_mat4_proj_inverse = glm::inverse(m_mat4_proj);
		m_mat4_view_proj_inverse = m_mat4_view_inverse * m_mat4_proj_inverse;
	}


}

void Core::Resource::CameraProperty::update(ObjectProperty* _obj_prop)
{
	setParentProperty(_obj_prop);
	m_vec3_pos = _obj_prop->m_vec3_pos;
	m_rotate_euler_world_degree = _obj_prop->m_rotate_euler_world_degree;
	m_parallel_R = _obj_prop->m_parallel_R;
	update();
}

void Core::Resource::CameraProperty::initTemplate()
{
	RObjectProperty::initTemplate();
	// Pos
	{
		auto _output = p_template->p_outputs[0];
		_output->setName3("Camera");
	}
	// 将数据作为输出项，无输入项
	// View
	{
		auto _output = new ThirdParty::imgui::SlotTemplateOutput();
		_output->setName("View");
		_output->setName2("View");
		_output->p_data = &m_mat4_view;
		p_template->addOutputSlotTemplate(_output);
	}
	// Projection
	{
		auto _output = new ThirdParty::imgui::SlotTemplateOutput();
		_output->setName("Projection");
		_output->setName2("Projection");
		_output->p_data = &m_mat4_proj;
		p_template->addOutputSlotTemplate(_output);
	}
	// View Projection
	{
		auto _output = new ThirdParty::imgui::SlotTemplateOutput();
		_output->setName("View-Projection");
		_output->setName2("View-Projection");
		_output->p_data = &m_mat4_view_proj;
		p_template->addOutputSlotTemplate(_output);
	}
}

void Core::Resource::RLightProperty::update()
{
	// 计算光照方向, 默认向Z。
	// 计算物体坐标轴
	{
		// UI 控制Euler ,控制四元数旋转轴
		m_rotate_euler_world_radians = glm::radians(m_rotate_euler_world_degree);
		m_rotate_axis_degree = m_rotate_euler_world_degree;
		m_rotate_axis_radians = m_rotate_euler_world_radians;
		// 通过XYZ四元数旋转轴，计算最终旋转轴
		m_rotate_quat_world_x = glm::angleAxis(m_rotate_axis_radians.x, glm::vec3(1, 0, 0));
		m_rotate_quat_world_y = glm::angleAxis(m_rotate_axis_radians.y, glm::vec3(0, 1, 0));
		m_rotate_quat_world_z = glm::angleAxis(m_rotate_axis_radians.z, glm::vec3(0, 0, 1));
		m_rotate_quat_world = glm::normalize(m_rotate_quat_world_z * m_rotate_quat_world_y * m_rotate_quat_world_x);
		// 四元数旋转计算Front
		m_Z = glm::normalize(m_rotate_quat_world * m_world_Z);	// 获取m_Z;
		m_X = glm::cross(m_world_Y, m_Z);
		m_Y = glm::cross(m_Z, m_X);

		m_front = -m_Z;
		m_right = m_X;
		m_up = m_Y;
	}

	m_cut_off = glm::cos(glm::radians(m_cut_off_deg / 2.0f));
	m_cut_off_outer = glm::cos(glm::radians(m_cut_off_outer_deg / 2.0f));

	// 如果是平行光，则通过方向计算位置
	if (m_type == PARALLEL) {
		m_vec3_pos = glm::vec3(0) - m_front * m_parallel_R;
	}

	if (m_model_last != m_model || m_type_last != m_type) {
		m_is_change = true;
		m_model_last = m_model;
		m_type_last = m_type;
	}
	else {
		m_is_change = false;
	}
}

Core::Resource::RLightProperty::RLightProperty()
{
	m_name = "RLight Property";
	m_rotate_euler_world_degree.x = 90.0f;
}

void Core::Resource::RLightProperty::initTemplate()
{
	RObjectProperty::initTemplate();
	// Pos
	{
		auto _output = p_template->p_outputs[0];
		_output->setName3("Light");
	}
	// COLOR
	{
		auto _output = new ThirdParty::imgui::SlotTemplateOutput();
		_output->setName("Color");
		_output->setName2("Color");
		_output->p_data = &m_color;
		_output->m_functional = [this, _output]() -> void {
			if (ImGui::CollapsingHeader("Color")) {
				auto _width = _output->m_final_size.x - ImGui::GetFont()->CalcTextSizeA(ImGui::GetFont()->FontSize, FLT_MAX, 0.0f, "Color").x;
				ImGui::SetNextItemWidth(_width);
				ImGui::ColorEdit3("Color", glm::value_ptr(m_color));
			}
			};
		p_template->addOutputSlotTemplate(_output);
	}
	// 辐射率
	{
		auto _output = new ThirdParty::imgui::SlotTemplateOutput();
		_output->setName("Radiance");
		_output->setName2("Radiance");
		_output->p_data = &m_radiance;
		_output->m_functional = [this, _output]() -> void {
			if (ImGui::CollapsingHeader("Radiance")) {
				auto _width = _output->m_final_size.x - ImGui::GetFont()->CalcTextSizeA(ImGui::GetFont()->FontSize, FLT_MAX, 0.0f, "Radiance (?) ").x;
				ImGui::SetNextItemWidth(_width);
				ImGui::DragFloat("Radiance", &m_radiance);
				ThirdParty::imgui::help("辐射率，光照强度。PBR属性");
			}
			};
		p_template->addOutputSlotTemplate(_output);
	}
	// 光强度
	{
		auto _output = new ThirdParty::imgui::SlotTemplateOutput();
		_output->setName("Ambient/Strength");
		_output->setName2("Ambient");
		_output->p_data = &m_strength;
		_output->m_functional = [this, _output]() -> void {
			if (ImGui::CollapsingHeader("Ambient")) {
				auto _width = _output->m_final_size.x - ImGui::GetFont()->CalcTextSizeA(ImGui::GetFont()->FontSize, FLT_MAX, 0.0f, "Ambient (?) ").x;
				ImGui::SetNextItemWidth(_width);
				ImGui::DragFloat("Ambient", &m_strength);
				ThirdParty::imgui::help("环境光强度");
			}
			};
		p_template->addOutputSlotTemplate(_output);
	}
}

Core::Resource::LIGHT_TYPE Core::Resource::getLightType(uint32_t _index)
{
	switch (_index)
	{
	case 0:
		return AMBIENT;
	case 1:
		return POINT;
	case 2:
		return PARALLEL;
	case 3:
		return SPOT;
	default:
		return AMBIENT;
		break;
	}
}

uint32_t Core::Resource::getLightTypeIndex(LIGHT_TYPE _type)
{
	switch (_type)
	{
	case Core::Resource::AMBIENT:
		return 0;
		break;
	case Core::Resource::POINT:
		return 1;
		break;
	case Core::Resource::PARALLEL:
		return 2;
		break;
	case Core::Resource::SPOT:
		return 3;
		break;
	default:
		return 0;
		break;
	}
}

void Core::Resource::RObjectProperty::update()
{
	// 响应Input
	if (m_input) {
		if (m_axis_rotate.x != 0 || m_axis_rotate.y != 0 || m_axis_rotate.z != 0) {
			m_rotate_euler_world_degree += m_axis_rotate;

			if (m_rotate_euler_world_degree.x > 89.0f)
				m_rotate_euler_world_degree.x = 89.0f;
			else if (m_rotate_euler_world_degree.x < -89.0f)
				m_rotate_euler_world_degree.x = -89.0f;
			if (m_rotate_euler_world_degree.y > 360.0f)
				m_rotate_euler_world_degree.y = 0.0f;
			else if (m_rotate_euler_world_degree.y < -360.0f)
				m_rotate_euler_world_degree.y = 0.0f;

			m_axis_rotate = glm::vec3(0);
		}

		m_time_last = m_time_current;
		m_time_current = ImGui::GetTime();
	}
	// 右手坐标系
	//		Y
	//		|___ X
	//	   /
	//	  Z
	// 计算物体坐标轴
	{
		// UI 控制Euler ,控制四元数旋转轴
		m_rotate_euler_world_radians = glm::radians(m_rotate_euler_world_degree);
		m_rotate_axis_degree = m_rotate_euler_world_degree;
		m_rotate_axis_radians = m_rotate_euler_world_radians;
		// 通过XYZ四元数旋转轴，计算最终旋转轴
		m_rotate_quat_world_x = glm::angleAxis(m_rotate_axis_radians.x, glm::vec3(1, 0, 0));
		m_rotate_quat_world_y = glm::angleAxis(m_rotate_axis_radians.y, glm::vec3(0, 1, 0));
		m_rotate_quat_world_z = glm::angleAxis(m_rotate_axis_radians.z, glm::vec3(0, 0, 1));
		m_rotate_quat_world = glm::normalize(m_rotate_quat_world_z * m_rotate_quat_world_y * m_rotate_quat_world_x);
		// 四元数旋转计算Front
		m_Z = glm::normalize(m_rotate_quat_world * m_world_Z);	// 获取m_Z;
		m_X = glm::cross(m_world_Y, m_Z);
		m_Y = glm::cross(m_Z, m_X);

		m_front = -m_Z;
		m_right = m_X;
		m_up = m_Y;
	}

	// 计算model
	glm::mat4 _model_trans = glm::translate(glm::mat4(1), m_vec3_pos);
	glm::mat4 _model_ratet = glm::mat4_cast(m_rotate_quat_world);
	glm::mat4 _model_sacle = glm::scale(glm::mat4(1), m_scale);
	m_model = _model_trans * _model_ratet * _model_sacle;

	// 父空间计算
	if (p_parent_object) {
		m_model = p_parent_object->m_model * m_model;
	}
	m_model_inverse = glm::inverse(m_model);

	if (m_model_last != m_model) {
		m_is_change = true;
		m_model_last = m_model;
	}
	else {
		m_is_change = false;
	}
}

void Core::Resource::RObjectProperty::update(RObjectProperty* _parent)
{
	setParentProperty(_parent);
	update();
}

Core::Resource::RObjectProperty* Core::Resource::RObjectProperty::getParentProperty()
{
	return p_parent_object;
}

Core::Resource::RObjectProperty* Core::Resource::RObjectProperty::setParentProperty(RObjectProperty* _property)
{
	p_parent_object = _property;
	return this;
}

Core::Resource::RObjectProperty::RObjectProperty()
{
	if (!g_imgui_context) {
		g_imgui_context = ThirdParty::imgui_manager.getCurrentContext();
		ImGui::SetCurrentContext(g_imgui_context);
	}
	if (!p_template) {
		p_template = new ThirdParty::imgui::Template();
	}
}

void Core::Resource::RObjectProperty::initTemplate()
{
	// Pos
	{
		auto _output = new ThirdParty::imgui::SlotTemplateOutput();
		_output->setName("Pos");
		_output->setName2("Pos");
		_output->p_data = &m_vec3_pos;
		_output->m_functional = [this, _output]() -> void {
			if (ImGui::CollapsingHeader("Position")) {
				auto _width = _output->m_final_size.x - ImGui::GetFont()->CalcTextSizeA(ImGui::GetFont()->FontSize, FLT_MAX, 0.0f, "Pos").x;
				ImGui::SetNextItemWidth(_width);
				ImGui::DragFloat3("Pos", glm::value_ptr(m_vec3_pos));
			}
			};
		p_template->addOutputSlotTemplate(_output);
	}
}

Core::Resource::RObjectProperty::~RObjectProperty()
{
}

void Core::Resource::RMeshProperty::update(RMeshProperty* _parent)
{
	setParentProperty(_parent);
	m_texture_count = _parent->m_texture_count;
	m_shinness = _parent->m_shinness;
	m_vec3_color = _parent->m_vec3_color;
	m_has_shadow = _parent->m_has_shadow;
	RObjectProperty::update(_parent);
}

void Core::Resource::RMeshProperty::initTemplate()
{
	SceneObjectProperty::initTemplate();
	// COLOR
	{
		auto _output = new ThirdParty::imgui::SlotTemplateOutput();
		_output->setName("Color");
		_output->setName2("Color");
		_output->p_data = &m_vec3_color;
		_output->m_functional = [this, _output]() -> void {
			if (ImGui::CollapsingHeader("BaseColor")) {
				auto _width = _output->m_final_size.x - ImGui::GetFont()->CalcTextSizeA(ImGui::GetFont()->FontSize, FLT_MAX, 0.0f, "BaseColor").x;
				ImGui::SetNextItemWidth(_width);
				ImGui::ColorEdit3("BaseColor", glm::value_ptr(m_vec3_color));
			}
			};
		p_template->addOutputSlotTemplate(_output);
	}
}

void Core::Resource::SceneObjectProperty::initTemplate()
{
	ObjectProperty::initTemplate();
	{
		auto _output = new ThirdParty::imgui::SlotTemplateOutput();
		_output->setName("Model Matrix");
		_output->setName2("Model");
		_output->setName3("Model");
		_output->p_data = &m_model;
		p_template->addOutputSlotTemplate(_output);
	}
}
