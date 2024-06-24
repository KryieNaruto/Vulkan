#include "RCamera.h"
#include <Core/include/core_global.h>
#include <ThirdParty/glm/gtx/transform.hpp>
#include <Core/include/core_util_function.h>
#include <RModel.h>
#include <RLight.h>

Core::Resource::RCamera::RCamera(const VkExtent3D& _target /* 目标画布大小 */, const std::string& _name)
{
	this->m_res_name = _name;
	this->m_json_title = "RCamera";
	this->p_id_generate->init(RESOURCE_TYPE::SHADER);
	this->m_uuid = this->p_id_generate->generateUUID();
	this->m_initialized = true;
	if(!p_property)
	{
		p_property = new CameraProperty; 
		p_camera_property = (CameraProperty*)p_property;
		p_camera_property->m_canvas_3d = _target; 
		p_camera_property->m_canvas_2d = { _target.width,_target.height };
	}
}

Core::Resource::RCamera::RCamera(const std::string& _name)
{
	this->m_res_name = _name;
	this->m_json_title = "RCamera";
	this->p_id_generate->init(RESOURCE_TYPE::SHADER);
	this->m_uuid = this->p_id_generate->generateUUID();
	this->m_initialized = true;
	if (!p_property)
	{
		p_property = new CameraProperty;
		p_camera_property = (CameraProperty*)p_property;
	}
	m_is_sub_camera = true;
}

Core::Resource::RCamera::~RCamera()
{
	if (p_camera_property) delete p_camera_property;
}

Json::Value Core::Resource::RCamera::serializeToJSON()
{
	Json::Value sub;
	sub["m_res_name"] = m_res_name;
	sub["m_json_title"] = m_json_title;
	return sub;
}

void Core::Resource::RCamera::deserializeToObj(Json::Value& _root)
{
	this->m_res_name = _root["m_res_name"].asString();
	this->m_json_title = _root["m_json_title"].asString();
}

void Core::Resource::RCamera::onUpdate()
{
	onInput();
	p_camera_property->update();
}

void Core::Resource::RCamera::onRender()
{
	if (isReady()) {
		
	}
}

void Core::Resource::RCamera::setup()
{
	memset(m_skip_first, true, 4);
	if (!m_is_sub_camera) {
		// 创建深度图
		p_depth_texture = new ThirdParty::TextureEXT(
			p_camera_property->m_canvas_3d,
			ThirdParty::findDepthFormat(),
			VK_IMAGE_TILING_OPTIMAL,
			VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
			VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
			m_vkInfo.m_sample_count
		);
		// 创建颜色附件，用于多重采样
		p_color_texture = new ThirdParty::TextureEXT(
			p_camera_property->m_canvas_3d,
			VK_FORMAT_R8G8B8A8_UNORM,
			VK_IMAGE_TILING_OPTIMAL,
			VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
			VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
			m_vkInfo.m_sample_count
		);

		// 初始化vkimage/vkimageview/framebuffer/fence
		for (size_t i = 0; i < m_vkInfo.m_frame_count; i++) {
			//------------------------------------
			ThirdParty::Image* _image = new ThirdParty::Image("Camera Image");
			ThirdParty::ImageView* _imageView = new ThirdParty::ImageView("Camera Image View");
			_image->createImage(
				VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
				p_camera_property->m_canvas_3d);
			_imageView->createImageView(_image->getVkImage());
			//-------------------------------------
			ThirdParty::Texture* _texture = new ThirdParty::Texture(_image->getVkImage(), _imageView->getVkImageView(), VK_NULL_HANDLE, "Camera Texture");
			//-------------------------------------
			p_render_pass = RRenderPass::getRRenderPass(RENDER_PASS_0)->getpRenderPass();
			//-------------------------------------
			ThirdParty::FrameBuffer* _frame = new ThirdParty::FrameBuffer("Camera Frame Buffer");
			_frame->createFrameBuffer(
				p_camera_property->m_canvas_2d,
				p_render_pass->getVkRenderPass(),
				{ p_color_texture->getVkImageView(),p_depth_texture->getVkImageView(),_imageView->getVkImageView() }
			);
			//-------------------------------------
			ThirdParty::CommandBuffer* _command = new ThirdParty::CommandBuffer(m_vkInfo.m_graphics_command_pool, "Camera Command Buffer");
			//-------------------------------------
			ThirdParty::Fence* _fence = new ThirdParty::Fence("Camera Fence");
			ThirdParty::Semaphore* _sema_wait = new ThirdParty::Semaphore("Camera Wait Fence");
			ThirdParty::Semaphore* _sema_sign = new ThirdParty::Semaphore("Camera Signal Fence");
			//-------------------------------------

			m_image_output.push_back(_image);
			m_image_view_output.push_back(_imageView);
			m_texture_output.push_back(_texture);
			m_framebuffer_output.push_back(_frame);
			m_command_buffer.push_back(_command);
			m_fences.push_back(_fence);
			m_wait_semaphores.push_back(_sema_wait);
			m_signal_semaphores.push_back(_sema_sign);
		}
		m_render_initialized = true;
	}
}

