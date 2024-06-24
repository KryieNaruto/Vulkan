#include "vkobject/vk_texture_ext.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include "vk_party_manager.h"
#include "vkobject/vk_sampler.h"
#include <ThirdParty/imgui/imgui_impl_vulkan.h>

#undef max
ThirdParty::TextureEXT::TextureEXT()
	:Texture("Material Editor Createing")
{
	initTemplate();
}

ThirdParty::TextureEXT::TextureEXT(const VkExtent3D& _extent, VkFormat _format, VkImageTiling _tiling, VkImageUsageFlags _usage, VkMemoryPropertyFlags _prop, VkImageLayout _layout, uint32_t _layer_count, VkSampleCountFlags _sample_count , const std::string& _name, uint32_t _mip_map)
{
	if (!p_file_info) {
		p_file_info = new TextureFileInfo;
		p_file_info->m_width = _extent.width;
		p_file_info->m_height = _extent.height;
		p_file_info->m_aspect = (float)p_file_info->m_width / (float)p_file_info->m_height;
		p_file_info->m_channels = 4;
		p_file_info->m_path = _name;
	}
	m_format = _format;
	m_aspect = ThirdParty::hasDepthComponent(_format) ? VK_IMAGE_ASPECT_DEPTH_BIT : VK_IMAGE_ASPECT_COLOR_BIT;
	if (ThirdParty::hasStencilComponent(_format)) m_aspect |= VK_IMAGE_ASPECT_STENCIL_BIT;
	p_image_create_info = new VkImageCreateInfo;
	*p_image_create_info = createImage(_extent, _format, _tiling, _usage, _prop, m_image, _sample_count, _layer_count, _mip_map);
	m_image_memory = vkManager.bindImageMemory(m_image, _prop, m_size, m_offset);
	// ���mipmap > 1 ����mipmap
	if (_mip_map > 1) {
		m_mipLevels = _mip_map;
		transitionImageLayout(VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, { VK_IMAGE_ASPECT_COLOR_BIT,0,m_mipLevels,0,_layer_count });
		// ����mipmap
		for (int i = 0; i < _layer_count; i++) {
			vkManager.generateMipmaps(
				m_image,
				{ (uint32_t)p_file_info->m_width,(uint32_t)p_file_info->m_height, 1 },
				m_mipLevels,
				i);
		}
	}
	p_image_view_create_info = new VkImageViewCreateInfo;
	if(_layer_count == 6 || _mip_map > 1)
		for (int _mip = 0; _mip < _mip_map;_mip++) {
			for (int i = 0; i < _layer_count; i++) {
				VkImageView _v = VK_NULL_HANDLE;
				*p_image_view_create_info = createImageView(m_image, _format, m_aspect, _v, i, 1, _mip, 1);		// ����6��view
				m_image_view_array.push_back(_v);
			}
		}
	*p_image_view_create_info = createImageView(m_image, _format, m_aspect, m_image_view, 0, _layer_count, 0, _mip_map);		// ���յ�Cube����View

	// ���ֱ仯
	for (uint32_t i = 0; i < _layer_count; i++)
		transitionImageLayout(VK_IMAGE_LAYOUT_UNDEFINED, _layout, { m_aspect,0,1,i,1 });

	if (_name != "")
		Core::g_textureEXT_loaded[p_file_info->m_path] = this;
}

ThirdParty::TextureEXT::TextureEXT(std::string& _path, const std::string& _name)
	:Texture(_path)
{
	loadDataFromFile(_path);
	if (_name != "")
		Core::g_textureEXT_loaded[_name] = this;
}

ThirdParty::TextureEXT::TextureEXT(const std::vector<std::string>& _paths, const std::string& _name)
{
	loadDataFromFiles(_paths);
	if(_name != "")
		Core::g_textureEXT_loaded[_name] = this;
}

ThirdParty::TextureEXT& ThirdParty::TextureEXT::operator=(const TextureEXT& _tex)
{
	m_copy = true;

	p_stage = _tex.p_stage;
	p_file_info = _tex.p_file_info;
	m_size = _tex.m_size;
	m_offset = _tex.m_offset;
	p_image_create_info = _tex.p_image_create_info;
	p_image_view_create_info = _tex.p_image_view_create_info;
	m_image_memory = _tex.m_image_memory;

	return *this;
}

