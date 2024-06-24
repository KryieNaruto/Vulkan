#pragma once

#include <string>
#include <Core/include/core_global.h>
#include <Scene/scene_include.h>
#include <ThirdPartyManager/third_party_manager_global.h>
#include <ThirdPartyManager/common/include/_debug_func.h>
#include "editor_define.h"

namespace Editor {

	class ENGINE_API_EDITOR BaseEditor: protected Core::ZClass
	{
	public:
		BaseEditor();
		virtual ~BaseEditor();
		virtual void onInitialize();
		virtual void onStart();
		virtual void onRender() = 0;
		virtual void onResize();
		virtual void onUpdate();
		static BaseEditor* getEditor(uint64_t uuid);
		inline bool isShow() { return this->m_show; }
		// 基于根目录
		static std::string g_rootPath;
		// 根目录信息
		static Core::FileInfo* g_root_folder;
		// 项目文件
		static Core::FileInfo* g_root_file;
		// 当前目录文件列表
		static std::vector<Core::FileInfo*> m_file_lists;
	protected:
		ThirdParty::Core::ApplicationWindowInfo& windowInfo = ThirdParty::Core::windowInfo;
		ThirdParty::Core::ApplicationVulkanInfo& vkInfo = ThirdParty::Core::vkInfo;
		ThirdParty::Core::ApplicationSDLInfo& sdlInfo = ThirdParty::Core::sdlInfo;
		// 样式设定
		ImGuiWindowFlags window_flags = ImGuiWindowFlags_None;
		std::string m_editor_title;
		bool m_show = false;
		// 窗口大小
		ImVec2 m_window_size = { 0,0 };
		ImVec2 m_window_size_last = { -1,-1 };
	private:

	};

	class ENGINE_API_EDITOR ZEditor : public BaseEditor {
	public:
		ZEditor() = default;
		inline ZEditor(bool show);
		inline ZEditor(const char* title, bool show);
		virtual ~ZEditor();
		virtual void onRender() override;
		virtual void onUpdate() override;
		inline void setOnRenderFunc(void (*p_func)()) { this->p_onRender_func = p_func; }
	private:
		void (*p_onRender_func)() = nullptr;
	protected:
	};

	ENGINE_API_EDITOR void destroy();
}

