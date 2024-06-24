#include "file_editor.h"
#include <filesystem>
#include <Core/include/core_util_function.h>
#include <algorithm>
#include <format>
#include <ThirdParty/imgui/imgui_impl_vulkan.h>
#include <editor_global_func.h>
namespace fs = std::filesystem;
using namespace Editor::OnRenderFunc;

Editor::FileEditor::FileEditor()
{
	this->m_editor_title = "Scene";
	this->m_show = true;
	this->m_line_file_count = 10;
	this->m_child_size = { 0,0 };
	this->m_icon_size = { 50,50 };
	this->m_hover_color = { 1,1,1,1 };
	this->m_stand_color = { 0.7,0.7,0.7,1 };
	this->m_disable_color = { 0.3,0.3,0.3,1 };
	this->m_child_size_small = { 50,50 };
	this->m_icon_size_small = { 30,30 };
	this->m_child_flag = ImGuiChildFlags_Border/* | ImGuiChildFlags_AutoResizeX*/ | ImGuiChildFlags_AutoResizeY | ImGuiChildFlags_AlwaysAutoResize;
	this->m_child_flag_without_border = ImGuiChildFlags_AutoResizeY | ImGuiChildFlags_AlwaysAutoResize;
	this->m_window_flag = ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse;
	preloadIcon();
	// 初始化根目录
	if (!g_root_folder) {
		auto _path = ThirdParty::Core::getModuleCurrentDirectory();
		g_root_folder = new Core::FileInfo(fs::directory_entry(_path));
		p_current_folder = g_root_folder;
		updateFileList();
	}
}

Editor::FileEditor::~FileEditor()
{
	this->m_show = false;
	if (g_root_folder)
		delete g_root_folder;
	for (const auto& _file : m_file_lists) {
		delete _file;
	}
	m_file_lists.clear();
}