ThirdParty::TextureEXT::TextureEXT(const TextureEXT& _tex)
{
	m_copy = true;

	m_format = _tex.m_format;
	m_layout_old = _tex.m_layout_old;
	m_layout_current = _tex.m_layout_current;
	p_stage = _tex.p_stage;
	p_file_info = _tex.p_file_info;
	m_size = _tex.m_size;
	m_offset = _tex.m_offset;
	p_image_create_info = _tex.p_image_create_info;
	p_image_view_create_info = _tex.p_image_view_create_info;
	m_image_memory = _tex.m_image_memory;
}

ThirdParty::TextureEXT::TextureEXT(const VkExtent3D& _extent, VkFormat _format, VkImageTiling _tiling, VkImageUsageFlags _usage, VkMemoryPropertyFlags _prop, VkImageLayout _layout, VkSampleCountFlags _sample_count,const std::string& _name)
{
	if(!p_file_info) {
		p_file_info = new TextureFileInfo;
		p_file_info->m_width = _extent.width;
		p_file_info->m_height = _extent.height;
		p_file_info->m_aspect = (float)p_file_info->m_width / (float)p_file_info->m_height;
		p_file_info->m_channels = 4;
		p_file_info->m_path = _name;
	}
	m_format = _format;
	m_aspect = ThirdParty::hasDepthComponent(_format) ? VK_IMAGE_ASPECT_DEPTH_BIT : VK_IMAGE_ASPECT_COLOR_BIT;
	if (ThirdParty::hasStencilComponent(_format)) m_aspect |= VK_IMAGE_ASPECT_STENCIL_BIT;
	p_image_create_info = new VkImageCreateInfo;
	*p_image_create_info = createImage(_extent, _format, _tiling, _usage, _prop, m_image, _sample_count);
	m_image_memory = vkManager.bindImageMemory(m_image, _prop, m_size, m_offset);
	p_image_view_create_info = new VkImageViewCreateInfo;
	*p_image_view_create_info = createImageView(m_image, _format, m_aspect,m_image_view);

	// ���ֱ仯
	transitionImageLayout(_layout, { m_aspect,0,1,0,1 });

	if (_name != "")
	Core::g_textureEXT_loaded[p_file_info->m_path] = this;
}

ThirdParty::TextureEXT::TextureEXT(VkSampler _sampler,bool _only_sampler)
	:m_only_sampler(_only_sampler)
{
	m_sampler = _sampler;
}

ThirdParty::TextureEXT::~TextureEXT()
{
	for (const auto& _view : m_image_view_array)
		vkManager.destroyVkObject(_view);
	if (p_stage) delete p_stage;
	if (p_file_info) delete p_file_info;
	for (const auto& _info : p_cube_file_info) delete _info;
	if (p_image_create_info) delete p_image_create_info;
	if (p_image_view_create_info) delete p_image_view_create_info;

	if (m_image != VK_NULL_HANDLE) vkManager.destroyVkObject(m_image);
	if (m_image_view != VK_NULL_HANDLE) vkManager.destroyVkObject(m_image_view);
	if (m_image_memory != VK_NULL_HANDLE) {
		vkManager.unbindImageMemory(m_image_memory, m_size, m_offset);
	}
}

