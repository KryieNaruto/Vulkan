#include "editor_global_func.h"
#include <Resource/resource_global.h>
#include <Core/include/core_global.h>
#include <Scene/scene_include.h>
#undef max
namespace fs = std::filesystem;

bool Editor::OnRenderFunc::editor_flags[512];
bool Editor::OnRenderFunc::editor_drag_flags[512];
Core::FileInfo* Editor::OnRenderFunc::p_select_file_info = nullptr;
std::pair<ThirdParty::SlotData*, uint32_t> Editor::OnRenderFunc::p_select_binding;
Core::Resource::Scene* Editor::OnRenderFunc::p_current_scene = nullptr;

Core::FileInfo* Editor::OnRenderFunc::file_window(bool isFile,const std::string& suffix) {
	std::string& currentPath = BaseEditor::g_rootPath;

	bool& get_list = editor_flags[UPDATA_FILE_LIST];
	static std::vector<Core::FileInfo*> files;
	static Core::FileInfo* currentFile;
	static Core::FileInfo* chooseFile;
	static Core::FileInfo* createingFile;
	static std::stack<Core::FileInfo*> lastFiles;	// 上一步
	static std::stack<Core::FileInfo*> nextFiles;	// 下一步
	static int selection = -1;
	bool item_is_hover = false;
	if (ImGui::BeginChild("ConstrainedChild", ImVec2(200.0, 100.0f), ImGuiChildFlags_Border | ImGuiChildFlags_ResizeX | ImGuiChildFlags_ResizeY)) {
		{
			// 上一步---------------
			ImGui::BeginDisabled(lastFiles.empty());
			if (ImGui::Button("<-")) {
				if (!lastFiles.empty()) {
					Core::FileInfo* info = lastFiles.top();
					lastFiles.pop();
					// 进入文件夹，更新列表
					get_list = true;
					currentPath = info->m_absolutePath;
					nextFiles.push(currentFile);
					if (!currentFile) nextFiles.pop();
				}
			}
			ImGui::SameLine();
			ImGui::EndDisabled();
			// 下一步按钮---------------
			ImGui::BeginDisabled(nextFiles.empty());
			if (ImGui::Button("->")) {
				if (!nextFiles.empty()) {
					Core::FileInfo* info = nextFiles.top();
					nextFiles.pop();
					// 进入文件夹，更新列表
					get_list = true;
					currentPath = info->m_absolutePath;
					lastFiles.push(currentFile);
				}
			}
			ImGui::SameLine();
			ImGui::EndDisabled();
			// 上一文件夹---------------
			ImGui::BeginDisabled(currentPath.empty());
			if (ImGui::Button("^")) {
				// 进入文件夹，更新列表
				get_list = true;
				size_t found = currentFile->m_absolutePath.find_last_of("/\\");
				currentPath = (found == std::string::npos) ? currentFile->m_absolutePath : currentFile->m_absolutePath.substr(0, found);
				// 第一次读取到盘符
				if (currentPath.find_last_of("/\\") == std::string::npos) {
					currentPath += "\\";
				}
				// 第二次读取到盘符
				if (currentPath == currentFile->m_absolutePath) {
					currentPath = "";
				}
				lastFiles.push(currentFile);
				// 清空下一步
				while (!nextFiles.empty()) nextFiles.pop();
			}
			ImGui::SameLine();
			// 新建文件夹---------------
			if (ImGui::Button(Core::string_To_UTF8("新建文件夹").c_str())) {
				createingFile = new Core::FileInfo(); createingFile->m_rename = true;
			}
			ImGui::SameLine();
			ImGui::EndDisabled();
			// 取消---------------
			if (ImGui::Button(Core::string_To_UTF8("取消").c_str())) {
				ImGui::CloseCurrentPopup();
				files.clear(); files.shrink_to_fit();
				// 清空上一步
				while (!lastFiles.empty()) lastFiles.pop();
				// 清空下一步
				while (!nextFiles.empty()) nextFiles.pop();
				get_list = true;
			}
		}
		// 显示当前路径
		ImGui::Text("%s", Core::string_To_UTF8(currentPath).c_str());
		// 显示目录
		{
			// 获取文件信息
			if (get_list) {
				selection = -1;	// 消除所选
				files = Core::g_file_manager->getFileList(BaseEditor::g_rootPath, isFile, suffix);
				currentFile = files.empty() ? nullptr : files[0];
				currentPath = (currentFile == NULL) ? "" : currentFile->m_absolutePath;
				get_list = !get_list;
			}
			// 列表显示
			static ImGuiTableFlags flags = ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_RowBg | ImGuiTableFlags_Borders | ImGuiTableFlags_Resizable | ImGuiTableFlags_Reorderable | ImGuiTableFlags_Hideable;
			static int column_count = 4;
			if (ImGui::BeginTable("FileInfo", column_count, flags)) {
				{
					ImGui::TableSetupColumn(Core::string_To_UTF8("名称").c_str(), ImGuiTableColumnFlags_WidthFixed);
					ImGui::TableSetupColumn(Core::string_To_UTF8("修改日期").c_str(), ImGuiTableColumnFlags_WidthFixed);
					ImGui::TableSetupColumn(Core::string_To_UTF8("类型").c_str(), ImGuiTableColumnFlags_WidthFixed);
					ImGui::TableSetupColumn(Core::string_To_UTF8("大小").c_str(), ImGuiTableColumnFlags_WidthFixed);
					ImGui::TableHeadersRow();
				}
				for (size_t info_index = 0; info_index < files.size(); info_index++) {
					const auto& info = files[info_index];
					if (info->isFile() && isFile) continue; // 只显示文件夹
					if (info->m_absolutePath._Equal(currentFile == NULL ? "" : currentFile->m_absolutePath)) continue;	// 跳过当前文件夹
					ImGui::TableNextRow();
					{
						ImGui::TableSetColumnIndex(0);
						ImGuiSelectableFlags selectable_flags = ImGuiSelectableFlags_SpanAllColumns | ImGuiSelectableFlags_AllowOverlap;
						const bool item_is_selected = selection == info_index;

						// 名称
						const auto title = Core::string_To_UTF8(info->m_fileName);
						// 单击选中
						if (ImGui::Selectable(title.c_str(), item_is_selected, selectable_flags)) selection = info_index;
						// 双击进入文件夹
						const bool is_hover = ImGui::IsItemHovered();
						if (ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left) && is_hover)	// 双击
						{
							if (info->isFolder()) {
								// 进入文件夹，更新列表
								get_list = true;
								currentPath = info->m_absolutePath;
								// 压入堆栈
								lastFiles.push(currentFile);
								if (!currentFile) lastFiles.pop();
								// 清空下一步
								while (!nextFiles.empty()) nextFiles.pop();
							}
						}
						if (is_hover) item_is_hover |= is_hover;
					}
					// 日期
					{ImGui::TableSetColumnIndex(1); ImGui::Text("%s", info->m_date.c_str()); }
					// 类型
					{ImGui::TableSetColumnIndex(2); ImGui::Text("%s", Core::string_To_UTF8(info->getFileType()).c_str()); }
					// 大小
					{ImGui::TableSetColumnIndex(3); if (info->isFile()) ImGui::Text("%d", info->m_size); }
				}
				// 显示正在创建的文件夹
				if (createingFile) {
					if (createingFile->m_rename) {
						ImGui::TableNextRow();
						ImGui::TableSetColumnIndex(0);
						char buf[256] = { '\0' };
						if (ImGui::InputText("", buf, ImGuiInputTextFlags_EnterReturnsTrue)) {
							createingFile->m_fileName = buf;
						}
						if (!ImGui::IsItemActivated()) {
							if (ImGui::IsKeyDown(ImGuiKey_Enter) ||
								(ImGui::IsMouseClicked(ImGuiMouseButton_Left) && !ImGui::IsItemHovered())) {
								createingFile->m_rename = false;
							}
						}
					}
					// 创建文件夹
					else {
						if (!createingFile->m_fileName.empty() && !createingFile->m_fileName._Equal("")) {
							const auto& new_folder = create_a_new_folder(createingFile->m_fileName.c_str());
							if (new_folder)
								files.push_back(new_folder);
						}
						delete createingFile;
						createingFile = nullptr;
					}
				}
				ImGui::EndTable();
			}
		}
		// 单击空白,取消选择
		if (ImGui::IsMouseClicked(0) && ImGui::IsWindowHovered(0) && !item_is_hover)
			selection = -1;

		ImGui::EndChild();
	}
	if (selection >= 0)
		chooseFile = files[selection];	// 更新需要显示的路径
	else chooseFile = currentFile;
	return chooseFile;
}


