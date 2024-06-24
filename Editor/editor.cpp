#include "editor.h"
#include <ThirdPartyManager/third_party_manager_global.h>
#include <Resource/resource_global.h>
#include <unordered_map>
#include <Core/include/core_util_function.h>
Editor::Editor::Editor()
{
	this->m_show = true;
	this->p_main_command_buffer = new ThirdParty::CommandBuffer("Editor CommandBuffer");
	this->p_graphics_queue = new ThirdParty::Queue(ThirdParty::Core::vkInfo.m_graphics_queue, "Editor Graphics Queue");
	this->p_graphics_pipeline = new ThirdParty::Pipeline("Editor Graphics Pipeline");

}

Editor::Editor::~Editor()
{
	this->m_show = false;
	if (p_main_command_buffer)
	{
		p_main_command_buffer->removeFromVkObjects();
		delete p_main_command_buffer;
	}
	if (p_graphics_queue) {
		p_graphics_queue->removeFromVkObjects();
		delete p_graphics_queue;
	}
	if (p_graphics_pipeline)
	{
		p_graphics_pipeline->removeFromVkObjects();
		delete p_graphics_pipeline;
	}
}

void Editor::Editor::onStart()
{
	static bool _init = false;
	if (_init) return;
	else _init = !_init;

}

void Editor::Editor::onRender()
{
	onStart();

	bool renderReady = true;
	while (!windowInfo.m_window_isClosed)
	{
		// 渲染前准备
		onRenderStart();
		
		// 事件处理，返回False则不继续向下执行
		if (!onEventProcess(renderReady)) 
		{
			vkInfo.newFrame();	
			continue;
		}

		if (renderReady) {
			// 获取交换链图像
			ThirdParty::vk_manager.getCurrentSwapchainImageIndex();
			
			// 渲染IMGUI
			onImGuiRender();

			// 渲染呈现
			onFrameRenderAndPresent();
		}
	}
	save();
}

void Editor::Editor::save()
{
	// 如果打开新文件，则保存旧文件
	if (ThirdParty::Core::appInfo.m_opening ) {
		if(Core::g_old_file_path != "") ThirdParty::jsoncpp_manager.save(Core::g_old_file_path);
		// 清理旧文件内存
		ThirdParty::jsoncpp_manager.clear();
		// 加载新文件到节点
		ThirdParty::jsoncpp_manager.addValueFromFile(Core::g_project_info->m_file_path);
		ThirdParty::Core::appInfo.m_opening = false;
	}
	if (ThirdParty::Core::appInfo.m_saving) {
		Core::save();
		ThirdParty::Core::appInfo.m_isSaved = true;
	}

	// 保存完毕后,写入文件
	if (ThirdParty::Core::appInfo.m_isSaved && ThirdParty::Core::appInfo.m_saving) {
		ThirdParty::jsoncpp_manager.save(Core::g_project_info->m_file_path);
		ThirdParty::Core::appInfo.m_saving = false;
	}
}

bool Editor::Editor::onEventProcess(bool& _renderReady)
{
	const auto& e = sdlInfo.p_sdlevent;
	auto& renderReady = _renderReady;
	// SDL Loop
	while (SDL_PollEvent(e))
	{
		ThirdParty::imgui_manager.poolEvent(e);
		switch (e->type)
		{
		case SDL_WINDOWEVENT:
			switch (e->window.event)
			{
			case SDL_WINDOWEVENT_MINIMIZED: windowInfo.m_window_isMinimized = true; break;
			case SDL_WINDOWEVENT_RESTORED: windowInfo.m_window_isMinimized = false; break;
			default:
				break;
			}
			break;
		case SDL_QUIT:
			windowInfo.m_window_isClosed = true;
			break;
		default:
			break;
		}
		// 监听键鼠
		ThirdParty::sdlManager.onStartListener();
	}
	// 更新SDLManager，以保证键鼠实时监测
	ThirdParty::sdlManager.onUpdate();
	// 最小化
	if (windowInfo.m_window_isMinimized) return false;
	// 窗口大小更改
	if (windowInfo.m_window_isResized) {
		ThirdParty::sdl_manager.windowResize();
		ThirdParty::vk_manager.windowResize();
		ThirdParty::imgui_manager.windowResize();
		ThirdParty::jsoncpp_manager.windowResize();
		if (ThirdParty::isReady()) {
			windowInfo.m_window_isResized = false;
			renderReady = true;
		}
		return false;
	}
	// 保存
	this->save();
	return renderReady;
}

void Editor::Editor::onRenderStart()
{
	//------------------------------
	// 测试
	static bool _init = false;
}

void Editor::Editor::onImGuiRender()
{
	// Graphics render[ImGui]
	ThirdParty::imgui_manager.onRenderStart();
	for (const auto& editor : Core::Core_Pool::p_all_editor) {
		ZEditor* p_editor = static_cast<ZEditor*>(editor.second);
		if (p_editor->isShow() && p_editor != this)
		{
			p_editor->onResize();
			p_editor->onInitialize();
			p_editor->onStart();
			p_editor->onUpdate();
			p_editor->onRender();
		}
	}
	ThirdParty::imgui_manager.onRender();
	ThirdParty::imgui_manager.onRenderEnd();
}

void Editor::Editor::onFrameRenderAndPresent()
{
	// 显示最终画面
	// 获取交换链图像索引
	uint32_t imageIndex = ThirdParty::Core::vkInfo.m_current_image;
	uint32_t frameIndex = ThirdParty::vk_manager.getCurrentFrameIndex();
	VkCommandBuffer& commandBuffer = vkInfo.m_main_command_buffers[imageIndex];
	//=====================
	// 记录指令
	auto result = p_main_command_buffer->beginCommandBuffer(commandBuffer);
	if (result == VK_SUCCESS) {
		// RenderPass 1 : IMGUI
		ThirdParty::imgui_manager.render(commandBuffer);
	}
	p_main_command_buffer->endCommandBuffer();
	// ===================== 提交 =====================
	// WAIT
	std::vector<VkSemaphore> waitSemaphore = { vkInfo.m_image_available_semaphores[vkInfo.m_current_frame] };
	std::vector<VkPipelineStageFlags> waitStages = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
	// SIGNAL
	std::vector<VkSemaphore> signalSemaphore = { vkInfo.m_render_finished_semaphores[vkInfo.m_current_frame] };
	//-------------------------------------------------
	(&vkInfo)->addCommandBuffer({ commandBuffer })
		->addWaitPipelineStage(waitStages)
		->addWaitSemaphores(waitSemaphore)
		->addSignalSemaphores(signalSemaphore);
	//-------------------------------------------------
	result = p_graphics_queue
		->setWaitSemaphores(vkInfo.m_submit_wait_semaphores)
		->setWaitPipelineStages(vkInfo.m_submit_wait_pipeline_stages)
		->setSignalSemaphores(vkInfo.m_submit_signal_semaphores)
		->setCommandBuffers(vkInfo.m_submit_command_buffers)
		->submit(vkInfo.m_fences[vkInfo.m_current_frame]);
	if (!ThirdParty::checkVkResult(result, "Failed to submit commandbuffer")) {
		return;
	}
	// ===================== 呈现 =====================
	result = p_graphics_queue->setWaitSemaphores(vkInfo.m_submit_signal_semaphores)->present();
}

