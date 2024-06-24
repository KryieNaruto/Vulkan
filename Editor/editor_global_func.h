#pragma once
#include <third_party_manager_global.h>
#include <Core/include/core_util_function.h>
#include <stack>
#include "include/base_editor.h"
#include "include/editor_resource.h"
// 函数
namespace Editor {
	namespace OnRenderFunc {
		extern bool editor_flags[512];
		extern bool editor_drag_flags[512];
		extern Core::FileInfo* p_select_file_info;
		extern std::pair<ThirdParty::SlotData*, uint32_t> p_select_binding;
		extern Core::Resource::Scene* p_current_scene;
		enum EDITOR_FLAGS {
			CREATE_PROJ = 0x0000,	// 新建项目
			CREATE_PROJ_2 = 0x0001,	// 新建项目
			CREATE_PROJ_WINDOW = 0x0002,	// 新建项目窗口
			CREATE_PROJ_WINDOW_2 = 0x0003,	// 新建项目窗口
			UPDATA_FILE_LIST = 0x0004,		// 更新列表
			OPEN_PROJ = 0x0005,				// 打开项目
			OPEN_PROJ_WINDOW = 0x0006,		// 打开项目窗口
			//-------------Begin窗口-----------------
			SCENE = 0x0010,
			PROPERTY = 0x0020,
			MATERIAL,
			FILE = 0x0030,
			MATERIAL_EDITOR = 0x0040,
			//----------------------------------------
			SELECT_WINDOW = 0x0100, // 选择文件
			SELECT_FILE,			// 选择文件
			SELECT_IMAGE,			// 选择文件
		};

		// 文件窗口,返回选中文件
		Core::FileInfo* file_window(bool isFile = false,const std::string& suffix = "");
		Core::FileInfo* file_window(const std::vector<std::string>& suffix);
		
		// 新建文件夹
		Core::FileInfo* create_a_new_folder(const char* folder_name);

		// 新建项目 step.1
		void editor_root_menu_bar_create_new_file();

		// 新建项目 step.2
		void editor_root_menu_bar_create_new_file_2();

		// 创建项目
		Core::FileInfo* create_project(const std::string& path,const std::string proj_name);

		// 图片选择窗口
		Core::FileInfo* editor_chooseImage();

		// 拖动文件读取, 添加在endchild()后
		std::string drag_drop_file(int _window_index, void* _data);

		// 保存项目文件
		void save_project();

		// 打开新项目
		void editor_root_menu_bar_open_project();

		// Dock 根
		void editor_root_func();
	
		// scene 窗口
		void editor_scene_func(Core::Resource::Scene* p_scene);

		// material 窗口
		void editor_material_func(Core::Resource::Scene* p_scene);
		// material 属性窗口
		void editor_material_property_func(Core::Resource::Material* p_mat);
		// model 属性窗口
		void editor_model_property_func(Core::Resource::Model* _obj);
		// mesh 属性窗口
		void editor_mesh_property_func(Core::Resource::Mesh* _obj);
		// 显示深度图
		void editor_depth_property_func(Core::Resource::Camera* _render_camera);

		// 测试窗口
		void editor_test_func(Core::Resource::Scene* p_scene);

		// HELP小控件
		inline void help(const std::string& _path) { ThirdParty::imgui::help(_path); }
		// 图标小工具
		bool icon(const std::string& _label, const std::string& _icon_name,const std::string& _tex_label = "");
		// 和icon一模一样，_label后会增加#
		bool icon_2(const std::string& _label, const std::string& _icon_name, uint32_t _sharp_count = 2);

		// check box 图标小工具
		// _toptip 鼠标悬浮使显示信息
		// 开-关图标
		// 开关标志
		bool icon_check_box(const std::string& _toptip, const std::vector<std::string>& _icon_names, bool& _value, const ImVec2& _icon_size = { 50,50 });
	}
}