Core::FileInfo* Editor::OnRenderFunc::file_window(const std::vector<std::string>& suffix /*= {}*/)
{
	std::string& currentPath = BaseEditor::g_rootPath;

	bool& get_list = editor_flags[UPDATA_FILE_LIST];
	static std::vector<Core::FileInfo*> files;
	static Core::FileInfo* currentFile;
	static Core::FileInfo* chooseFile;
	static Core::FileInfo* createingFile;
	static std::stack<Core::FileInfo*> lastFiles;	// 上一步
	static std::stack<Core::FileInfo*> nextFiles;	// 下一步
	static int selection = -1;
	bool item_is_hover = false;
	if (ImGui::BeginChild("ConstrainedChild", ImVec2(200.0, 100.0f), ImGuiChildFlags_Border | ImGuiChildFlags_ResizeX | ImGuiChildFlags_ResizeY)) {
		{
			// 上一步---------------
			ImGui::BeginDisabled(lastFiles.empty());
			if (ImGui::Button("<-")) {
				if (!lastFiles.empty()) {
					Core::FileInfo* info = lastFiles.top();
					lastFiles.pop();
					// 进入文件夹，更新列表
					get_list = true;
					currentPath = info->m_absolutePath;
					nextFiles.push(currentFile);
					if (!currentFile) nextFiles.pop();
				}
			}
			ImGui::SameLine();
			ImGui::EndDisabled();
			// 下一步按钮---------------
			ImGui::BeginDisabled(nextFiles.empty());
			if (ImGui::Button("->")) {
				if (!nextFiles.empty()) {
					Core::FileInfo* info = nextFiles.top();
					nextFiles.pop();
					// 进入文件夹，更新列表
					get_list = true;
					currentPath = info->m_absolutePath;
					lastFiles.push(currentFile);
				}
			}
			ImGui::SameLine();
			ImGui::EndDisabled();
			// 上一文件夹---------------
			ImGui::BeginDisabled(currentPath.empty());
			if (ImGui::Button("^")) {
				// 进入文件夹，更新列表
				get_list = true;
				size_t found = currentFile->m_absolutePath.find_last_of("/\\");
				currentPath = (found == std::string::npos) ? currentFile->m_absolutePath : currentFile->m_absolutePath.substr(0, found);
				// 第一次读取到盘符
				if (currentPath.find_last_of("/\\") == std::string::npos) {
					currentPath += "\\";
				}
				// 第二次读取到盘符
				if (currentPath == currentFile->m_absolutePath) {
					currentPath = "";
				}
				lastFiles.push(currentFile);
				// 清空下一步
				while (!nextFiles.empty()) nextFiles.pop();
			}
			ImGui::SameLine();
			// 新建文件夹---------------
			if (ImGui::Button(Core::string_To_UTF8("新建文件夹").c_str())) {
				createingFile = new Core::FileInfo(); createingFile->m_rename = true;
			}
			ImGui::SameLine();
			ImGui::EndDisabled();
			// 取消---------------
			if (ImGui::Button(Core::string_To_UTF8("取消").c_str())) {
				ImGui::CloseCurrentPopup();
				files.clear(); files.shrink_to_fit();
				// 清空上一步
				while (!lastFiles.empty()) lastFiles.pop();
				// 清空下一步
				while (!nextFiles.empty()) nextFiles.pop();
				get_list = true;
			}
		}
		// 显示当前路径
		ImGui::Text("%s", Core::string_To_UTF8(currentPath).c_str());
		// 显示目录
		{
			// 获取文件信息
			if (get_list) {
				selection = -1;	// 消除所选
				files = Core::g_file_manager->getFileList(BaseEditor::g_rootPath, true, suffix);
				currentFile = files.empty() ? nullptr : files[0];
				currentPath = (currentFile == NULL) ? "" : currentFile->m_absolutePath;
				get_list = !get_list;
			}
			// 列表显示
			static ImGuiTableFlags flags = ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_RowBg | ImGuiTableFlags_Borders | ImGuiTableFlags_Resizable | ImGuiTableFlags_Reorderable | ImGuiTableFlags_Hideable;
			static int column_count = 4;
			if (ImGui::BeginTable("FileInfo", column_count, flags)) {
				{
					ImGui::TableSetupColumn(Core::string_To_UTF8("名称").c_str(), ImGuiTableColumnFlags_WidthFixed);
					ImGui::TableSetupColumn(Core::string_To_UTF8("修改日期").c_str(), ImGuiTableColumnFlags_WidthFixed);
					ImGui::TableSetupColumn(Core::string_To_UTF8("类型").c_str(), ImGuiTableColumnFlags_WidthFixed);
					ImGui::TableSetupColumn(Core::string_To_UTF8("大小").c_str(), ImGuiTableColumnFlags_WidthFixed);
					ImGui::TableHeadersRow();
				}
				for (size_t info_index = 0; info_index < files.size(); info_index++) {
					const auto& info = files[info_index];
					if (info->isFile() && false) continue; // 只显示文件夹
					if (info->m_absolutePath._Equal(currentFile == NULL ? "" : currentFile->m_absolutePath)) continue;	// 跳过当前文件夹
					ImGui::TableNextRow();
					{
						ImGui::TableSetColumnIndex(0);
						ImGuiSelectableFlags selectable_flags = ImGuiSelectableFlags_SpanAllColumns | ImGuiSelectableFlags_AllowOverlap;
						const bool item_is_selected = selection == info_index;

						// 名称
						const auto title = Core::string_To_UTF8(info->m_fileName);
						// 单击选中
						if (ImGui::Selectable(title.c_str(), item_is_selected, selectable_flags)) selection = info_index;
						// 双击进入文件夹
						const bool is_hover = ImGui::IsItemHovered();
						if (ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left) && is_hover)	// 双击
						{
							if (info->isFolder()) {
								// 进入文件夹，更新列表
								get_list = true;
								currentPath = info->m_absolutePath;
								// 压入堆栈
								lastFiles.push(currentFile);
								if (!currentFile) lastFiles.pop();
								// 清空下一步
								while (!nextFiles.empty()) nextFiles.pop();
							}
							else {
								chooseFile = info;
								ImGui::CloseCurrentPopup();
							}
						}
						if (is_hover) item_is_hover |= is_hover;
					}
					// 日期
					{ImGui::TableSetColumnIndex(1); ImGui::Text("%s", info->m_date.c_str()); }
					// 类型
					{ImGui::TableSetColumnIndex(2); ImGui::Text("%s", Core::string_To_UTF8(info->getFileType()).c_str()); }
					// 大小
					{ImGui::TableSetColumnIndex(3); if (info->isFile()) ImGui::Text("%d", info->m_size); }
				}
				// 显示正在创建的文件夹
				if (createingFile) {
					if (createingFile->m_rename) {
						ImGui::TableNextRow();
						ImGui::TableSetColumnIndex(0);
						char buf[256] = { '\0' };
						if (ImGui::InputText("", buf, ImGuiInputTextFlags_EnterReturnsTrue)) {
							createingFile->m_fileName = buf;
						}
						if (!ImGui::IsItemActivated()) {
							if (ImGui::IsKeyDown(ImGuiKey_Enter) ||
								(ImGui::IsMouseClicked(ImGuiMouseButton_Left) && !ImGui::IsItemHovered())) {
								createingFile->m_rename = false;
							}
						}
					}
					// 创建文件夹
					else {
						if (!createingFile->m_fileName.empty() && !createingFile->m_fileName._Equal("")) {
							const auto& new_folder = create_a_new_folder(createingFile->m_fileName.c_str());
							if (new_folder)
								files.push_back(new_folder);
						}
						delete createingFile;
						createingFile = nullptr;
					}
				}
				ImGui::EndTable();
			}
		}
		// 单击空白,取消选择
		if (ImGui::IsMouseClicked(0) && ImGui::IsWindowHovered(0) && !item_is_hover)
			selection = -1;

		ImGui::EndChild();
	}
	if (selection >= 0)
		chooseFile = files[selection];	// 更新需要显示的路径
	else chooseFile = currentFile;
	return chooseFile;
}

