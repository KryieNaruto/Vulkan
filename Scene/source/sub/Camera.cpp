#include "sub/Camera.h"

Core::Resource::Camera::Camera(const VkExtent3D& _size, const std::string& _name /*= "Camera"*/)
	: RCamera(_size, Core::Resource::getName(_name)), SceneObject(_name)
{
	m_scene_type = CAMERA;
	setZResourceProperty(p_property);
	g_all_cameras.insert(this);
	initTemplate();
}
Core::Resource::Camera::Camera(const std::string& _name /*= "Camera"*/)
	: RCamera(Core::Resource::getName(_name)), SceneObject(_name)
{
	m_scene_type = CAMERA;
	setZResourceProperty(p_property);
	g_all_cameras.insert(this);
	initTemplate();
}

Core::Resource::Camera::~Camera()
{

}

Core::Resource::Camera* Core::Resource::Camera::setMainCamera()
{
	this->m_is_main = true;
	if (p_camera_property) {
		p_camera_property->m_input = true;
	}
	setup();
	return this;
}

void Core::Resource::Camera::propertyEditor()
{
	auto _p = p_camera_property;
	float _zndc[] = {_p->m_zNear,_p->m_zFar};
	// 显示鼠标的世界空间
	auto _mouse_pos = _p->m_mouse_pos_relative;
	auto _world_pos = ThirdParty::calcScreenToWorld(_mouse_pos, _p->m_window_size, _p->m_mat4_proj_inverse, _p->m_mat4_view_inverse);
	ImGui::Text("Mouse Point(Screen): %.2f,%.2f", _mouse_pos.x, _mouse_pos.y);
	ImGui::Text("Mouse Point(World) : %.2f,%.2f,%.2f", _world_pos.x, _world_pos.y, _world_pos.z);
	ImGui::DragFloat3("Position"	, glm::value_ptr(_p->m_vec3_pos));
	ImGui::DragFloat3("Tatget"		, glm::value_ptr(_p->m_vec3_target));
	ImGui::DragFloat("Distance"		, &_p->m_rotate_R);
	ImGui::DragFloat3("Right(X)"	, glm::value_ptr(_p->m_right));
	ImGui::DragFloat3("Up(Y)"		, glm::value_ptr(_p->m_up));
	ImGui::DragFloat3("Front(-Z)"	, glm::value_ptr(_p->m_front));
	ImGui::DragFloat("Speed"		, &_p->m_move_speed);
	if (ImGui::DragFloat2("ZNDC", _zndc, 1.0f, 0.0f)) {
		_p->m_zNear = _zndc[0];
		_p->m_zFar = _zndc[1];
	}
	ImGui::SeparatorText("Mouse Input");
	ImGui::Checkbox("Input", &_p->m_input);
	ImGui::RadioButton("Around", (int*)&_p->m_controller_type, 0); ImGui::SameLine();
	ImGui::RadioButton("FPS", (int*)&_p->m_controller_type, 1);
	ImGui::BeginDisabled(!p_property->m_input);
	ImGui::DragFloat2("Move", glm::value_ptr(_p->m_move));
	ImGui::DragFloat3("Rotate", glm::value_ptr(_p->m_axis_rotate));
	ImGui::EndDisabled();
	ImGui::SeparatorText("Euler");
	ThirdParty::imgui::help("Euler控制四元旋转轴");
	ImGui::DragFloat3("Rotate(World,Euler)", glm::value_ptr(_p->m_rotate_euler_world_degree));
	ImGui::DragFloat3("Rotate(Object,Euler)", glm::value_ptr(_p->m_rotate_euler_object_degree));
	ImGui::SeparatorText("Quaternion");
	ThirdParty::imgui::help("Quaternion控制物体旋转");
	ImGui::DragFloat4("Rotate(World,Quaternion)", glm::value_ptr(_p->m_rotate_quat_world));
	ImGui::DragFloat4("Rotate(Object,Quaternion)", glm::value_ptr(_p->m_rotate_quat_object));
	ImGui::SeparatorText("Static");
	ThirdParty::imgui::help("Camera 共享");
	ImGui::DragFloat("Depth Bias Constant", &CameraProperty::getDepthBiasConstant(), 0.001f);
	ImGui::SameLine();
	ThirdParty::imgui::help("深度偏移值常量");
	ImGui::DragFloat("Depth Bias Slop", &CameraProperty::getDepthBiasSlop(), 0.01f);
	ImGui::SameLine();
	ThirdParty::imgui::help("深度几何斜率偏移值");

}

