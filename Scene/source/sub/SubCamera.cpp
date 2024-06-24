#include "sub/SubCamera.h"

Core::Resource::SubCamera::SubCamera(const std::string& _name)
	:Camera(_name)
{
	m_scene_type = CAMERA_SUB;
	p_camera_property->m_zNear = 1.0f;
	p_camera_property->m_zFar = 15.0f;
}

Core::Resource::SubCamera::~SubCamera()
{

}

Core::Resource::SubCamera* Core::Resource::SubCamera::setRRenderPass(RRenderPass* _renderpass)
{
	if (_renderpass)
		p_render_pass = _renderpass->getpRenderPass();
	return this;
}

Core::Resource::SubCamera* Core::Resource::SubCamera::setCanvasSize(const VkExtent2D& _extent)
{
	p_camera_property->m_canvas_2d = _extent;
	p_camera_property->m_canvas_3d = { _extent.width,_extent.height ,1 };
	p_camera_property->update();
	return this;
}
Core::Resource::SubCamera* Core::Resource::SubCamera::setCanvasSize(const VkExtent3D& _extent) {
	p_camera_property->m_canvas_3d = _extent;
	p_camera_property->m_canvas_2d = { _extent.width,_extent.height };
	return this;
}

Core::Resource::SubCamera* Core::Resource::SubCamera::addAttachment(TEXTURE_TYPE _type, TextureEXT* _texture_ext)
{
	if (p_attachments.find(_type) == p_attachments.end()) {
		p_attachments[_type] = { _texture_ext };
	}
	else {
		p_attachments[_type].push_back(_texture_ext);
	}
	return this;
}

Core::Resource::SubCamera* Core::Resource::SubCamera::createFramebuffer(uint32_t _frame_count)
{
	auto _count = _frame_count;
	if (_count == 0)
		_count = m_vkInfo.m_frame_count;
	if (p_render_pass && !p_attachments.empty()) {
		for (uint32_t i = 0; i < _count; i++) {
			FrameBuffer* _fb = new FrameBuffer;
			std::vector<VkImageView> _views;
			for (const auto& [_name,_attachment] : p_attachments) {
				std::vector<VkImageView> _v;
				if (_attachment.size() == 1)
					_v = _attachment[0]->getVkImageViews();
				else
					_v = _attachment[i]->getVkImageViews();
				_views.insert(_views.end(), _v.begin(), _v.end());
			}
			uint32_t _layer = 1;
			// 输出为Cube
			if (_views.size() == 1) {
				if (p_attachments[COLOR_ATTACHMENT][0]->isCube())
					_layer = 6;
			}
			for (int _l = 0; _l < _layer; _l++)
				_fb->createFrameBuffer(
					p_camera_property->m_canvas_2d,
					p_render_pass->getVkRenderPass(),
					_views, 1);
			m_framebuffer_output.push_back(_fb);
		}
	}

	return this;
}

Core::Resource::SubCamera* Core::Resource::SubCamera::createCommandBuffer(uint32_t _frame_count)
{
	auto _count = _frame_count;
	if (_count == 0)
		_count = m_vkInfo.m_frame_count;

	for (uint32_t i = 0; i < _count; i++) {
		CommandBuffer* _cmd = new CommandBuffer(m_vkInfo.m_graphics_command_pool);
		m_command_buffer.push_back(_cmd);
	}

	return this;
}

Core::Resource::SubCamera* Core::Resource::SubCamera::endInitialize()
{
	m_initialized = true;
	m_render_initialized = true;
	return this;
}

Core::Resource::SubCamera* Core::Resource::SubCamera::setPosAndTarget(const glm::vec3& _pos, const glm::vec3& _target)
{
	p_camera_property->m_vec3_pos = _pos;
	p_camera_property->m_vec3_target = _target;
	// 计算上轴
	glm::vec3 _world_Y = glm::vec3(0, 1, 0);
	glm::vec3 _z = glm::normalize(_target - _pos);
	if (_world_Y.x == _z.x && _world_Y.y == _z.y && _world_Y.z == _z.z) _world_Y.z = 1.0f;
	glm::vec3 _x = glm::normalize(glm::cross(_world_Y, _z));
	glm::vec3 _y = glm::normalize(glm::cross(_x, _z));
	p_camera_property->m_mat4_view = glm::lookAt(_pos, _target, _y);
	return this;
}

Core::Resource::SubCamera* Core::Resource::SubCamera::setPosAndTarget(SceneObject* _target)
{
	p_target = _target;
	p_target_property = p_target->getSceneObjectProperty();
	p_camera_property->update(p_target->getSceneObjectProperty());
	return this;
}

Core::Resource::SubCamera* Core::Resource::SubCamera::setPosAndTarget(ObjectProperty* _target_property)
{
	p_camera_property->update(_target_property);
	return this;
}

void Core::Resource::SubCamera::onUpdate()
{
	if (p_target_property) {
		p_camera_property->update(p_target_property);
		if(!p_target_property->m_is_camera)
			p_target_property->m_mat4_view_proj = p_camera_property->m_mat4_view_proj;
	}
}

Core::Resource::SceneObject* Core::Resource::SubCamera::getSceneObjectTarget()
{
	return p_target;
}

Core::Resource::SubCamera* Core::Resource::SubCamera::setTempViewMatrix(const glm::mat4& _view)
{
	p_camera_property->m_mat4_view = _view;
	return this;
}

Core::Resource::SubCamera* Core::Resource::SubCamera::createCubeFramebuffer(uint32_t _frame_count, uint32_t _mip_map)
{
	auto _count = _frame_count;
	if (_count == 0)
		_count = m_vkInfo.m_frame_count;
	if (p_render_pass && !p_attachments.empty()) {
		std::vector<VkImageView> _views;
		for (const auto& [_name, _attachment] : p_attachments) {
			std::vector<VkImageView> _v;
			if (_attachment.size() == 1)
				_v = _attachment[0]->getVkImageViews();
			_views.insert(_views.end(), _v.begin(), _v.end());
		}
		// 输出为Cube
		for (uint32_t _mip = 0; _mip < _mip_map; _mip++) {
			auto _size = p_camera_property->m_canvas_2d;
			_size.width = _size.width * std::pow(0.5, _mip);
			_size.height = _size.height * std::pow(0.5, _mip);
			for (uint32_t _layer = 0; _layer < 6; _layer++) {
				FrameBuffer* _fb = new FrameBuffer;
				std::vector<VkImageView> _view = { _views[_mip * 6 + _layer] };
				_fb->createFrameBuffer(
					_size,
					p_render_pass->getVkRenderPass(),
					_view, 1);
				m_framebuffer_output.push_back(_fb);
			}
		}
	}
	

	return this;
}

Core::Resource::SubCamera* Core::Resource::SubCamera::setTempProjectionMatrix(const glm::mat4& _proj)
{
	p_camera_property->m_mat4_proj = _proj;
	return this;
}
