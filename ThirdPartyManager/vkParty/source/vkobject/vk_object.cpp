#include "vkobject/vk_object.h"

ThirdParty::vkObject::vkObject(const std::string &_label):m_label(_label) {
	m_into_pool = true;
	Core::g_vk_objects.push_back(this);
	if(!p_template) p_template = imgui::Template::newTemplate();
	m_result = VK_SUCCESS;
}
ThirdParty::vkObject::~vkObject()
{
	if(p_template) imgui::Template::deleteTemplate(p_template);
}

void ThirdParty::vkObject::removeVkObject(vkObject* _obj)
{
	ThirdParty::Core::g_vk_objects.push_back(_obj);
}

void ThirdParty::vkObject::addVkObject(vkObject* _obj)
{
	_obj->removeFromVkObjects();
}

void ThirdParty::vkObject::removeFromVkObjects()
{
	if (m_into_pool) {
		auto& _a = ThirdParty::Core::g_vk_objects;
		for (auto _i = _a.begin(); _i < _a.end(); _i++)
			if (*_i == this) { _a.erase(_i); break; }
		m_into_pool = false;
	}
}

void ThirdParty::vkObject::initObjFromTemplate()
{

}

void ThirdParty::vkObject::initTemplate() {
	if(!p_template) p_template = imgui::Template::newTemplate();
}