Core::FileInfo* Editor::OnRenderFunc::create_a_new_folder(const char* folder_name)
{
	return Core::g_file_manager->createFolder(BaseEditor::g_rootPath, folder_name);
}

void Editor::OnRenderFunc::editor_root_menu_bar_create_new_file() {
	if (editor_flags[CREATE_PROJ]) {
		ImGui::OpenPopup(Core::string_To_UTF8("新建项目").c_str());
		editor_flags[CREATE_PROJ] = !editor_flags[CREATE_PROJ];
		editor_flags[CREATE_PROJ_WINDOW] = !editor_flags[CREATE_PROJ_WINDOW];
	}

	if (ImGui::BeginPopupModal(Core::string_To_UTF8("新建项目").c_str(), &editor_flags[CREATE_PROJ_WINDOW], ImGuiWindowFlags_AlwaysAutoResize)) {
		const auto& currentFile = file_window(true);
		const auto& currentPath = currentFile == nullptr ? "" : currentFile->m_absolutePath;
		// 显示路径
		ImGui::Text(Core::string_To_UTF8(currentPath).c_str()); ImGui::Separator();
		// 下一步
		if (ImGui::Button(Core::string_To_UTF8("下一步").c_str())) {
			editor_flags[CREATE_PROJ_2] = true;
			BaseEditor::g_rootPath = currentPath;
			ImGui::CloseCurrentPopup();
		};
		ImGui::EndPopup();
	}
}

void Editor::OnRenderFunc::editor_root_menu_bar_create_new_file_2() {
	if (editor_flags[CREATE_PROJ_2]) {
		ImGui::OpenPopup(Core::string_To_UTF8("配置新项目").c_str());
		editor_flags[CREATE_PROJ_2] = !editor_flags[CREATE_PROJ_2];
		editor_flags[CREATE_PROJ_WINDOW_2] = !editor_flags[CREATE_PROJ_WINDOW_2];
	}

	if (ImGui::BeginPopupModal(Core::string_To_UTF8("配置新项目").c_str(), &editor_flags[CREATE_PROJ_WINDOW_2], ImGuiWindowFlags_AlwaysAutoResize)) {
		static std::string proj_name = "";
		static char buf[256] = { '\0' };
		ImGui::Text(Core::string_To_UTF8("项目名称:").c_str()); ImGui::SameLine();
		ImGui::PushID("项目名称");
		if (ImGui::InputText("", buf, 256)) {
			proj_name = buf;
		}
		ImGui::PopID();
		auto root_path = BaseEditor::g_rootPath;
		root_path += "\\" + proj_name;
		ImGui::Text(Core::string_To_UTF8("项目路径:").c_str()); ImGui::SameLine(); ImGui::Text("%s", Core::string_To_UTF8(root_path).c_str());
		const bool is_exists = Core::g_file_manager->folder_is_exists(root_path.c_str());
		//==========================================
		// 上一步
		if (ImGui::Button(Core::string_To_UTF8("上一步").c_str())) {
			editor_flags[CREATE_PROJ] = true;
			ImGui::CloseCurrentPopup();
		}
		ImGui::SameLine();
		ImGui::BeginDisabled(is_exists);
		if (ImGui::Button(Core::string_To_UTF8("创建项目").c_str())) {
			create_project(root_path, proj_name);
			save_project();
			ImGui::CloseCurrentPopup();
		}
		ImGui::EndDisabled();
		if (is_exists) ImGui::Text(Core::string_To_UTF8("项目文件夹已存在").c_str());
		ImGui::EndPopup();
	}
}

Core::FileInfo* Editor::OnRenderFunc::create_project(const std::string& path, const std::string proj_name)
{
	const std::string root_path = path;
	// 创建项目文件夹
	BaseEditor::g_root_folder = Core::g_file_manager->createFolder(root_path);
	BaseEditor::g_rootPath = BaseEditor::g_root_folder->m_absolutePath;
	// 创建资源文件夹
	Core::g_file_manager->createFolder(root_path + "\\" + "resources");
	// 创建项目文件
	const auto file_path = BaseEditor::g_rootPath + "\\" + proj_name + ".zs";
	BaseEditor::g_root_file = Core::g_file_manager->createFile(file_path);
	// 设置项目信息
	if (Core::g_project_info) {
		// 保存旧信息
		Core::g_old_file_path = Core::g_project_info->m_file_path;
		delete Core::g_project_info;
	}
	Core::g_project_info = new Core::ProjectInfo(*Editor::BaseEditor::g_root_file);
	// 获取项目文件列表
	BaseEditor::m_file_lists = Core::g_file_manager->getFileList(BaseEditor::g_rootPath, false);
	// 打开新文件
	ThirdParty::Core::appInfo.m_opening = true;
	return BaseEditor::g_root_file;
}