void ThirdParty::TextureEXT::copyData(void* data)
{
	// ���ֱ任 current->transfer
	transitionImageLayout(m_layout_current, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
	// �ݴ滺��
	p_stage = new Buffer(
		m_size,
		VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
	);
	// ��������
	p_stage->bindData(data);
	p_stage->copyDataToVkImage(m_image, p_file_info->m_width, p_file_info->m_height, VK_IMAGE_ASPECT_COLOR_BIT, 0);
	// ���ֱ任 transfer -> shader_read_only
	transitionImageLayout(VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
}

void ThirdParty::TextureEXT::copyTextureEXT(TextureEXT* _tex)
{
	auto cmd = vkManager.BeginSingleCommandBuffer();

	VkImageCopy _region;
	_region.srcOffset = { 0,0,0 };
	_region.srcSubresource = { _tex->m_aspect ,_tex->m_mipLevels,0,1 };
	_region.dstOffset = { 0,0,0 };
	_region.dstSubresource = { m_aspect ,m_mipLevels,0,1 };
	_region.extent = p_image_create_info->extent;

	// SRCת������
	VkImageLayout _src_current = _tex->m_layout_current;
	_tex->transitionImageLayout(
		_src_current,
		VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, { _tex->m_aspect,0,1,0,1 }, cmd);
	// DSTת������
	transitionImageLayout(
		VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
		VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, { m_aspect,0,1,0,1 }, cmd);


	vkCmdCopyImage(cmd, _tex->m_image, _tex->m_layout_current, m_image, m_layout_current, 1, &_region);

	// SRCת������
	_tex->transitionImageLayout(
		VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
		_src_current, { _tex->m_aspect,0,1,0,1 }, cmd);
	// DSTת������
	transitionImageLayout(
		VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
		VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, { m_aspect,0,1,0,1 }, cmd);
	vkManager.endSingleCommandBuffer();
}

void ThirdParty::TextureEXT::copyAllTextureEXT(TextureEXT* _tex)
{
	auto cmd = vkManager.BeginSingleCommandBuffer();
	for (uint32_t i = 0; i < _tex->p_image_create_info->mipLevels; i++) {
		VkImageCopy _region;
		_region.srcOffset = { 0,0,0 };
		_region.srcSubresource = { _tex->m_aspect ,i,0,_tex->p_image_create_info->arrayLayers };
		_region.dstOffset = { 0,0,0 };
		_region.dstSubresource = { m_aspect ,i,0,_tex->p_image_create_info->arrayLayers };
		_region.extent = p_image_create_info->extent;
		_region.extent.width *= std::pow(0.5, i);
		_region.extent.height *= std::pow(0.5, i);

		// SRCת������
		VkImageLayout _src_current = _tex->m_layout_current;
		_tex->transitionImageLayout(
			_src_current,
			VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, { _tex->m_aspect,i,1,0,_tex->p_image_create_info->arrayLayers }, cmd);
		// DSTת������
		transitionImageLayout(
			VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, { m_aspect,i,1,0,_tex->p_image_create_info->arrayLayers }, cmd);


		vkCmdCopyImage(cmd, _tex->m_image, _tex->m_layout_current, m_image, m_layout_current, 1, &_region);

		// SRCת������
		_tex->transitionImageLayout(
			VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
			_src_current, { _tex->m_aspect,i,1,0,_tex->p_image_create_info->arrayLayers }, cmd);
		// DSTת������
		transitionImageLayout(
			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, { m_aspect,i,1,0,_tex->p_image_create_info->arrayLayers }, cmd);
	}
	vkManager.endSingleCommandBuffer();
}

void ThirdParty::TextureEXT::reload(std::string& _path)
{

}

void ThirdParty::TextureEXT::recreate(const VkExtent3D& _extent)
{
	if (!p_image_create_info || !p_image_view_create_info) return;
	vkManager.destroyVkObject(m_image);
	vkManager.destroyVkObject(m_image_view);
	vkManager.unbindImageMemory(m_image_memory, m_size, m_offset);
	p_image_create_info->extent = _extent;

	m_image = std::get<VkImage>(vkManager.createVkObject(p_image_create_info));
	m_image_memory = vkManager.bindImageMemory(m_image, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, m_size, m_offset);
	VkImageLayout _current = m_layout_current;
	m_layout_current = VK_IMAGE_LAYOUT_UNDEFINED;
	transitionImageLayout(_current, { m_aspect,0,1,0,1 });
	p_image_view_create_info->image = m_image;
	m_image_view = std::get<VkImageView>(vkManager.createVkObject(p_image_view_create_info));
}

ThirdParty::TextureEXT* ThirdParty::TextureEXT::loadDataFromFile(std::string& _path)
{
	if (Core::g_textureEXT_loaded.contains(_path)) return (TextureEXT*)Core::g_textureEXT_loaded[_path];
	if (m_only_sampler) return this;
	if (!p_file_info) p_file_info = new TextureFileInfo;
	p_file_info->m_path = _path;
	stbi_uc* pixels = stbi_load(
		p_file_info->m_path.c_str(),
		&p_file_info->m_width,
		&p_file_info->m_height,
		&p_file_info->m_channels, STBI_rgb_alpha);

	m_size = p_file_info->m_width * p_file_info->m_height * 4;
	p_file_info->m_aspect = (float)p_file_info->m_width / (float)p_file_info->m_height;
	if (!pixels)
	{
		Core::warn("Failed to load TextureEXT from file:%s", _path.c_str());
		return nullptr;
	}
	// ����ϸ���ȼ�
	m_mipLevels = static_cast<uint32_t>(std::floor(std::log2(std::max(p_file_info->m_width, p_file_info->m_height)))) + 1;
	// �ݴ滺��
	p_stage = new Buffer(
		m_size,
		VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
		);
	// ��������
	p_stage->bindData(pixels);
	stbi_image_free(pixels);

	// ����vkimage
	p_image_create_info = new VkImageCreateInfo;
	p_image_create_info->sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	p_image_create_info->imageType = VK_IMAGE_TYPE_2D;
	p_image_create_info->extent = { (uint32_t)p_file_info->m_width, (uint32_t)p_file_info->m_height,1 };
	p_image_create_info->mipLevels = m_mipLevels;
	p_image_create_info->arrayLayers = 1;
	p_image_create_info->format = VK_FORMAT_R8G8B8A8_UNORM;
	p_image_create_info->tiling = VK_IMAGE_TILING_OPTIMAL;
	p_image_create_info->initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	p_image_create_info->usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
	p_image_create_info->sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	p_image_create_info->queueFamilyIndexCount = 0;
	p_image_create_info->pQueueFamilyIndices = 0;
	p_image_create_info->samples = VK_SAMPLE_COUNT_1_BIT;
	p_image_create_info->flags = 0;
	p_image_create_info->pNext = 0;
	m_image = std::get<VkImage>(vkManager.createVkObject(p_image_create_info));
	m_image_memory = vkManager.bindImageMemory(m_image, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, m_size, m_offset);

	// ���ֱ任 undefined -> transfer
	transitionImageLayout(VK_IMAGE_LAYOUT_UNDEFINED,
		VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
		{ VK_IMAGE_ASPECT_COLOR_BIT, 0,m_mipLevels,0,1 });
	// ����buffer��image
	p_stage->copyDataToVkImage(m_image, p_file_info->m_width, p_file_info->m_height, VK_IMAGE_ASPECT_COLOR_BIT, 0);
	// ����p_stage
	delete p_stage;
	p_stage = nullptr;
	// ����mipmap
	vkManager.generateMipmaps(m_image, p_image_create_info->extent, m_mipLevels);
	// ���ֱ任 transfer -> shader_read_only
	m_layout_old = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
	m_layout_current = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	/*transitionImageLayout(VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
		VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
		{ VK_IMAGE_ASPECT_COLOR_BIT, 0,m_mipLevels,0,1 });*/

	// ����vkImageView
	p_image_view_create_info = new VkImageViewCreateInfo;
	p_image_view_create_info->sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	p_image_view_create_info->image = m_image;
	p_image_view_create_info->viewType = VK_IMAGE_VIEW_TYPE_2D;
	p_image_view_create_info->format = VK_FORMAT_R8G8B8A8_UNORM;
	p_image_view_create_info->subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT,0,m_mipLevels,0,1 };
	p_image_view_create_info->components = {
		VK_COMPONENT_SWIZZLE_IDENTITY,
		VK_COMPONENT_SWIZZLE_IDENTITY,
		VK_COMPONENT_SWIZZLE_IDENTITY,
		VK_COMPONENT_SWIZZLE_IDENTITY
	};
	p_image_view_create_info->flags = 0;
	p_image_view_create_info->pNext = 0;
	m_image_view = std::get<VkImageView>(vkManager.createVkObject(p_image_view_create_info));

	Core::g_textureEXT_loaded[_path] = this;
	return this;
}

ThirdParty::TextureEXT* ThirdParty::TextureEXT::loadDataFromFiles(const std::vector<std::string>& _paths)
{
	if (m_only_sampler) return this;
	m_is_cube = true;
	m_layers = _paths.size();
	p_cube_file_info.resize(m_layers);
	uint32_t _index = 0;
	for (const auto& _path : _paths) {
		auto& _file_info = p_cube_file_info[_index];
		if (!_file_info) {
			_file_info = new TextureFileInfo;
			_file_info->m_path = _path;
			// ����ͼƬ
			stbi_uc* pixels = stbi_load(
				_file_info->m_path.c_str(),
				&_file_info->m_width,
				&_file_info->m_height,
				&_file_info->m_channels, STBI_rgb_alpha);
			if (!pixels) {
				Core::error("Failed to load image:%s", _path.c_str());
			}
			m_size = _file_info->m_width * _file_info->m_height * 4;
			_file_info->m_aspect = (float)_file_info->m_width / (float)_file_info->m_height;
			// �ݴ滺��
			p_stage = new Buffer(
				m_size,
				VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
				VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
			);
			// �������ݵ�stage buffer
			p_stage->bindData(pixels);
		}
		// �Ե�һ��ͼƬ��СΪ��׼, ������������6����ͼ��һ��image,Ȼ��ʹ��һ��view���ʡ�
		if (_index == 0) {
			auto& _CI = p_image_create_info;
			m_mipLevels = static_cast<uint32_t>(std::floor(std::log2(std::max(_file_info->m_width, _file_info->m_height)))) + 1;
			// ����vkimage
			if (!_CI || m_image == VK_NULL_HANDLE) {
				_CI = new VkImageCreateInfo;
				_CI->sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
				_CI->pNext = 0;
				_CI->imageType = VK_IMAGE_TYPE_2D;
				_CI->format = VK_FORMAT_R8G8B8A8_UNORM;
				_CI->mipLevels = m_mipLevels;
				_CI->samples = VK_SAMPLE_COUNT_1_BIT;
				_CI->tiling = VK_IMAGE_TILING_OPTIMAL;
				_CI->sharingMode = VK_SHARING_MODE_EXCLUSIVE;
				_CI->initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
				_CI->extent = { (uint32_t)_file_info->m_width,(uint32_t)_file_info->m_height,1 };
				_CI->usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
				_CI->arrayLayers = m_layers;		// Layer ����
				_CI->queueFamilyIndexCount = 0;
				_CI->pQueueFamilyIndices = 0;
				_CI->flags = VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;	// �����ڴ���VK_IMAGE_VIEW_TYPE_CUBE���͵�view
				m_image = std::get<VkImage>(vkManager.createVkObject(_CI));
				m_image_memory = vkManager.bindImageMemory(m_image, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, m_size, m_offset);
				// ���ֱ任,�Ա�buffer���Ƶ�image
				transitionImageLayout(VK_IMAGE_LAYOUT_UNDEFINED,
					VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
					{ VK_IMAGE_ASPECT_COLOR_BIT, 0,m_mipLevels,0,m_layers });
			}
		}
		// �������ݵ�image
		{
			// ��������
			p_stage->copyDataToVkImage(m_image, _file_info->m_width, _file_info->m_height, VK_IMAGE_ASPECT_COLOR_BIT, 0, _index);
			// ������ɺ�ɾ��stage buffer
			delete p_stage;
			p_stage = nullptr;
		}
		_index++;
		// ���浽ȫ��
		Core::g_textureEXT_loaded[_path] = this;
	}
	// ȫ����ȡ��image��ʼ����mipmap
	{
		auto& _file_info = p_cube_file_info[0];
		for (uint32_t _layer = 0; _layer < m_layers; _layer++) {
			/*transitionImageLayout(VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
				VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
				{ VK_IMAGE_ASPECT_COLOR_BIT, 0,m_mipLevels,_layer,1 });*/
			vkManager.generateMipmaps(
				m_image, 
				{ (uint32_t)_file_info->m_width,(uint32_t)_file_info->m_height, 1 }, 
				m_mipLevels, 
				_layer);
		}
		// ���ֱ任 transfer -> shader_read_only
		m_layout_old = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
		m_layout_current = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	}

	// ����vkImageView
	{
		p_image_view_create_info = new VkImageViewCreateInfo;
		p_image_view_create_info->sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		p_image_view_create_info->image = m_image;
		p_image_view_create_info->viewType = VK_IMAGE_VIEW_TYPE_CUBE;
		p_image_view_create_info->format = VK_FORMAT_R8G8B8A8_UNORM;
		p_image_view_create_info->subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT,0,m_mipLevels,0,m_layers };
		p_image_view_create_info->components = {
			VK_COMPONENT_SWIZZLE_IDENTITY,
			VK_COMPONENT_SWIZZLE_IDENTITY,
			VK_COMPONENT_SWIZZLE_IDENTITY,
			VK_COMPONENT_SWIZZLE_IDENTITY
		};
		p_image_view_create_info->flags = 0;
		p_image_view_create_info->pNext = 0;
		m_image_view = std::get<VkImageView>(vkManager.createVkObject(p_image_view_create_info));
	}

	if (isArray()) {
		auto _image_view_create_info = new VkImageViewCreateInfo;
		for (uint32_t i = 0; i < m_layers; i++) {
			_image_view_create_info->sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
			_image_view_create_info->image = m_image;
			_image_view_create_info->viewType = VK_IMAGE_VIEW_TYPE_2D;
			_image_view_create_info->format = VK_FORMAT_R8G8B8A8_UNORM;
			_image_view_create_info->subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, m_mipLevels, i, 1 };
			_image_view_create_info->components = {
				VK_COMPONENT_SWIZZLE_IDENTITY,
				VK_COMPONENT_SWIZZLE_IDENTITY,
				VK_COMPONENT_SWIZZLE_IDENTITY,
				VK_COMPONENT_SWIZZLE_IDENTITY
			};
			_image_view_create_info->flags = 0;
			_image_view_create_info->pNext = 0;
			auto _image_view = std::get<VkImageView>(vkManager.createVkObject(_image_view_create_info));
			m_image_view_array.push_back(_image_view);
		}
		delete _image_view_create_info;
	}
	return this;
	
}

