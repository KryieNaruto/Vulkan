#include "ZResource.h"
#include <Core/include/core_global.h>

Core::Resource::ZResource::ZResource()
{
	this->p_id_generate = new IDGenerate();
	this->m_initialized = false;
	if(!p_template)
		p_template = ThirdParty::imgui::Template::newTemplate();
}

Core::Resource::ZResource::~ZResource()
{
	if (this->m_initialized) {
		delete this->p_id_generate;
		this->m_uuid = 0;
	}
	if(p_template) {
		ThirdParty::imgui::Template::deleteTemplate(p_template);
	}
}

std::string Core::Resource::ZResource::getResourcePath(const std::string& _path)
{
	std::string str = _path;
	std::replace(str.begin(), str.end(), '/', '\\');
	const auto& str_0 = std::filesystem::absolute(str).string();
	const auto& str_1 = str.c_str();
	if (str_0 == _path) return _path;
	str = Core::g_project_info->m_resource_path + "\\" + _path;
	return str;
}

void Core::Resource::ZResource::initObjFromTemplate()
{

}

void Core::Resource::ZResource::initTemplate()
{

}