void Editor::OnRenderFunc::save_project()
{
	ThirdParty::Core::appInfo.m_isSaved = false;
	ThirdParty::Core::appInfo.m_saving = true;
}

void Editor::OnRenderFunc::editor_root_menu_bar_open_project()
{
	if (editor_flags[OPEN_PROJ]) {
		ImGui::OpenPopup(Core::string_To_UTF8("打开项目").c_str());
		editor_flags[OPEN_PROJ] = !editor_flags[OPEN_PROJ];
		editor_flags[OPEN_PROJ_WINDOW] = !editor_flags[OPEN_PROJ_WINDOW];
	}

	if (ImGui::BeginPopupModal(Core::string_To_UTF8("打开项目").c_str(), &editor_flags[OPEN_PROJ_WINDOW], ImGuiWindowFlags_AlwaysAutoResize)) {
		const auto& currentFile = file_window(false, ".zs");
		const auto& currentPath = currentFile == nullptr ? "" : currentFile->m_absolutePath;
		// 显示路径
		ImGui::Text(Core::string_To_UTF8(currentPath).c_str()); ImGui::Separator();
		// 下一步
		if (currentFile)
			ImGui::BeginDisabled(!currentFile->isSuffix(".zs"));
		else
			ImGui::BeginDisabled(true);
		if (ImGui::Button(Core::string_To_UTF8("确定").c_str())) {
			// 打开新文件
			ThirdParty::Core::appInfo.m_opening = true;
			// 设定项目文件
			if (Core::g_project_info) {
				Core::g_old_file_path = Core::g_project_info->m_file_path;
				delete Core::g_project_info;
			}
			const auto& file_path = currentPath;
			// 打开文件并读取配置
			ThirdParty::jsoncpp_manager.addValueFromFile(file_path);
			Core::g_project_info = ThirdParty::jsoncpp_manager.getObjectFromFile<Core::ProjectInfo>(file_path);

			ImGui::CloseCurrentPopup();
		};
		ImGui::EndDisabled();
		ImGui::EndPopup();
	}
}

void Editor::OnRenderFunc::editor_root_func() {
	// 根节点样式设置
	ImGuiWindowFlags window_flags = 0;
	window_flags |= ImGuiWindowFlags_MenuBar;
	window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
	window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
	const ImGuiViewport* viewport = ImGui::GetMainViewport();
	ImGui::SetNextWindowPos(viewport->WorkPos);
	ImGui::SetNextWindowSize(viewport->WorkSize);
	ImGui::SetNextWindowViewport(viewport->ID);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
	// 绘制根Docking 节点
	if (ImGui::Begin("Docking Space", 0, window_flags)) {
		ImGui::DockSpace(ImGui::GetID("Editor_root"));
		ImGui::PopStyleVar(3);
	}
	// 绘制根节点menubar
	if (ImGui::BeginMenuBar()) {
		if (ImGui::BeginMenu(Core::string_To_UTF8("文件").c_str())) {
			if (ImGui::MenuItem(Core::string_To_UTF8("新建项目").c_str())) {
				editor_flags[CREATE_PROJ] = true;
				editor_flags[UPDATA_FILE_LIST] = true;
			}
			if (ImGui::MenuItem(Core::string_To_UTF8("打开项目").c_str())) {
				editor_flags[OPEN_PROJ] = true;
				editor_flags[UPDATA_FILE_LIST] = true;
			}
			ImGui::Separator();
			if (ImGui::MenuItem(Core::string_To_UTF8("保存项目").c_str())) {
				save_project();
			}

			ImGui::Separator();
			if (ImGui::MenuItem(Core::string_To_UTF8("退出").c_str())) {
				ThirdParty::Core::windowInfo.m_window_isClosed = true;
			}

			ImGui::EndMenu();
		}
		ImGui::EndMenuBar();
	}
	editor_root_menu_bar_create_new_file();
	editor_root_menu_bar_create_new_file_2();
	editor_root_menu_bar_open_project();
	editor_chooseImage();
	ImGui::End();
}

