#pragma once
#include <third_party_manager_global.h>
#include <Core/include/core_util_function.h>
#include <stack>
#include "include/base_editor.h"
#include "include/editor_resource.h"
// ����
namespace Editor {
	namespace OnRenderFunc {
		extern bool editor_flags[512];
		extern bool editor_drag_flags[512];
		extern Core::FileInfo* p_select_file_info;
		extern std::pair<ThirdParty::SlotData*, uint32_t> p_select_binding;
		extern Core::Resource::Scene* p_current_scene;
		enum EDITOR_FLAGS {
			CREATE_PROJ = 0x0000,	// �½���Ŀ
			CREATE_PROJ_2 = 0x0001,	// �½���Ŀ
			CREATE_PROJ_WINDOW = 0x0002,	// �½���Ŀ����
			CREATE_PROJ_WINDOW_2 = 0x0003,	// �½���Ŀ����
			UPDATA_FILE_LIST = 0x0004,		// �����б�
			OPEN_PROJ = 0x0005,				// ����Ŀ
			OPEN_PROJ_WINDOW = 0x0006,		// ����Ŀ����
			//-------------Begin����-----------------
			SCENE = 0x0010,
			PROPERTY = 0x0020,
			MATERIAL,
			FILE = 0x0030,
			MATERIAL_EDITOR = 0x0040,
			//----------------------------------------
			SELECT_WINDOW = 0x0100, // ѡ���ļ�
			SELECT_FILE,			// ѡ���ļ�
			SELECT_IMAGE,			// ѡ���ļ�
		};

		// �ļ�����,����ѡ���ļ�
		Core::FileInfo* file_window(bool isFile = false,const std::string& suffix = "");
		Core::FileInfo* file_window(const std::vector<std::string>& suffix);
		
		// �½��ļ���
		Core::FileInfo* create_a_new_folder(const char* folder_name);

		// �½���Ŀ step.1
		void editor_root_menu_bar_create_new_file();

		// �½���Ŀ step.2
		void editor_root_menu_bar_create_new_file_2();

		// ������Ŀ
		Core::FileInfo* create_project(const std::string& path,const std::string proj_name);

		// ͼƬѡ�񴰿�
		Core::FileInfo* editor_chooseImage();

		// �϶��ļ���ȡ, �����endchild()��
		std::string drag_drop_file(int _window_index, void* _data);

		// ������Ŀ�ļ�
		void save_project();

		// ������Ŀ
		void editor_root_menu_bar_open_project();

		// Dock ��
		void editor_root_func();
	
		// scene ����
		void editor_scene_func(Core::Resource::Scene* p_scene);

		// material ����
		void editor_material_func(Core::Resource::Scene* p_scene);
		// material ���Դ���
		void editor_material_property_func(Core::Resource::Material* p_mat);
		// model ���Դ���
		void editor_model_property_func(Core::Resource::Model* _obj);
		// mesh ���Դ���
		void editor_mesh_property_func(Core::Resource::Mesh* _obj);
		// ��ʾ���ͼ
		void editor_depth_property_func(Core::Resource::Camera* _render_camera);

		// ���Դ���
		void editor_test_func(Core::Resource::Scene* p_scene);

		// HELPС�ؼ�
		inline void help(const std::string& _path) { ThirdParty::imgui::help(_path); }
		// ͼ��С����
		bool icon(const std::string& _label, const std::string& _icon_name,const std::string& _tex_label = "");
		// ��iconһģһ����_label�������#
		bool icon_2(const std::string& _label, const std::string& _icon_name, uint32_t _sharp_count = 2);

		// check box ͼ��С����
		// _toptip �������ʹ��ʾ��Ϣ
		// ��-��ͼ��
		// ���ر�־
		bool icon_check_box(const std::string& _toptip, const std::vector<std::string>& _icon_names, bool& _value, const ImVec2& _icon_size = { 50,50 });
	}
}