void Core::Resource::RCamera::onResize(const VkExtent3D& _canvas)
{
	if(m_render_initialized) {
		m_render_initialized = false;
		m_vkInfo.removeCommandBuffer(getVkCommandBuffer());
		memset(m_skip_first, true, 4);

		p_camera_property->m_canvas_3d = _canvas;
		p_camera_property->m_canvas_2d = { _canvas.width,_canvas.height };
		// 重建image/imageView/framebuffer/renderpass beginInfo
		vkDeviceWaitIdle(m_vkInfo.m_device);
		vkQueueWaitIdle(m_vkInfo.m_graphics_queue);
		p_depth_texture->recreate(p_camera_property->m_canvas_3d);
		p_color_texture->recreate(p_camera_property->m_canvas_3d);
		for (size_t i = 0; i < m_vkInfo.m_frame_count; i++) {
			m_image_output[i]->recreate(p_camera_property->m_canvas_3d);
			m_image_view_output[i]->recreate(m_image_output[i]->getVkImage());
			m_texture_output[i]->recreate(m_image_output[i]->getVkImage(), m_image_view_output[i]->getVkImageView());
			m_framebuffer_output[i]->recreate(p_camera_property->m_canvas_2d,
				{ p_color_texture->getVkImageView(),p_depth_texture->getVkImageView(),m_image_view_output[i]->getVkImageView() });

			m_command_buffer[i]->recreate();
			m_fences[i]->recreate();
			m_wait_semaphores[i]->recreate();
			m_signal_semaphores[i]->recreate();
		}
		m_canvas_is_resize = false;
		m_render_initialized = true;
	}
}

bool Core::Resource::RCamera::isFirst()
{
	bool rel = m_skip_first[m_vkInfo.m_current_image];
	m_skip_first[m_vkInfo.m_current_image] = false;
	return rel;
}

void Core::Resource::RCamera::onInput()
{
	if (!m_focus) return;
	ThirdParty::Core::ApplicationSDLInfo& m_sdl = ThirdParty::Core::g_ApplicationSDLInfo;
	ThirdParty::SDLManager& _m = ThirdParty::sdlManager;
	// 视图界面操作 [非指针锁定]
	auto _move = _m.getMouseMove();
	auto _wheel_move = _m.getMouseWheelMove();
	if (_m.keyDown(SDL_SCANCODE_LALT)) {
		// 指针旋转视角 左alt + 左键
		if (_m.mouseDown(SDL_BUTTON_LEFT)) {
			p_camera_property->m_axis_rotate.x += _move[1];
			p_camera_property->m_axis_rotate.y += _move[0];
		}
		// 缩放 左alt + 右键
		else if (_m.mouseDown(SDL_BUTTON_RIGHT)) {
		}
	}
	// 鼠标中健平移
	else if (_m.mouseDown(SDL_BUTTON_MIDDLE)) {
		p_camera_property->m_move.x = _move[0];
		p_camera_property->m_move.y = _move[1];
	}
	// 滑轮
	else if (_m.mouseWheel()) {
		p_camera_property->m_move.z = _wheel_move;
	}

}

void Core::Resource::RCamera::skipOnceCommandBuffer()
{
	m_vkInfo.removeCommandBuffer(getVkCommandBuffer());
}

void Core::Resource::RCamera::repaint()
{
	skipOnceCommandBuffer();
	onRender();
}

