#pragma once
#include "common/include/third_party_manager.h"
#include "core/info/include/global_info.h"
#include "imgui/imgui.h"
#include "imgui/imgui_impl_sdl2.h"
#include "imgui/imgui_impl_vulkan.h"
#include "imgui_utils.h"

namespace ThirdParty {
	class ENGINE_API_THIRDPARTY ImGuiManager
		:public ThirdParty::Manager<ImGuiManager>
	{
	public:
		ImGuiManager() = default;
		virtual ~ImGuiManager();
		static void ImGuiCheckVkResult(VkResult result);
	private:
		VkRenderPass m_renderpass = VK_NULL_HANDLE;
		std::vector<VkImageView> m_swapchain_image_views;
		std::vector<VkFramebuffer> m_swapchain_framebuffers;
		ImGui_ImplVulkan_InitInfo* p_init_info = nullptr;
		ImGuiIO* p_io;
		ImGuiStyle* p_style;
		ImDrawData* p_draw_data = nullptr;
		ImGuiContext* p_context = nullptr;
	private:
		bool m_bRenderStart = false;
		bool m_bRendering = false;
		bool m_bRenderEnd = true;
		ThirdParty::Core::ApplicationInfo& modeInfo = ThirdParty::Core::g_ApplicationInfo;
		ThirdParty::Core::ApplicationWindowInfo& appInfo = ThirdParty::Core::g_ApplicationWindowInfo;
		ThirdParty::Core::ApplicationVulkanInfo& vkInfo = ThirdParty::Core::g_ApplicationVulkanInfo;
		ThirdParty::Core::ApplicationSDLInfo& sdlInfo = ThirdParty::Core::g_ApplicationSDLInfo;
	public:
		//************************************
		// Method:    onRenderStart
		// FullName:  ThirdParty::ImGuiManager::onRenderStart
		// Access:    virtual public 
		// Returns:   void
		// Qualifier: 告诉imgui 新的一帧开始
		//************************************
		virtual void onRenderStart();

		//************************************
		// Method:    onRender
		// FullName:  ThirdParty::ImGuiManager::onRender
		// Access:    virtual public 
		// Returns:   void
		// Qualifier: 生成imgui渲染命令
		//************************************
		virtual void onRender();

		//************************************
		// Method:    onRenderEnd
		// FullName:  ThirdParty::ImGuiManager::onRenderEnd
		// Access:    virtual public 
		// Returns:   void
		// Qualifier: 告诉imgui 结束一帧渲染
		//************************************
		virtual void onRenderEnd();

		//************************************
		// Method:    render
		// FullName:  ThirdParty::ImGuiManager::render
		// Access:    virtual public 
		// Returns:   void
		// Qualifier: 渲染imgui内容
		// Parameter: VkCommandBuffer cmd
		//************************************
		virtual void render(VkCommandBuffer cmd);

		inline ImDrawData* getDrawData() { return this->p_draw_data; }
		inline ImGuiContext* getCurrentContext() { return this->p_context; }
		inline void poolEvent(SDL_Event* p_event) { ImGui_ImplSDL2_ProcessEvent(p_event); }
		inline VkRenderPass getRenderPass() { return m_renderpass; }

		ImGuiManager* init() override;
		ImGuiManager* getManager() override;
		void destroy() override;
		void windowResize() override;

	private:
		// 创建额外vk所需对象
		void createVulkanObject();
		// 清除额外vk所需对象
		void destroyVulkanObject();
	};

}