void Editor::FileEditor::onRender()
{
	if (ImGui::Begin("File System",&editor_flags[EDITOR_FLAGS::FILE],ImGuiWindowFlags_NoScrollbar|ImGuiWindowFlags_NoScrollWithMouse)) {
		// 上一步
		if(ImGui::BeginChild("previous", m_child_size_small, m_child_flag_without_border, m_window_flag)) {
			ImVec4 _col = m_stand_color;
			if (ImGui::IsWindowHovered()) {
				_col = m_hover_color;
				// 单击
				if (ImGui::IsMouseClicked(0)) {
					if (!p_folder_previous.empty()) {
						// 保存当前文件夹到下一步
						p_folder_next.push_back(p_current_folder->m_absolutePath);
						// 设置当前路径
						auto _path = p_folder_previous[p_folder_previous.size() - 1];
						Core::FileInfo* _f = new Core::FileInfo(fs::directory_entry(_path));
						p_current_folder = _f;
						// 弹出下一步路径
						p_folder_previous.pop_back();
					}
				}
			}
			if (p_folder_previous.empty()) _col = m_disable_color;
			ImGui::Image(getIcon(BUTTON_PNG_PREVIOUS), m_icon_size_small, { 0,0 }, { 1,1 }, _col);
		}
		ImGui::EndChild();
		ImGui::SameLine();
		// 下一步
		if (ImGui::BeginChild("next", m_child_size_small, m_child_flag_without_border, m_window_flag)) {
			ImVec4 _col = m_stand_color;
			if (ImGui::IsWindowHovered()) {
				_col = m_hover_color;
				// 单击
				if (ImGui::IsMouseClicked(0)) {
					if (!p_folder_next.empty()) {
						// 保存当前文件夹到上一步
						p_folder_previous.push_back(p_current_folder->m_absolutePath);
						// 设置当前路径
						auto _path = p_folder_next[p_folder_next.size() - 1];
						Core::FileInfo* _f = new Core::FileInfo(fs::directory_entry(_path));
						p_current_folder = _f;
						// 弹出下一步路径
						p_folder_next.pop_back();
					}
				}
			}
			if (p_folder_next.empty()) _col = m_disable_color;
			ImGui::Image(getIcon(BUTTON_PNG_NEXT), m_icon_size_small, { 0,0 }, { 1,1 }, _col);
		}
		ImGui::EndChild(); 
		ImGui::SameLine();
		// 上一文件夹
		if (ImGui::BeginChild("previous_folder", m_child_size_small, m_child_flag_without_border, m_window_flag)) {
			ImVec4 _col = m_stand_color;
			if (ImGui::IsWindowHovered()) {
				_col = m_hover_color;
				// 单击
				if (ImGui::IsMouseClicked(0)) {
					// 清空下一步
					p_folder_next.clear();
					// 清空上一步
					p_folder_previous.clear();
					// 保存当前路径到上一步
					p_folder_previous.push_back(p_current_folder->m_absolutePath);
					// 进入父目录
					auto _path = p_current_folder->m_absolutePath;
					size_t found = p_current_folder->m_absolutePath.find_last_of("/\\");
					_path = (found == std::string::npos) ? p_current_folder->m_absolutePath : p_current_folder->m_absolutePath.substr(0, found);
					// 第一次读取到盘符
					if (_path.find_last_of("/\\") == std::string::npos) {
						_path += "\\";
					}
					// 第二次读取到盘符
					if (_path == p_current_folder->m_absolutePath) {
						_path = "";
					}
					auto _f = new Core::FileInfo(fs::directory_entry(_path));
					p_current_folder = _f;
				}
			}
			if (p_current_folder->m_absolutePath == "") _col = m_disable_color;
			ImGui::Image(getIcon(BUTTON_PNG_UP), m_icon_size_small, { 0,0 }, { 1,1 }, _col);
		}
		ImGui::EndChild(); ImGui::SameLine();

		// 当前路径显示
		ImGui::TextWrapped(Core::string_To_UTF8(p_current_folder->m_absolutePath).c_str());

		// 绘制图标
		auto _window_size = ImGui::GetWindowSize();
		auto _pre_cursor_pos = ImGui::GetCursorPos();
		auto _child_size = _window_size;
		_child_size.y -= _pre_cursor_pos.y;
		_child_size.y *= 0.8f;
		if (ImGui::BeginChild("File Icon List", _child_size)) {
			uint32_t _file_index = 0;
			bool _hover = false;
			for (size_t i = 0; i < m_file_lists.size(); i++) {
				auto _file = m_file_lists[i];
				// 跳过当前目录本目录
				if (_file->m_absolutePath == p_current_folder->m_absolutePath) {
					continue;
				}
				_hover |= renderFileIcon(_file, _file_index++);
			}
			// 如果没有指向
			if (!_hover) p_folder_hover = nullptr;
			if (p_folder_choose && !_hover) {
				if (ImGui::IsMouseClicked(0))
					p_folder_choose = nullptr;
			}
			// 设置图标大小 Ctrl+滚轮
			if ((ImGui::IsWindowHovered() || _hover) && ImGui::IsKeyPressed(ImGuiKey_LeftCtrl)) {
				auto _offset = ThirdParty::sdl_manager.getMouseWheelMove();
				m_line_file_count += _offset;
				if (m_line_file_count < 5) m_line_file_count = 5;
				if (m_line_file_count > 20) m_line_file_count = 20;
			}
		}
		ImGui::EndChild();
		m_window_size_last = m_window_size;
		m_window_size = ImGui::GetWindowSize();
		// 底部显示选中文件名
		if (p_folder_choose) {
			auto _height = ImGui::GetTextLineHeight();
			auto _pos = ImGui::GetCursorPos();
			_pos.x = ImGui::GetStyle().FramePadding.x;
			_pos.y = _window_size.y - 1.2 * _height;
			ImGui::SetCursorPos(_pos);
			ImGui::Selectable(p_folder_choose->m_fileName.c_str(), true);
		}
		// 第一次自适应设置每行数量
		static bool _first = true;
		auto _padding = ImGui::GetStyle().FramePadding.x;
		if (_first && (m_window_size.x == m_window_size_last.x && m_window_size.y == m_window_size_last.y)) {
			auto _window_size = ImGui::GetWindowSize();
			m_line_file_count = _window_size.x / (m_icon_size.x + _padding * 2);
			_first = false;
		}
		m_child_size.x = (m_window_size.x - m_line_file_count * _padding) / m_line_file_count;
		m_child_size.y = m_child_size.x + ImGui::GetTextLineHeight();
	}
	ImGui::End();
}

