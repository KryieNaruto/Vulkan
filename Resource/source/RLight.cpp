#include "RLight.h"
#include "model/RPlane.h"
Json::Value Core::Resource::RLight::serializeToJSON()
{
	return Json::Value();
}

Core::Resource::RLight::~RLight()
{
	if (p_property) delete p_property;
}

Core::Resource::RLight::RLight()
{
	init_0();
}

Core::Resource::RLight::RLight(LIGHT_TYPE _type, glm::vec3 _pos /*= glm::vec3(0)*/, glm::vec4 _color /*= glm::vec4(1)*/, glm::vec3 _dir /*= glm::vec3(0)*/)
{
	init_0();
	p_light_property->m_type = _type;
	p_light_property->m_vec3_pos = _pos;
	p_light_property->m_color = _color;
	p_light_property->m_strength = _color.z;
	p_light_property->m_axis_rotate = _dir;
}

Core::Resource::RLight::RLight(const std::string& _name)
{
	init_0();
	this->m_res_name = _name;
}

void Core::Resource::RLight::deserializeToObj(Json::Value& root)
{

}

void Core::Resource::RLight::init_0()
{
	this->m_json_title = "RLight";
	this->m_res_name = "Light";
	this->p_id_generate->init(RESOURCE_TYPE::RLIGHT);
	this->m_uuid = this->p_id_generate->generateUUID();
	this->p_property = new RLightProperty;
	this->p_light_property = (RLightProperty*)p_property;
}

Core::Resource::RLight* Core::Resource::RLight::update()
{
	p_property->update();
	return this;
}

void Core::Resource::RLight::initTemplate()
{
}