void Editor::OnRenderFunc::editor_scene_func(Core::Resource::Scene* p_scene) {
	if(p_current_scene == nullptr)
		p_current_scene = p_scene;
	// 世界物体导航
	if (ImGui::Begin("Scene Collection")) {
		if (ImGui::Button("+"))
			ImGui::OpenPopup("Add Scene Object");
		if (ImGui::BeginPopupContextItem("Add Scene Object")) {
			auto _target_pos = p_scene->getMainCamera()->getProperty()->m_vec3_target;
			ImGui::SeparatorText("Model");
			{
				if (ImGui::Selectable("Plane")) {
					p_scene->addShape(Core::Resource::SCENE_TYPE::PLANE	, _target_pos);
					ImGui::CloseCurrentPopup();
				}
				if (ImGui::Selectable("Cube")) {
					p_scene->addShape(Core::Resource::SCENE_TYPE::CUBE	, _target_pos);
					ImGui::CloseCurrentPopup();
				}
				if (ImGui::Selectable("Sphere")) {
					p_scene->addShape(Core::Resource::SCENE_TYPE::SPHERE, _target_pos);
					ImGui::CloseCurrentPopup();
				}
			}
			ImGui::SeparatorText("Light");
			{
				if (ImGui::Selectable("Ambient")) {
					p_scene->addAmbientLight(glm::vec3(1), 0.2f);
					ImGui::CloseCurrentPopup();
				}
				if (ImGui::Selectable("Point")) {
					p_scene->addPointLight(_target_pos, glm::vec4(1));
					ImGui::CloseCurrentPopup();
				}
				if (ImGui::Selectable("Direction")) {
					p_scene->addDirectionLight(glm::vec3(1), glm::vec4(1));
					ImGui::CloseCurrentPopup();
				}
				if (ImGui::Selectable("Spot")) {
					p_scene->addSpotLight(glm::vec3(1), glm::vec4(1));
					ImGui::CloseCurrentPopup();
				}
			}
			ImGui::EndPopup();
		}
		if(ImGui::BeginChild("Scene Collection Child")) {
			if (p_scene) {
				auto _objs = p_scene->getWorldSceneObjects();
				for (const auto& [_name, _obj] : _objs) {
					ImGui::PushID(_name.c_str());
					ImGui::Checkbox("Hidden", &_obj->getHidden()); ImGui::SameLine();

					// 如果是模型，则遍历出mesh供选择
					if (_obj->getSceneObejctType() == Core::Resource::SCENE_TYPE::MODEL) {
						// Node Select 样式
						auto _node_flag = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick | ImGuiTreeNodeFlags_SpanAvailWidth;
						_obj->setChoose(p_scene->getCurrentChooseSceneObject() == _obj);
						if (_obj->getChoose()) {
							_node_flag |= ImGuiTreeNodeFlags_Selected;
						}
						if (ImGui::TreeNodeEx(_obj->getSceneName().c_str(), _node_flag)) {
							for (const auto& [_mat_index,_meshes]: ((Core::Resource::Model*)_obj)->getMeshes()) {
								for (const auto& _mesh : _meshes) {
									_mesh->setChoose(p_scene->getCurrentChooseSceneObject() == _obj);
									if (ImGui::Selectable(_mesh->getSceneName().c_str(), &_mesh->getChoose())) {
										p_scene->chooseSceneObject(_mesh);
									}
								}
							}
							ImGui::TreePop();
						}
						if (ImGui::IsItemClicked())
							p_scene->chooseSceneObject(_obj);
					}
					// 否则提供一次Select
					else {
						_obj->setChoose(p_scene->getCurrentChooseSceneObject() == _obj);
						if (ImGui::Selectable(_obj->getSceneName().c_str(),&_obj->getChoose())) {
							p_scene->chooseSceneObject(_obj);
						}
					}
					ImGui::PopID();
				}
			}
		}
		ImGui::EndChild();
		drag_drop_file(SCENE, p_scene);

	}
	ImGui::End();
	// 物体属性导航
	if (ImGui::Begin("Property Editor")) {
		if (p_scene) {
			auto _obj = p_scene->getCurrentChooseSceneObject();
			if (_obj) {
				// 通用属性
				static char _name[32];
				memset(_name, '\0', 32);
				memcpy(_name, _obj->getSceneName().c_str(), _obj->getSceneName().length());
				if (ImGui::InputText("Name", _name, 32)) {
					_obj->setSceneName(_name);
				}
				_obj->propertyEditor();
				// 如果是material 
				if (_obj->getSceneObejctType() == Core::Resource::SCENE_TYPE::MATERIAL) {
					editor_material_property_func((Core::Resource::Material*)_obj);
				}
				else if (_obj->getSceneObejctType() == Core::Resource::SCENE_TYPE::MESH) {
					auto _obj_mesh = (Core::Resource::Mesh*)_obj;
					auto _obj_parent = (Core::Resource::Model*)_obj_mesh->getParent();
					editor_mesh_property_func(_obj_mesh);
					editor_material_property_func(_obj_parent->getMaterials()[_obj_mesh->getIndex()]);
				}
				else if ((_obj->getSceneObejctType() & Core::Resource::SCENE_TYPE::MODEL) == Core::Resource::SCENE_TYPE::MODEL) {
					editor_model_property_func((Core::Resource::Model*)_obj);
				}
				else if (_obj->getSceneObejctType() == Core::Resource::SCENE_TYPE::LIGHT) {
					auto _light = (Core::Resource::Light*)_obj;
					auto cam = (Core::Resource::Camera*)_light->getRenderCamera();
					// 设置Cmaera的投影矩阵
					cam->setPorjectType(_light->getLightType());
					// 输出 深度
					editor_depth_property_func(cam);
				}
				else if ((_obj->getSceneObejctType() & Core::Resource::SCENE_TYPE::CAMERA) == Core::Resource::SCENE_TYPE::CAMERA) {
					auto cam = (Core::Resource::Camera*)_obj;
					// 输出 深度
					editor_depth_property_func(cam);
				}
			}
		}
	}
	ImGui::End();

	// 所有Editor 编辑
	if (ImGui::Begin("SceneEditor")) {
		help("包含所有Scene的编辑面板");
		auto _root = p_scene->getRootScene()->getNext();
		while (_root)
		{
			if (ImGui::TreeNode((_root->getSceneName()+"##").c_str())) {
				// 天空盒
				ImGui::SeparatorText("Skybox");
				static const char* items[] = {
					"Cube",
					"Sphere",
				};
				static auto& _typeIndex = _root->getSkyboxIndex();
				//ImGui::Combo("Shape", (int*)& _typeIndex, items, IM_ARRAYSIZE(items));
				_root->propertyEditor();
				drag_drop_file(SCENE, _root);
				if (Core::Resource::Scene::p_hdri_textures.size() > 1) {
					auto _paths = Core::Resource::Scene::m_hdri_texture_paths;
					static auto _preview = _paths[0];
					if (ImGui::BeginCombo("Skybox Index", _preview.c_str())) {
						uint32_t _path_index = 0;
						for (auto _path : _paths) {
							if (ImGui::Selectable(_path.c_str())) 
								_root->setSkyboxIndex(_path_index);
							_path_index++;
						}
						ImGui::EndCombo();
					}
				}
				ImGui::TreePop();
			}
			_root = _root->getNext();
		}
	}
	ImGui::End();

	// Material 编辑器
	editor_material_func(p_scene);

	editor_test_func(p_scene);
}

std::string Editor::OnRenderFunc::drag_drop_file(int _window_index, void* _data) {
	// 拖动文件进行读取
	static std::string _file_path = "";
	static std::string _suffix = "";
	std::string _rel = "";
	if (ImGui::BeginDragDropTarget())
	{
		auto drop_target_flags = ImGuiDragDropFlags_AcceptBeforeDelivery | ImGuiDragDropFlags_AcceptNoPreviewTooltip;
		if (auto paylod = ImGui::AcceptDragDropPayload(DRAG_DROP_PAYLOAD_FILE, drop_target_flags))
		{
			editor_drag_flags[_window_index] = true;
			auto _file_info = (Core::FileInfo*)paylod->Data;	// 获取文件信息
			if (_file_info->isFolder()) {
				ImGui::SetMouseCursor(ImGuiMouseCursor_NotAllowed);
				ImGui::BeginTooltip();
				ImGui::Text(Core::string_To_UTF8("未识别文件类型").c_str());
				ImGui::EndTooltip();
			}
			// 读取文件
			else {
				_suffix = _file_info->m_suffixName;
				if(_suffix == SUFFIX_PROJECT_FILE)
					_file_path = _file_info->m_absolutePath;
				else if (SUFFIX_MODEL.contains(_suffix))
					_file_path = _file_info->m_absolutePath;
				else if(SUFFIX_IMAGE.contains(_suffix))
					_file_path = _file_info->m_absolutePath;
				else if(SUFFIX_SHADER.contains(_suffix))
					_file_path = _file_info->m_absolutePath;
				else {
					ImGui::BeginTooltip();
					ImGui::Text(Core::string_To_UTF8("未识别文件类型").c_str());
					ImGui::EndTooltip();
				}
			}
		}
		ImGui::EndDragDropTarget();
	}
	else if(editor_drag_flags[_window_index]) {
		editor_drag_flags[_window_index] = false;
	}

	if (editor_drag_flags[_window_index]&& ImGui::IsMouseReleased(0)) {
		editor_drag_flags[_window_index] = false;
		if (!_file_path.empty()) {
			// 项目文件
			if (_suffix == SUFFIX_PROJECT_FILE) {
				// 打开文件并读取配置
				ThirdParty::jsoncpp_manager.addValueFromFile(_file_path);
				Core::g_project_info = ThirdParty::jsoncpp_manager.getObjectFromFile<Core::ProjectInfo>(_file_path);
				_file_path = "";
			}
			// 模型
			if (SUFFIX_MODEL.contains(_suffix)) {
				if (_data) {
					auto _scene = (Core::Resource::Scene*)_data;
					_scene->loadModelFromFile(_file_path);
					_file_path = "";
				}
			}
			// binding 图片
			if (SUFFIX_IMAGE.contains(_suffix)) {
				if (_data == p_select_binding.first && _data) {
					auto _binding = (ThirdParty::SlotData*)_data;
					auto _loaded = ThirdParty::TextureEXT::getTextureEXT(_file_path);
					if (!_loaded) _loaded = new ThirdParty::TextureEXT(_file_path);
					_binding->bindData(_loaded, p_select_binding.second);
					p_select_binding.first = NULL;
				}
				else if (_data) {
					auto _scene = (Core::Resource::Scene*)_data;
					auto _loaded = ThirdParty::TextureEXT::getTextureEXT(_file_path);
					if (!_loaded) _loaded = new ThirdParty::TextureEXT(_file_path);
					_scene->addSkybox(_file_path, _loaded);
					_file_path = "";
				}
				_rel = _file_path;
			}
			// Spv / ShaderSource
			if (SUFFIX_SHADER.contains(_suffix)) {
				_rel = _file_path;
			}
		}
	}
	return _rel;
}