void Editor::FileEditor::onUpdate()
{
	// 判断是否打开新项目

	// 获取项目信息
	auto _proj = Core::g_project_info;
	// 打开了项目文件
	if (_proj) {
		if (g_root_folder)
		{
			// 如果是新项目
			if (_proj->m_root_path != g_root_folder->m_absolutePath)
			{
				// 清空栈
				p_folder_next.clear();
				p_folder_previous.clear();
				delete g_root_folder;
				g_root_folder = new Core::FileInfo(fs::directory_entry(_proj->m_root_path));
				p_current_folder = g_root_folder;
				updateFileList();
			}
		}
	}

	// 改变文件夹
	// 如果记录的当前路径值与p_current_folder 的绝对路径不一样，则重新获取file list
	if (m_current_path != p_current_folder->m_absolutePath) {
		updateFileList();
	}
}

void Editor::FileEditor::clearFileList()
{
	if (!m_file_lists.empty()) {
		for (const auto& _f : m_file_lists)
			delete _f;
		m_file_lists.clear();
	}
}

void Editor::FileEditor::updateFileList()
{
	clearFileList();
	m_file_lists = Core::g_file_manager->getFileList(p_current_folder->m_absolutePath);
	m_current_path = p_current_folder->m_absolutePath;
	// 对file_list 排序
	std::vector<Core::FileInfo*> temp_folder_list;
	std::vector<Core::FileInfo*> temp_file_list;
	for (const auto& _f : m_file_lists) {
		if (_f->isFolder())
			temp_folder_list.push_back(_f);
		else
			temp_file_list.push_back(_f);
	}

	static auto _cmp_func = [](Core::FileInfo* _a, Core::FileInfo* _b)->bool {
		std::string _lower_a = _a->m_fileName;
		std::transform(_lower_a.begin(), _lower_a.end(), _lower_a.begin(), (int(*)(int)) std::tolower);
		std::string _lower_b = _b->m_fileName;
		std::transform(_lower_b.begin(), _lower_b.end(), _lower_b.begin(), (int(*)(int)) std::tolower);
		return _lower_a < _lower_b;
		};

	std::sort(temp_folder_list.begin(), temp_folder_list.end(), _cmp_func);
	std::sort(temp_file_list.begin(), temp_file_list.end(), _cmp_func);

	m_file_lists.clear();
	for (const auto& _f : temp_folder_list)
		m_file_lists.push_back(_f);
	for (const auto& _f : temp_file_list)
		m_file_lists.push_back(_f);
}