ThirdParty::TextureEXT* Core::Resource::Camera::getCurrentTextureEXT(TEXTURE_TYPE _type)
{
	if (p_attachments.find(_type) != p_attachments.end()) {
		if (p_attachments[_type].size() == 1)
			return p_attachments[_type][0];
		else
			return p_attachments[_type][m_vkInfo.m_current_frame];

	}
	else
		return nullptr;
}

Core::Resource::Camera* Core::Resource::Camera::setClearValues(const std::vector<VkClearValue>& _values)
{
	t_clear_values = _values;
	return this;
}

void Core::Resource::Camera::onUpdate()
{
	if (m_is_main) onInput();
	p_camera_property->update();
}

void Core::Resource::Camera::onInput()
{
	if (!m_is_main) return;
	if (!m_focus) return;
	if (!p_camera_property->m_input) return;
	auto& _sdl = ThirdParty::sdl_manager;
	auto _move = ThirdParty::sdl_manager.getMouseMove();
	// Rotation around point
	// 绕点, 鼠标控制视角变化与FPS（默认）是相反的
	if (p_camera_property->m_controller_type == CameraProperty::ROTATION_AROUND_POINT) {
		if (_sdl.keyDown(SDL_SCANCODE_LALT)) {
			// ALT + LEFT 旋转
			if (_sdl.mouseDown(SDL_BUTTON_LEFT))
			{
				// 旋转轴增量
				p_camera_property->m_axis_rotate.x = -_move[1];	// X 为上下，鼠标的Y偏移
				p_camera_property->m_axis_rotate.y = -_move[0];	// Y 为左右，鼠标的X偏移
			}
		}
	}
	else if (p_camera_property->m_controller_type == CameraProperty::FIRST_PERSON_VIEW) {
		if (_sdl.mouseDown(SDL_BUTTON_LEFT))
		{
			// 旋转轴增量
			p_camera_property->m_axis_rotate.x = _move[1];	// X 为上下，鼠标的Y偏移
			p_camera_property->m_axis_rotate.y = _move[0];	// Y 为左右，鼠标的X偏移
		}
		if (_sdl.keyDown(SDL_SCANCODE_W))
			p_camera_property->m_move.z -= p_camera_property->m_move_speed;
		if (_sdl.keyDown(SDL_SCANCODE_S))
			p_camera_property->m_move.z += p_camera_property->m_move_speed;
		if (_sdl.keyDown(SDL_SCANCODE_A))
			p_camera_property->m_move.x -= p_camera_property->m_move_speed;
		if (_sdl.keyDown(SDL_SCANCODE_D))
			p_camera_property->m_move.x += p_camera_property->m_move_speed;
	}
	// 中键	控制Target
	if (_sdl.mouseDown(SDL_BUTTON_MIDDLE))
	{
		p_camera_property->m_move.x = _move[0];	// X 为左右，鼠标的Z偏移
		p_camera_property->m_move.y = _move[1];	// Y 为上下，鼠标的Y偏移
	}
	p_camera_property->m_wheel = ThirdParty::sdl_manager.getMouseWheelMove();

}

void Core::Resource::Camera::setPorjectType(LIGHT_TYPE _tpye)
{
	if (p_camera_property) {
		if (_tpye == PARALLEL)
			p_camera_property->m_project_type = CameraProperty::ORTHO;
		else if (_tpye == LIGHT_TYPE::POINT)
			p_camera_property->m_project_type = CameraProperty::PERSPECTIVE;
		else if (_tpye == LIGHT_TYPE::SPOT)
			p_camera_property->m_project_type = CameraProperty::PERSPECTIVE;
	}
}

void Core::Resource::Camera::initTemplate()
{
	
}