void Core::Resource::RCamera::onRenderStart()
{
	if (t_clear_values.empty())
		t_clear_values = { p_camera_property->m_canvas_bg,p_camera_property->m_canvas_dp,p_camera_property->m_canvas_bg };
	if (isReady() && p_render_pass) {
		const auto& _index = m_vkInfo.m_current_image;
		const auto& cmd = m_command_buffer[_index];
		cmd->beginCommandBuffer();
		auto _cmd = cmd->getVkCommandBuffer();
		p_render_pass
			->setClearValues(t_clear_values)
			->setFrameBuffer(p_temp_framebuffer_output == nullptr ? m_framebuffer_output[_index]->getVkFramebuffer() : p_temp_framebuffer_output->getVkFramebuffer())
			->setRenderArea({ {0,0},p_camera_property->m_canvas_2d })
			->beginRenderPass(_cmd);
		p_temp_framebuffer_output = nullptr;
		// 设定viewport
		VkViewport _viewport;
		_viewport.width = p_camera_property->m_canvas_2d.width;
		_viewport.height = p_camera_property->m_canvas_2d.height;
		_viewport.x = 0;
		_viewport.y = 0;
		_viewport.minDepth = 0.0;
		_viewport.maxDepth = 1.0;
		vkCmdSetViewport(_cmd, 0, 1, &_viewport);
		// 设定scissor
		VkRect2D _scissor;
		_scissor.offset = { 0,0 };
		_scissor.extent = p_camera_property->m_canvas_2d;
		vkCmdSetScissor(_cmd, 0, 1, &_scissor);
		// 设定深度偏移
		vkCmdSetDepthBias(_cmd, p_camera_property->m_depthBiasConstant, 0, p_camera_property->m_depthBiasSlope);
	}
}

void Core::Resource::RCamera::onRenderEnd()
{
	if (isReady() && p_render_pass) {
		const auto& _index = m_vkInfo.m_current_image;
		const auto& cmd = m_command_buffer[_index];
		auto _cmd = cmd->getVkCommandBuffer();

		p_render_pass->endRenderPass();
		m_command_buffer[_index]->endCommandBuffer();

		m_vkInfo.addCommandBuffer({ _cmd });
	}
}

Core::Resource::RCamera* Core::Resource::RCamera::setClearValues(const std::vector<VkClearValue>& _value)
{
	t_clear_values.clear();
	t_clear_values = _value;
	return this;
}

VkCommandBuffer Core::Resource::RCamera::onRenderStartImmediately()
{
	if (t_clear_values.empty())
		t_clear_values = { p_camera_property->m_canvas_bg,p_camera_property->m_canvas_dp,p_camera_property->m_canvas_bg };
	if (isReady() && p_render_pass) {
		const auto& _index = m_vkInfo.m_current_image;
		const auto& _cmd = ThirdParty::vkManager.BeginNewSigleCommandBuffer();
		VkRect2D _rect = { {0,0},p_camera_property->m_canvas_2d };
		p_render_pass
			->setClearValues(t_clear_values)
			->setFrameBuffer(p_temp_framebuffer_output == nullptr ? m_framebuffer_output[_index]->getVkFramebuffer() : p_temp_framebuffer_output->getVkFramebuffer())
			->setRenderArea(m_temp_render_area_bool ? m_temp_render_area : _rect)
			->beginRenderPass(_cmd);
		p_temp_framebuffer_output = nullptr;
		m_temp_render_area_bool = false;
		// 设定viewport
		VkViewport _viewport;
		_viewport.width = p_camera_property->m_canvas_2d.width;
		_viewport.height = p_camera_property->m_canvas_2d.height;
		_viewport.x = 0;
		_viewport.y = 0;
		_viewport.minDepth = 0.0;
		_viewport.maxDepth = 1.0;
		vkCmdSetViewport(_cmd, 0, 1, &_viewport);
		// 设定scissor
		VkRect2D _scissor;
		_scissor.offset = { 0,0 };
		_scissor.extent = p_camera_property->m_canvas_2d;
		vkCmdSetScissor(_cmd, 0, 1, &_scissor);
		// 设定深度偏移
		vkCmdSetDepthBias(_cmd, p_camera_property->m_depthBiasConstant, 0, p_camera_property->m_depthBiasSlope);
		return _cmd;
	}
	return VK_NULL_HANDLE;
}

void Core::Resource::RCamera::onRenderEndImmediately(VkCommandBuffer _cmd)
{
	if (isReady() && p_render_pass) {
		const auto& _index = m_vkInfo.m_current_image;

		p_render_pass->endRenderPass();
		ThirdParty::vkManager.endSingleCommandBuffer(_cmd);
	}
}