bool Editor::FileEditor::renderFileIcon(Core::FileInfo* _file_info, uint32_t _file_index)
{
	static auto _same_line_cout = 1;
	bool _hover = false;
	// Image
	ImTextureID _id = 0;
	if (_file_info->isFolder()) {
		_id = (ImTextureID)getIcon(ICON_FOLDER);
	}
	else {
		auto _suf = _file_info->m_suffixName;
		if (SUFFIX_IMAGE.contains(_suf))
			_id = (ImTextureID)getIcon(ICON_FILE_IMAGE);
		else if (SUFFIX_MODEL.contains(_suf))
			_id = (ImTextureID)getIcon(ICON_FILE_MODEL);
		else if (SUFFIX_JSON.contains(_suf))
			_id = (ImTextureID)getIcon(ICON_FILE_JSON);
		else if (SUFFIX_TEXT.contains(_suf))
			_id = (ImTextureID)getIcon(ICON_FILE_TEXT);
		else if (SUFFIX_SHADER.contains(_suf))
			_id = (ImTextureID)getIcon(ICON_FILE_SHADER);
		else if (SUFFIX_BINARY.contains(_suf))
			_id = (ImTextureID)getIcon(ICON_FILE_BINARY);
		else
			_id = (ImTextureID)getIcon(ICON_FILE);
	}
	if (_id != 0) {
		{
			ImVec2 _rect_size = { 0,0 };
			ImVec2 _pos = { 0,0 };
			static auto& _style = ImGui::GetStyle();
			ImVec2 _size = { m_window_size.x / (m_line_file_count)-2 * _style.FramePadding.x - ImGui::GetStyle().ScrollbarSize,0 };
			_size.x *= 0.8;
			_size.y = _size.x;
			ImVec4 _border_color = m_stand_color;
			if (p_folder_hover == _file_info || p_folder_choose == _file_info) {
				_border_color = m_hover_color;
			}
			ImGui::PushStyleColor(ImGuiCol_Border, _border_color);
			if(ImGui::BeginChild(_file_info->m_fileName.c_str(), m_child_size, m_child_flag, m_window_flag))
			{
				if (ImGui::BeginDragDropSource()) {
					ImGui::SetDragDropPayload(DRAG_DROP_PAYLOAD_FILE, _file_info, sizeof(Core::FileInfo));
					ImGui::Image(_id, _size, { 0,0 }, { 1,1 }, _border_color);
					ImGui::EndDragDropSource();
				}
				ImGui::Image(_id, _size, { 0,0 }, { 1,1 }, _border_color);
				ImGui::TextWrapped("%s", Core::string_To_UTF8(_file_info->m_fileName).c_str());

				// 指向
				if (ImGui::IsWindowHovered()) {
					p_folder_hover = _file_info;
					_hover = true;
					// 单击文件夹
					if (ImGui::IsMouseClicked(0)) {
						p_folder_choose = _file_info;
					}
					// 双击文件夹
					if (ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left) && _file_info->isFolder()) {
						// 存入上一步
						p_folder_previous.push_back(p_current_folder->m_absolutePath);
						p_current_folder = new Core::FileInfo(*_file_info);
						// 清空下一步
						p_folder_next.clear();
						p_folder_choose = nullptr;
					}
				}
			}
			ImGui::EndChild();
			ImGui::PopStyleColor();
			auto _window_x = ImGui::GetWindowSize().x;
			auto _padding_x = ImGui::GetStyle().FramePadding.x;
			if (_file_index == 0) _same_line_cout = 1;
			if ((_file_index % (m_line_file_count)) * (m_child_size.x + _padding_x) < _window_x &&
				(_same_line_cout+1) * (m_child_size.x+ _padding_x) < _window_x) {
				ImGui::SameLine();
				_same_line_cout++;
			}
			else {
				_same_line_cout = 1;
			}
		}
	}
	return _hover;
}

void Editor::FileEditor::preloadIcon()
{
	auto _res_path = ThirdParty::Core::getModuleCurrentDirectory() + "\\resources\\img\\icon\\";
	std::vector<Core::FileInfo*> _list;
	_list.resize(0);
	_list = Core::g_file_manager->getFileList(_res_path);
	for (const auto& _img : _list) {
		if (_img->isFolder()) continue;;
		ThirdParty::TextureEXT* _tex = new ThirdParty::TextureEXT(_img->m_absolutePath);
		p_textures.push_back(_tex);
		auto _set = ImGui_ImplVulkan_AddTexture(
			Core::Resource::RMaterial::getSampler()->getVkSampler(), 
			_tex->getVkImageView(), 
			_tex->getVkImageLayout());
		p_icons[_img->m_fileName] = _set;
	}
}
