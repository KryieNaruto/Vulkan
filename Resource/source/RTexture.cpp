#include "RTexture.h"

Core::Resource::RTexture::RTexture(const std::string& _name /*= "RTexture"*/)
{
	this->m_res_name = _name;
	this->m_json_title = "RTexture";
	this->p_id_generate->init(RESOURCE_TYPE::RTEXTURE);
	this->m_uuid = this->p_id_generate->generateUUID();
}

Core::Resource::RTexture::RTexture(const std::string& _path,aiTextureType _type, const std::string& _name /*= "RTexture"*/)
{
	this->m_res_name = _name;
	this->m_path = _path;
	this->m_type = aiTextureTypeToResourceType(_type);

	this->m_json_title = "RTexture";
	this->p_id_generate->init(m_type);
	this->m_uuid = this->p_id_generate->generateUUID();
}

Core::Resource::RTexture::~RTexture()
{

}

Json::Value Core::Resource::RTexture::serializeToJSON()
{
	return Json::Value();
}

void Core::Resource::RTexture::deserializeToObj(Json::Value& _root)
{

}

Core::RESOURCE_TYPE Core::Resource::RTexture::aiTextureTypeToResourceType(aiTextureType _type)
{
	switch (_type)
	{
	case aiTextureType_DIFFUSE:
		return RTEXTURE_DIFFUSE;
	case aiTextureType_SPECULAR:
		return RTEXTURE_SPECULAR;
	default:
		break;
	}
}

void Core::Resource::RTexture::initTemplate()
{
}
