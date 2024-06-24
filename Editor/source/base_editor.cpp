#include "base_editor.h"
#include <core/common/include/debug_to_console.h>

Editor::BaseEditor::BaseEditor()
{
	p_id_generate = new Core::IDGenerate();
	p_id_generate->init(Core::RESOURCE_TYPE::EDITOR);
	this->m_uuid = p_id_generate->generateUUID();
	// 初始化imgui context
	ImGui::SetCurrentContext(ThirdParty::imgui_manager.getCurrentContext());

	// 存入对象池
	Core::Core_Pool::addResource(m_uuid, this);
	Core::Core_Pool::p_all_editor.insert(std::make_pair(this->m_uuid, this));
}

Editor::BaseEditor::~BaseEditor()
{
	if (g_root_folder) {
		delete g_root_folder;
		g_root_folder = nullptr;
	}
	if (g_root_file) {
		delete g_root_file;
		g_root_file = nullptr;
	}
	for (const auto& file : m_file_lists)
		delete file;
	m_file_lists.clear();
	if (this->p_id_generate)
		delete p_id_generate;
}

void Editor::BaseEditor::onInitialize()
{

}

void Editor::BaseEditor::onStart()
{

}

void Editor::BaseEditor::onResize()
{

}

void Editor::BaseEditor::onUpdate()
{

}

Editor::BaseEditor* Editor::BaseEditor::getEditor(uint64_t uuid)
{
	return (Editor::BaseEditor*)Core::Core_Pool::p_all_editor.find(uuid)->second;
}

std::string Editor::BaseEditor::g_rootPath = ThirdParty::Core::getModuleCurrentDirectory();
Core::FileInfo* Editor::BaseEditor::g_root_folder = nullptr;
Core::FileInfo* Editor::BaseEditor::g_root_file = nullptr;
std::vector<Core::FileInfo*> Editor::BaseEditor::m_file_lists;

Editor::ZEditor::ZEditor(bool show)
{
	this->m_show = show;
}

Editor::ZEditor::~ZEditor()
{

}

Editor::ZEditor::ZEditor(const char* title, bool show)
{
	this->m_show = show;
	this->m_editor_title = title;
}

void Editor::ZEditor::onRender()
{
	this->p_onRender_func();
}

void Editor::ZEditor::onUpdate()
{

}

void Editor::destroy()
{
	Core::Core_Pool::destroyEditor();
}
