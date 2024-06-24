#include "sub/Texture.h"
#include <Core/include/core_global.h>
#include <ThirdPartyManager/Core/common/include/debug_to_console.h>
Core::Resource::Texture::~Texture()
{
	if (p_texture_ext) delete p_texture_ext;
}

Core::Resource::Texture::Texture(const std::string& _path)
	:RTexture(_path), SceneObject(_path)
{
	if (Core::g_file_manager->file_is_exists(_path.c_str())) {
		p_texture_ext = new ThirdParty::TextureEXT(m_path);
	}
	else {
		LOG_ERROR("Texture is not exist:{0}", _path);
	}
	setZResourceProperty(p_property);
}

Core::Resource::Texture::Texture(uint32_t _width, uint32_t _height)
	: RTexture(Core::Resource::getName("Empty Image")), SceneObject("Empty Image")
{
	p_texture_ext = new ThirdParty::TextureEXT(
		{ _width,_height,1 },
		VK_FORMAT_R8G8B8A8_UNORM,
		VK_IMAGE_TILING_OPTIMAL,
		VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
		VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
		VK_SAMPLE_COUNT_1_BIT,
		m_res_name
	);
	std::vector<uint32_t> _data(_width * _height, 0);
	p_texture_ext->copyData(_data.data());
	setZResourceProperty(p_property);
}

void Core::Resource::Texture::propertyEditor()
{

}
