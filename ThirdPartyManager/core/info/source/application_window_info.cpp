#include "application_window_info.h"
#include <ThirdParty/SDL2/SDL_video.h>
#include <ThirdParty/SDL2/SDL.h>
using namespace ThirdParty;
Core::ApplicationWindowInfo::ApplicationWindowInfo(const char* title, int flags)
{
	this->m_title = title;
	SDL_Rect rect;
	if (SDL_GetDisplayUsableBounds(0, &rect) == 0) {
		this->m_window_width = rect.w / 2;
		this->m_window_height = rect.h / 2;
		this->m_window_aspect = (float)rect.w / (float)rect.h;
	}
	else {
		this->m_window_width = 0;
		this->m_window_height = 0;
		this->m_window_aspect = 0;
	}
	this->m_window_sdl_flags = flags;
	this->m_window_isClosed = false;
	this->m_window_isResized = false;
	this->m_window_isMinimized = false;
}

Core::ApplicationWindowInfo Core::ApplicationWindowInfo::updateWindowInfo(SDL_Window* p_window)
{
	if (p_window) {
		SDL_GetWindowSize(p_window, (int*)&this->m_window_width, (int*)&this->m_window_height);
		this->m_window_aspect = (float)this->m_window_width / (float)this->m_window_height;
	}
	else {
		SDL_Rect rect;
		if (SDL_GetDisplayUsableBounds(0, &rect) == 0) {
			this->m_window_width = rect.w / 2;
			this->m_window_height = rect.h / 2;
			this->m_window_aspect = (float)rect.w / (float)rect.h;
		}
		else {
			this->m_window_width = 0;
			this->m_window_height = 0;
			this->m_window_aspect = 0;
		}
	}
	return *this;
}

void Core::ApplicationVulkanInfo::nextFrame()
{
	m_submit_command_buffers.clear();
	m_submit_wait_pipeline_stages.clear();
	m_submit_wait_semaphores.clear();
	m_submit_signal_semaphores.clear();
	this->m_current_frame = (this->m_current_frame + 1) % this->m_frame_count;
}

ThirdParty::Core::ApplicationVulkanInfo* Core::ApplicationVulkanInfo::addCommandBuffer(const std::vector<VkCommandBuffer>& _v)
{
	m_submit_command_buffers.insert(m_submit_command_buffers.end(), _v.begin(), _v.end());
	return this;
}

ThirdParty::Core::ApplicationVulkanInfo* Core::ApplicationVulkanInfo::addWaitPipelineStage(const std::vector<VkPipelineStageFlags>& _v)
{
	m_submit_wait_pipeline_stages.insert(m_submit_wait_pipeline_stages.end(), _v.begin(), _v.end());
	return this;
}

ThirdParty::Core::ApplicationVulkanInfo* Core::ApplicationVulkanInfo::addWaitSemaphores(const std::vector<VkSemaphore>& _v)
{
	m_submit_wait_semaphores.insert(m_submit_wait_semaphores.end(), _v.begin(), _v.end());
	return this;
}

ThirdParty::Core::ApplicationVulkanInfo* Core::ApplicationVulkanInfo::addSignalSemaphores(const std::vector<VkSemaphore>& _v)
{
	m_submit_signal_semaphores.insert(m_submit_signal_semaphores.end(), _v.begin(), _v.end());
	return this;
}

ThirdParty::Core::ApplicationVulkanInfo* Core::ApplicationVulkanInfo::removeCommandBuffer(const VkCommandBuffer& _v)
{
	for (auto ite = m_submit_command_buffers.begin(); ite < m_submit_command_buffers.end(); ite++) {
		if (*ite == _v) {
			m_submit_command_buffers.erase(ite);
			return this;
		}
	}
	return this;
}

void Core::ApplicationVulkanInfo::newFrame()
{
	m_submit_command_buffers.clear();
	m_submit_wait_pipeline_stages.clear();
	m_submit_wait_semaphores.clear();
	m_submit_signal_semaphores.clear();
}

Core::ApplicationInfo::ApplicationInfo(bool _debug /*= false*/, bool _multiView /*= false*/)
	:m_debug(_debug), m_multiView(_multiView) 
{
};