void Editor::OnRenderFunc::editor_test_func(Core::Resource::Scene* p_scene)
{
	if (ImGui::Begin(Core::string_To_UTF8("预加载Shader").c_str())) {
		for (const auto& [_name, _shader] : p_scene->getShaders()) {
			if (ImGui::TreeNode(_name.c_str())) {
				ImGui::PushID(_name.c_str());
				ImGui::Text(_name.c_str()); ImGui::SameLine();
				if (ImGui::Button("Reload")) {
					_shader->reload();
					p_scene->skipOnce();
					Core::Resource::Material::reloadMaterial(_shader);
				}
				// Vertex Input Attribute
				if (ImGui::TreeNode(Core::string_To_UTF8("顶点输入").c_str())) {
					auto _input = _shader->getVkVertexInputAttributeDescription();
					for (const auto& _v : _input) {
						ImGui::Text("Location: %d", _v.location);
						ImGui::Text("Binding: %d", _v.binding);
						ImGui::Text("Format: %s", ThirdParty::getFormatStr(_v.format).c_str());
						ImGui::Text("Offset: %d", _v.offset);
						ImGui::Separator();
					}
					ImGui::TreePop();
				}

				// DescriptorSetLayout
				if (ImGui::TreeNode(Core::string_To_UTF8("插槽").c_str())) {
					auto _slots = _shader->getSlots();
					for (const auto& _slot : _slots) {
						auto _node_title = "Set " + std::to_string(_slot->getSetID());
						if (ImGui::TreeNode(_node_title.c_str())) {
							// Slot
							for (const auto& [_name,_data]:_slot->getSlotDatas()) {
								ImGui::Text("Binding: %d", _data->getBinding());
								ImGui::Text("Name: %s", _data->getBindingName().c_str());
								ImGui::Text("DescriptorCount: %d", _data->getDescriptorCount());
								ImGui::Text("DescriptorType: %s", 
									ThirdParty::getDescriptorType(_data->getVkDEscriptorType()).c_str());
								ImGui::Text("Shader Stage: %s", 
									ThirdParty::getShaderStageStr((VkShaderStageFlagBits)_data->getVkShaderStage()).c_str());
								if (!_data->isTexture()) {
									auto& _struct_ite = _data->getMemberStruct();
									if (ImGui::TreeNode(_struct_ite.first.c_str())) {
										auto _struct_map = _struct_ite.second;
										for (const auto& [_name, _member] : _struct_map) {
											ImGui::Text("Name: %s", _name.c_str());
											ImGui::Text("Size: %d", _member.first);
											ImGui::Text("Offset: %d", _member.second);
										}
										ImGui::TreePop();
									}
								}
								ImGui::Separator();
							}
							ImGui::TreePop();
						}
					}
					ImGui::TreePop();
				}

				ImGui::PopID();
				ImGui::TreePop();
			}
		}
	}
	ImGui::End();
}

void Editor::OnRenderFunc::editor_material_func(Core::Resource::Scene* p_scene)
{
	if (ImGui::Begin("Material Collection")) {
		// 全局材质
		uint32_t _mat_index = 1;
		for (const auto& [_shader, _mat] : p_scene->getMaterials()) {
			if (icon(_mat->getSceneName(), ICON_MATERIAL)) {
				p_scene->chooseSceneObject(_mat);
			}
			// 计算是否换行
			auto _x_size = ImGui::GetItemRectSize().x;
			auto _x_window_size = ImGui::GetWindowSize().x;
			if (_x_size * ++_mat_index < _x_window_size)
				ImGui::SameLine();
			else _mat_index = 1;
		}
		// 模型材质
		for (const auto& [_shader, _mats] : p_scene->getModelMaterials()) {
			for (const auto& _mat : _mats) {
				if (icon(_mat->getSceneName(), ICON_MATERIAL)) {
					p_scene->chooseSceneObject(_mat);
				}
				// 计算是否换行
				auto _x_size = ImGui::GetItemRectSize().x;
				auto _x_window_size = ImGui::GetWindowSize().x;
				if (_x_size * _mat_index++ < _x_window_size)
					ImGui::SameLine();
				else _mat_index = 1;
			}
		}
	}
	ImGui::End();
}

bool Editor::OnRenderFunc::icon(const std::string& _label, const std::string& _icon_name, const std::string& _tex_label)
{
	static std::unordered_map<std::string, bool> m_ishover;
	static std::unordered_map<std::string, bool> m_isclick;
	static std::string m_current_click;
	bool _rel = false;
	// 图标大小
	ImVec2 m_icon_size = { 50,50 };
	ImVec2 m_child_size = m_icon_size;
	m_child_size.x += ImGui::GetTextLineHeight() + 4 * ImGui::GetStyle().FramePadding.y;
	m_child_size.y = m_child_size.x;
	auto _flag = ImGuiChildFlags_Border;
	auto _window_flag = ImGuiWindowFlags_NoScrollWithMouse|ImGuiWindowFlags_NoScrollbar;
	ImVec4 m_border_color = { 0,0,0,1 };
	if (m_ishover.find(_label) != m_ishover.end())
		if (m_ishover[_label])
			m_border_color = { 1,1,1,1 };
	if(m_current_click == _label)
		if (m_isclick[_label])
			m_border_color = { 1,1,1,1 };
	ImGui::PushStyleColor(ImGuiCol_Border, m_border_color);
	if (ImGui::BeginChild(_label.c_str(), m_child_size,_flag, _window_flag)) {
		auto _icon = Editor::getIcon(_icon_name);
		if (_icon != VK_NULL_HANDLE) {
			ImGui::Image(Editor::getIcon(_icon_name), m_icon_size);
			auto _text = _tex_label.empty() ? _label : _tex_label;
			ImGui::Text(_text.c_str());
		}
		else {
			ImGui::BeginChildFrame(ImGui::GetID("None"), m_icon_size);
			auto _text = _tex_label.empty() ? _label : _tex_label;
			ImGui::Text(_text.c_str());
			ImGui::EndChildFrame();
		}
		if (ImGui::IsWindowHovered()) {
			m_ishover[_label] = true;
			// 左键单击
			if (ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
				m_current_click = _label;
				m_isclick[m_current_click] = true;
				_rel = true;
			}
		}
		else {
			if (m_isclick.find(_label) != m_isclick.end()) {
				if (ImGui::IsMouseClicked(ImGuiMouseButton_Left) && m_isclick[_label]) {
					m_isclick[_label] = false;
				}
			}
			m_ishover[_label] = false;
		}
	}
	ImGui::EndChild();
	ImGui::PopStyleColor();
	return _rel;
}