void ThirdParty::TextureEXT::initTemplate()
{
	vkObject::initTemplate();
	// ����·��
	{
		auto _input = new ThirdParty::imgui::SlotTemplateInput();
		_input->setName("Path");
		_input->p_data = nullptr;
		p_template->addInputSlotTemplate(_input);
	}
	// ���·��
	{
		auto _output = new ThirdParty::imgui::SlotTemplateOutput();
		_output->setName("Texture");
		_output->setName2("Texture");
		_output->p_data = this;
		_output->m_functional = [this]()->void {
			if (ImGui::CollapsingHeader("Final Texture")) {
				if (_final_sets.contains(this)) {
					auto _aspect = this->getAspect();
					auto _width = p_template->p_inputs[0]->m_final_size.x;
					auto _height = _width / _aspect;
					ImGui::Image(_final_sets[this], { _width,_height });
				}
				else {
					if (m_image_view != VK_NULL_HANDLE) {
						if (m_sampler == VK_NULL_HANDLE) m_sampler = ThirdParty::Sampler::g_all_samplers["Sampler Repeat"]->getVkSampler();
						_final_sets[this] = ImGui_ImplVulkan_AddTexture(m_sampler, m_image_view, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
					}
				}
				static const uint32_t _mode_count = 2;
				static const char* _preview[_mode_count] = {
					"Repeat",
					"None",
				};
				static const char* _mode[_mode_count] = {
					"Sampler Repeat",
					"Sampler",
				};
				static uint32_t _choose = 0;
				if (ImGui::BeginCombo("Repeat Mode", _preview[_choose])) {
					for(int i=0;i< _mode_count;i++)
						if (ImGui::Selectable(_preview[i])) {
							_choose = i;
							m_sampler = ThirdParty::Sampler::g_all_samplers[_mode[i]]->getVkSampler();
						}
					ImGui::EndCombo();
				}
			}
			};
		p_template->addOutputSlotTemplate(_output);
	}
}

void ThirdParty::TextureEXT::initObjFromTemplate()
{
	// ��������
	if (!p_template->p_inputs[0]->p_data) return;
	auto _path = *(std::string*)p_template->p_inputs[0]->p_data;
	auto _tex = loadDataFromFile(_path);
	_tex->m_sampler = ThirdParty::Sampler::g_all_samplers["Sampler Repeat"]->getVkSampler();	p_template->p_outputs[0]->p_data = _tex; // �������
	
	p_template->p_inputs[0]->m_functional = [this, _path]()->void {
		if (ImGui::CollapsingHeader("Original Texture")) {
			auto _str = std::string(_path);
			if (_sets.contains(_str)) {
				auto _aspect = this->getAspect();
				auto _width = p_template->p_inputs[0]->m_final_size.x;
				auto _height = _width / _aspect;
				ImGui::Image(_sets[_str], { _width,_height });
			}
			else {
				if (m_sampler == VK_NULL_HANDLE) m_sampler = ThirdParty::Sampler::g_all_samplers["Sampler Repeat"]->getVkSampler();
				_sets[_str] = ImGui_ImplVulkan_AddTexture(m_sampler, m_image_view, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
			}
		}
		};
}