bool  Editor::OnRenderFunc::icon_2(const std::string& _label, const std::string& _icon_name, uint32_t _sharp_count)
{
	auto _label_with_sharp = std::string(_sharp_count, '#')+_label;
	return icon(_label_with_sharp, _icon_name, _label);
}

void Editor::OnRenderFunc::editor_material_property_func(Core::Resource::Material* p_mat)
{
	// 图像大小
	ImVec2 m_icon_size = { 100,100 };
	static bool _update_set = true;

	auto _shader = p_mat->getShader();
	// 显示Shader信息
	if (_shader) {
		auto _name = _shader->getName();
		if (ImGui::CollapsingHeader(_name.c_str())) {
			ImGui::Text(_name.c_str()); ImGui::SameLine();
			ImGui::BeginDisabled(true);
			ImGui::Button("Reload");
			ImGui::EndDisabled();
			ImGui::SameLine();
			ThirdParty::imgui::help("重载Shader在'预加载Shader'面板");
			// Vertex Input Attribute
			if (ImGui::TreeNode(Core::string_To_UTF8("顶点输入").c_str())) {
				auto _input = _shader->getVkVertexInputAttributeDescription();
				for (const auto& _v : _input) {
					ImGui::Text("Location: %d", _v.location);
					ImGui::Text("Binding: %d", _v.binding);
					ImGui::Text("Format: %s", ThirdParty::getFormatStr(_v.format).c_str());
					ImGui::Text("Offset: %d", _v.offset);
					ImGui::Separator();
				}
				ImGui::TreePop();
			}

			// DescriptorSetLayout
			if (ImGui::TreeNode(Core::string_To_UTF8("插槽").c_str())) {
				auto _slots = _shader->getSlots();
				for (const auto& _slot : _slots) {
					auto _node_title = "Set " + std::to_string(_slot->getSetID());
					if (ImGui::TreeNode(_node_title.c_str())) {
						// Slot
						for (const auto& [_name, _data] : _slot->getSlotDatas()) {
							ImGui::Text("Binding: %d", _data->getBinding());
							ImGui::Text("Name: %s", _data->getBindingName().c_str());
							ImGui::Text("DescriptorCount: %d", _data->getDescriptorCount());
							ImGui::Text("DescriptorType: %s",
								ThirdParty::getDescriptorType(_data->getVkDEscriptorType()).c_str());
							ImGui::Text("Shader Stage: %s",
								ThirdParty::getShaderStageStr((VkShaderStageFlagBits)_data->getVkShaderStage()).c_str());
							if (!_data->isTexture()) {
								auto& _struct_ite = _data->getMemberStruct();
								if (ImGui::TreeNode(_struct_ite.first.c_str())) {
									auto _struct_map = _struct_ite.second;
									for (const auto& [_name, _member] : _struct_map) {
										ImGui::Text("Name: %s", _name.c_str());
										ImGui::Text("Size: %d", _member.first);
										ImGui::Text("Offset: %d", _member.second);
									}
									ImGui::TreePop();
								}
							}
							ImGui::Separator();
						}
						ImGui::TreePop();
					}
				}
				ImGui::TreePop();
			}
		}
	}
	// 显示实际插槽信息
	ThirdParty::SlotData*& _binding_select = p_select_binding.first;
	uint32_t& _binding_select_id = p_select_binding.second;
	for (const auto& [_set_index, _slot] : p_mat->getSlots()) {
		auto _title = "Set " + std::to_string(_set_index);
		static std::unordered_map<ThirdParty::SlotData*,std::unordered_map<std::string, std::vector<VkDescriptorSet>>> _tex_id;
		if (ImGui::CollapsingHeader(_title.c_str())) {
			auto _bindings = _slot->getSlotDatas();
			ImVec2 _next_pos = ImGui::GetCursorPos();
			uint32_t _tex_index = 0;
			for (const auto& [_name, _binding] : _bindings) {
				// 只显示纹理
				if (_binding->isTexture()) {
					ImGui::SetCursorPos(_next_pos);
					if (ImGui::CollapsingHeader(_name.c_str())) {
						if (ImGui::BeginPopupContextItem()) // <-- use last item id as popup id
						{
							if (ImGui::Button("Add a empty texture")) {
								// 添加空纹理到Set Binding
								p_mat->addEmptyTexture(_set_index, _name);
								ImGui::CloseCurrentPopup();
							}
							ImGui::EndPopup();
						}

						auto _tex_ite = _binding->getTextureEXT();
						// 处理插槽是Bindless的情况
						for (const auto& _tex : _tex_ite.second) {
							//  跳过深度Format
							auto _tex_format = _tex->getVkFormat();
							if (_tex_format != VK_FORMAT_R8G8B8A8_UNORM) continue;
							auto _tex_name = _tex->getPaths();
							// 传入图像
							ImGui::Text("Texture %d: %s", _tex_index, _name.c_str());
							ImVec2 _child_beign = ImGui::GetCursorPos();	// 获取下一控件输出位置
							// 如果是第一次显示，则创建VkDescriptor
							if (_tex_id.find(_binding) == _tex_id.end() || _update_set) {
								if (_tex_id[_binding].find(_tex_ite.first) == _tex_id[_binding].end()) {
									auto _views = _tex->getVkImageViews();
									if (_update_set) _tex_id.clear();
									for (const auto& _view : _views) {
										_tex_id[_binding][_tex_ite.first].push_back(ImGui_ImplVulkan_AddTexture(
											_tex->getVkSampler()==VK_NULL_HANDLE?Core::Resource::Material::getSampler(SAMPLER)->getVkSampler() :_tex->getVkSampler(),
											_view,
											_tex->getVkImageLayout()));
									}
									_update_set = false;
								}
							}
							// 如果binding被改变，则重新创建被改变的set
							if (_binding->isChange()) {
								auto _indices = _binding->getChangeIndex();
								for (const auto& _index : _indices) {
									auto _view = _tex->getVkImageViews()[_index];
									_tex_id[_binding][_tex_ite.first][_index] = ImGui_ImplVulkan_AddTexture(
										_tex->getVkSampler() == VK_NULL_HANDLE ? Core::Resource::Material::getSampler(SAMPLER)->getVkSampler() : _tex->getVkSampler(),
										_view,
										_tex->getVkImageLayout());
								}
							}
							// 处理插槽在Bindless的情况下，Texture是Array或者Cube
							uint32_t _id_index = 0;
							std::vector<glm::vec2> _pos = {
								{1,2},	// 右
								{1,0},	// 左
								{0,1},	// 上
								{2,1},	// 下
								{1,1},	// 前
								{1,3},	// 后
							};
							for (const auto& _id : _tex_id[_binding][_tex_ite.first]) {
								if (!_binding->isEmpty()) {
									ImVec2 _img_pos = { _pos[_id_index].y * m_icon_size.x, _pos[_id_index].x * m_icon_size.y };
									if (_binding->isCube()) {
										_img_pos.x += _child_beign.x;
										_img_pos.y += _child_beign.y;
										ImGui::SetCursorPos(_img_pos);
									}
									else {
										float _aspect = _binding->getTextureEXT().second[_id_index]->getAspect();
										m_icon_size.x *= 4;
										m_icon_size.y = m_icon_size.x / _aspect;
									}
									//------------------------------------
									// layer 排列
									// 0 1 2 3 4 5
									// 右左上下前后，前为Z轴
									// -- 2- -- --
									// 1- 4- 0- 5-
									// -- 3- -- --
									//------------------------------------
									ImGui::Image(_id, m_icon_size, { 0,0 }, { 1,1 }, { 1,1,1,1 }, { 1,1,1,1 });
									_next_pos.y = std::max(_img_pos.y + m_icon_size.y, _next_pos.y);
									if (ImGui::IsItemHovered()) {
										_binding_select = _binding;
										_binding_select_id = _id_index;
									}
									drag_drop_file(MATERIAL, _binding);
								}
								else {
									ImGui::BeginChildFrame(ImGui::GetID("Empty"), m_icon_size);
									if (ImGui::Button("Select")) {
										_binding_select = _binding;
										_binding_select_id = _id_index;
										editor_flags[SELECT_IMAGE] = true;
										editor_flags[UPDATA_FILE_LIST] = true;
									}
									else if (ImGui::IsWindowHovered()) {
										_binding_select = _binding;
										_binding_select_id = _id_index;
									}
									ImGui::EndChildFrame();
									drag_drop_file(MATERIAL, _binding);
								}
								_id_index++;
							}
							_tex_index++;
						}
					}
					auto _new_pos = ImGui::GetCursorPos();
					_next_pos.y = std::max(_new_pos.y, _next_pos.y);
				}
			}
		}
	}
	// 图片选择
	if (p_select_file_info) {
		if (p_select_file_info->isFile() && SUFFIX_IMAGE.contains(p_select_file_info->m_suffixName)) {
			if (_binding_select) {
				auto _img = ThirdParty::TextureEXT::getTextureEXT(p_select_file_info->m_absolutePath);
				if (!_img && _binding_select) {
					_img = new ThirdParty::TextureEXT(p_select_file_info->m_absolutePath);
				}
				_binding_select->bindData(_img, _binding_select_id);
				_binding_select->bindData(Core::Resource::Material::getSampler(SAMPLER)->getVkSampler(), _binding_select_id);
				_update_set = true;
			}

			_binding_select = nullptr;
			_binding_select_id = 0;
			p_select_file_info = nullptr;
		}
	}
}

void Editor::OnRenderFunc::editor_model_property_func(Core::Resource::Model* _obj)
{
	auto _mats = _obj->getMaterials();
	// 显示合并后的Mesh
	if (ImGui::CollapsingHeader("All meshes in material")) {
		for (const auto& [_mat_index, _meshes] : _obj->getMeshes()) {
			auto _mat = _mats[_mat_index];
			if (ImGui::TreeNode(_mat->getSceneName().c_str())) {
				for (const auto& _mesh : _meshes) {
					// 显示网格
					if (icon_2(_mesh->getSceneName(), ICON_MESH, 2)) {
						((Core::Resource::Scene*)_obj->getScene())->chooseSceneObject(_mesh);
					}
				}
				ImGui::TreePop();
			}
		}
	}
	// 所有材质
	if (ImGui::CollapsingHeader("All Materials")) {
		uint32_t _mat_index = 1;
		for (const auto& _mat : _mats) {
			if (icon_2(_mat->getSceneName(), ICON_MATERIAL, 2)) {
				((Core::Resource::Scene*)_obj->getScene())->chooseSceneObject(_mat);
			}
			// 计算是否换行
			// 计算是否换行
			auto _x_size = ImGui::GetItemRectSize().x;
			auto _x_window_size = ImGui::GetWindowSize().x;
			if (_x_size * _mat_index++ < _x_window_size)
				ImGui::SameLine();
			else _mat_index = 1;
		}
	}
}

void Editor::OnRenderFunc::editor_mesh_property_func(Core::Resource::Mesh* _obj) {
	// 显示材质
	_obj->propertyEditor();
}


Core::FileInfo* Editor::OnRenderFunc::editor_chooseImage()
{
	if (editor_flags[SELECT_IMAGE]) {
		ImGui::OpenPopup("Select Image");
		editor_flags[SELECT_IMAGE] = !editor_flags[SELECT_IMAGE];
		editor_flags[SELECT_WINDOW] = !editor_flags[SELECT_WINDOW];
	}

	if (ImGui::BeginPopupModal("Select Image", &editor_flags[SELECT_WINDOW], ImGuiWindowFlags_AlwaysAutoResize)) {
		p_select_file_info = file_window(SUFFIX_IMAGE_VEC);
		ImGui::EndPopup();
	}
	return p_select_file_info;
}


bool Editor::OnRenderFunc::icon_check_box(const std::string& _toptip, const std::vector<std::string>& _icon_names, bool& _value, const ImVec2& _icon_size)
{
	auto _type = 0;
	bool _rel = false;
	static VkDescriptorSet _hover_id = 0;
	ImVec2 m_icon_size = _icon_size;
	switch (_value)
	{
	case false:	// 关
		_type = 0;
		break;
	case true:	// 开
		_type = 1;
		break;
	default:
		break;
	}
	auto _icon = getIcon(_icon_names[_type]);
	ImVec4 _tine_col = { 0.7,0.7,0.7,1 };
	if (_icon == _hover_id)
		_tine_col = { 0,1,1,1 };
	ImGui::Image(_icon, m_icon_size,{0,0},{1,1},_tine_col);
	if (ImGui::IsItemHovered()) {
		_hover_id = _icon;
		if (ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
			_value = !_value;
			_rel = true;
		}
	}
	else _hover_id = 0;
	if (_toptip != "") {
		ImGui::SameLine();
		ThirdParty::imgui::help(_toptip);
	}
	return _rel;
}


void Editor::OnRenderFunc::editor_depth_property_func(Core::Resource::Camera* _render_camera)
{
	auto _depth = _render_camera->getCurrentTextureEXT(Core::Resource::TEXTURE_TYPE::COLOR_ATTACHMENT);
	static std::unordered_map<void*, VkDescriptorSet> _sets;
	if (!_sets.contains(_depth) && _depth) {
		auto _sampler = Core::Resource::RMaterial::getSampler();
		auto id = ImGui_ImplVulkan_AddTexture(_sampler->getVkSampler(), _depth->getVkImageView(), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
		_sets[_depth] = id;
	}
	if (ImGui::CollapsingHeader("Depth")) {
		if (_depth) {
			auto _set = _sets[_depth];
			auto _size = ImGui::GetWindowSize();
			ImVec2 _image_size = _size;
			_image_size.y = _image_size.x / _depth->getAspect();
			ImGui::Image(_set, _image_size);
		}
	}
}