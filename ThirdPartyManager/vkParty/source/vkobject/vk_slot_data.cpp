#include "vkobject/vk_slot_data.h"
#include "vkobject/vk_sampler.h"
#define SAMPLER "Sampler"
#define TEXTURE_DEFAULT "default"
#define TEXTURE_SKYBOX_DEFAULT "default-skybox"
ThirdParty::SlotData::SlotData(SpvReflectDescriptorBinding* _binding_info)
	:p_binding_property(_binding_info)
{
	setup();
}

ThirdParty::SlotData::SlotData(SlotData* _slot_data)
{
	m_is_copy = true;
	p_binding_property = _slot_data->p_binding_property;
	setup();
	setupData();
}

ThirdParty::SlotData::~SlotData()
{
	for (const auto& _write_info : p_write_infos)
		delete _write_info;
	for (const auto& _ubo : p_uniform_buffer.second)
		delete _ubo;
	for (const auto& _space : p_datas)
		free(_space);
}

ThirdParty::SlotData* ThirdParty::SlotData::bindData(std::string _member, void* _data, uint32_t _index /*= 0*/)
{
	// UBO
	if (m_descriptor_type == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER) {
		// 检查是否创建Member 内存空间
		if ((int)p_datas.size() <= (int)_index) {
			void* _ptr = malloc(m_block_size);	// 申请内存空间
			p_datas.push_back(_ptr);
		}
		auto _member_size = 0;
		auto _member_offset = 0;
		// 查找是否存在_member, 获取member的size与offset
		if (m_struct_property.second.find(_member) != m_struct_property.second.end()) {
			auto _member_pair = m_struct_property.second[_member];
			_member_size = _member_pair.first;
			_member_offset = _member_pair.second;
		}
		else {
			bindData(_data, _index);
		}

		char* src = (char*)p_datas[_index] + _member_offset;
		memcpy(src, _data, _member_size);
	}
	return this;
}

ThirdParty::SlotData* ThirdParty::SlotData::bindData(VkSampler _sampler, uint32_t _index)
{
	if (m_descriptor_type == VK_DESCRIPTOR_TYPE_SAMPLER||
		m_descriptor_type == VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE || 
		m_descriptor_type == VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER||
		m_descriptor_type == VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT) {
		if (_sampler == VK_NULL_HANDLE) return this;
		auto& _texture_vec = p_texture_ext.second;
		// 插槽位置不够
		if (_texture_vec.size() <= _index) {
			auto _texture_ext = new TextureEXT(_sampler);
			_texture_vec.push_back(_texture_ext);
			// 新建bufferInfo
			VkDescriptorImageInfo _image_info;
			_image_info.sampler = _texture_ext->getVkSampler();
			_image_info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			_image_info.imageView = _texture_ext->getVkImageView();
			std::get<std::vector<VkDescriptorImageInfo>>(m_infos).push_back(_image_info);
		}

		_texture_vec[_index]->setVkSampler(_sampler);

		return this;
	}
	return this;
}

ThirdParty::SlotData* ThirdParty::SlotData::bindData(TextureEXT* _tex_ext, uint32_t _index)
{
	if (m_descriptor_type == VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE || 
		m_descriptor_type == VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER||
		m_descriptor_type == VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT
		) {
		if (!_tex_ext) return this;
		auto& _texture_vec = p_texture_ext.second;
		// 插槽位置不够
		while (_texture_vec.size() <= _index) {
			auto _texture_ext = _tex_ext;
			_texture_vec.push_back(_texture_ext);
			// 新建bufferInfo
			VkDescriptorImageInfo _image_info;
			_image_info.sampler = _texture_ext->getVkSampler();
			_image_info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			_image_info.imageView = _texture_ext->getVkImageView();
			std::get<std::vector<VkDescriptorImageInfo>>(m_infos).push_back(_image_info);
		}
		if (!_tex_ext->hasTexture())
			_texture_vec[_index]->copyTextureEXT(_tex_ext);
		else {
			if (_tex_ext != _texture_vec[_index]) {
				_texture_vec[_index] = _tex_ext;
				m_texture_change_index.insert(_index);
			}
		}
		m_need_data = false;
		return this;
	}
	if (m_descriptor_type == VK_DESCRIPTOR_TYPE_SAMPLER)
		return nullptr;
}

ThirdParty::SlotData* ThirdParty::SlotData::bindData(std::string _img_path, uint32_t _index /*= 0*/)
{
	if (m_descriptor_type == VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE || 
		m_descriptor_type == VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER||
		m_descriptor_type == VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT
		) {
		auto& _texture_vec = p_texture_ext.second;
		// 插槽位置不够
		if (_texture_vec.size() <= _index) {
			TextureEXT* _texture_ext = new TextureEXT(_img_path);
			_texture_vec.push_back(_texture_ext);
			// 新建bufferInfo
			VkDescriptorImageInfo _image_info;
			_image_info.sampler = _texture_ext->getVkSampler();
			_image_info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			_image_info.imageView = _texture_ext->getVkImageView();
			std::get<std::vector<VkDescriptorImageInfo>>(m_infos).push_back(_image_info);
		}
		else {
			_texture_vec[_index]->load(_img_path);
		}
		return this;
	}
	if (m_descriptor_type == VK_DESCRIPTOR_TYPE_SAMPLER)
		return nullptr;
}

ThirdParty::SlotData* ThirdParty::SlotData::bindData(uint32_t _member_index, void* _data, uint32_t _index /*= 0*/)
{
	// 检查是否创建Member 内存空间
	if ((int)p_datas.size() <= (int)_index) {
		void* _ptr = malloc(m_block_size);	// 申请内存空间
		p_datas.push_back(_ptr);
	}

	auto _member_size = 0;
	auto _member_offset = 0;

	// 获取member的size与offset
	{
		auto _member_ite = m_struct_property_order.second.begin();
		for (size_t i = 0; i < m_member_index; i++) _member_ite++;
		auto _member_pair = m_struct_property.second[(*_member_ite).second];
		_member_size = _member_pair.first;
		_member_offset = _member_pair.second;
		m_member_index++;
		// 如果执行完一轮，则重置当前member index
		if (m_member_index > m_block_member_count - 1)
			m_member_index = 0;
	}

	char* src = (char*)p_datas[_index] + _member_offset;
	memcpy(src, _data, _member_size);

	return this;

}

ThirdParty::SlotData* ThirdParty::SlotData::bindData(void* _data, uint32_t _index /*= 0*/)
{
	// 检查是否创建Member 内存空间
	if ((int)p_datas.size() <= (int)_index) {
		void* _ptr = malloc(m_block_size);	// 申请内存空间
		p_datas.push_back(_ptr);
	}

	auto _member_size = 0;
	auto _member_offset = 0;

	// 获取member的size与offset
	{
		auto _member_ite = m_struct_property_order.second.begin();
		for (size_t i = 0; i < m_member_index; i++) _member_ite++;
		auto _member_pair = m_struct_property.second[(*_member_ite).second];
		_member_size = _member_pair.first;
		_member_offset = _member_pair.second;
		m_member_index++;
		// 如果执行完一轮，则重置当前member index
		if (m_member_index > m_block_member_count - 1)
			m_member_index = 0;
	}

	char* src = (char*)p_datas[_index] + _member_offset;
	memcpy(src, _data, _member_size);

	return this;
}

void ThirdParty::SlotData::update()
{
	// UBO
	if(m_descriptor_type == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER) {
		auto& _binding_vector = p_uniform_buffer.second;
		uint32_t _data_index = 0;
		// 将数据绑定到ubo, 以数据为循环
		for (const auto& _data : p_datas) {
			// UBO 数量不够
			if (_binding_vector.size() <= _data_index) {
				// 如果数量不够则新建UBO
				UniformBuffer* p_ubo = new UniformBuffer(m_block_size, p_uniform_buffer.first);
				p_uniform_buffer.second.push_back(p_ubo);
				// 新建bufferInfo
				VkDescriptorBufferInfo _buffer_info;
				_buffer_info.buffer = p_ubo->getVkBuffer();
				_buffer_info.offset = 0;
				_buffer_info.range = VK_WHOLE_SIZE;
				std::get<std::vector<VkDescriptorBufferInfo>>(m_infos).push_back(_buffer_info);
			}
			auto& _write_info = std::get<std::vector<VkDescriptorBufferInfo>>(m_infos)[_data_index];
			_write_info.buffer = _binding_vector[_data_index]->getVkBuffer();
			_write_info.offset = 0;
			_write_info.range = VK_WHOLE_SIZE;
			_binding_vector[_data_index++]->bindData(_data, 0, m_block_size);
		}
	}
	else if (m_descriptor_type == VK_DESCRIPTOR_TYPE_SAMPLER) {
		auto& _binding_vector = p_texture_ext.second;
		uint32_t _data_index = 0;
		// 将Texture绑定到binding, 以Texture为循环
		for (const auto& _texture : p_texture_ext.second) {
			auto& _write_info = std::get<std::vector<VkDescriptorImageInfo>>(m_infos)[_data_index];
			_write_info.sampler = _texture->getVkSampler();
			_write_info.imageLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			_write_info.imageView = VK_NULL_HANDLE;
		}
	}
	else if (m_descriptor_type == VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE || 
		m_descriptor_type == VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER ||
		m_descriptor_type == VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT) {
		auto& _binding_vector = p_texture_ext.second;
		uint32_t _data_index = 0;
		// 将Texture绑定到binding, 以Texture为循环
		for (const auto& _texture : p_texture_ext.second) {
			auto& _write_info = std::get<std::vector<VkDescriptorImageInfo>>(m_infos)[_data_index++];
			if (_texture->getVkSampler() == VK_NULL_HANDLE) {
				auto _sampler = ThirdParty::Sampler::g_all_samplers[SAMPLER];
				_texture->setVkSampler(_sampler->getVkSampler());
			}
			_write_info.sampler = _texture->getVkSampler();
			_write_info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			_write_info.imageView = _texture->getVkImageView();
		}
	}
}

const VkWriteDescriptorSet& ThirdParty::SlotData::getVkWriteDescriptorSet(VkDescriptorSet _dst_set)
{
	if (p_write_infos.empty()) {
		p_write_infos.resize(m_vkInfo.m_frame_count);
		for (auto& _write : p_write_infos) {
			_write = new VkWriteDescriptorSet;
			_write->sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			_write->descriptorType = m_descriptor_type;
			_write->dstArrayElement = 0;
			_write->dstBinding = m_binding_id;
			_write->dstSet = _dst_set;
			_write->pNext = 0;
			_write->pBufferInfo = 0;
			_write->pImageInfo = 0;
			_write->pTexelBufferView = 0;
		}
	}

	auto& _write = p_write_infos[m_vkInfo.m_current_frame];
	_write->dstSet = _dst_set;

	switch (m_infos.index()) {
	case 0: // buffer
		_write->descriptorCount = std::get<std::vector<VkDescriptorBufferInfo>>(m_infos).size();
		_write->pBufferInfo = std::get<std::vector<VkDescriptorBufferInfo>>(m_infos).data();
		break;
	case 1: // image
		_write->descriptorCount = std::get<std::vector<VkDescriptorImageInfo>>(m_infos).size();
		_write->pImageInfo = std::get<std::vector<VkDescriptorImageInfo>>(m_infos).data();
		break;
	default: break;
	}

	return *_write;
}

bool ThirdParty::SlotData::isTexture()
{
	if (m_descriptor_type == VK_DESCRIPTOR_TYPE_SAMPLER) return true;
	if (m_descriptor_type == VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE) return true;
	if (m_descriptor_type == VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER) return true;
	if (m_descriptor_type == VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT) return true;
	return false;
}

bool ThirdParty::SlotData::isUniformBuffer()
{
	if (m_descriptor_type == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER) return true;
	return false;
}

bool ThirdParty::SlotData::isEmpty()
{
	return isTexture() && m_need_data;
}

bool ThirdParty::SlotData::isEmpty(bool _set_empty)
{
	m_need_data = _set_empty;
	return isTexture() && m_need_data;
}

bool ThirdParty::SlotData::isCube()
{
	return m_is_cube;
}

bool ThirdParty::SlotData::isChange()
{
	return m_texture_change_index.size() > 0;
}

std::set<uint32_t> ThirdParty::SlotData::getChangeIndex()
{
	auto _rel = m_texture_change_index; 
	m_texture_change_index.clear();
	return _rel;
}

void ThirdParty::SlotData::setup()
{
	auto& p_info = p_binding_property;
	// spv reflect type
	auto _spv_type = p_info->type_description;
	// binding block
	auto _binding_block = p_info->block;
	
	// set id
	size_t _set_id = p_info->set;
	// binding id
	size_t _binding_id = p_info->binding;
	// binding name
	std::string _binding_name = p_info->name ?
		p_info->name : "Binding_" + std::to_string(_set_id) + "_" + std::to_string(_binding_id);
	// binding type
	VkDescriptorType _binding_type = (VkDescriptorType)p_info->descriptor_type;
	if (_spv_type->op == SpvOp::SpvOpTypeArray || _spv_type->op == SpvOpTypeRuntimeArray) {
		m_is_array = true;
		m_binding_flags_ext = VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT | VK_DESCRIPTOR_BINDING_VARIABLE_DESCRIPTOR_COUNT_BIT_EXT;
	}
	// 如果是ubo ，则开始构建struct
	if (_binding_type == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER) {
		// ubo size
		size_t _binding_size = p_info->block.padded_size;
		// struct name
		std::string _binding_struct_name = _spv_type->type_name ? 
			_spv_type->type_name : "UniformBuffer_" + std::to_string(_set_id) + "_" + std::to_string(_binding_id);
		// 解析struct member
		for (size_t i = 0; i < _spv_type->member_count; i++) {
			// member prop
			auto _mem_prop = _spv_type->members[i];
			// member name
			std::string _member_name = _mem_prop.struct_member_name ? _mem_prop.struct_member_name : "Member_" + std::to_string(i);
			// member type
			std::string _type_name = getSpvOpType2String(_mem_prop.op);
			m_struct_type_name[_member_name] = _type_name;
			if (i == 0) {
				m_struct_property = { _binding_struct_name ,{} };
				m_struct_property_order = { _binding_struct_name,{} };
			}
			auto& _vec = m_struct_property.second;
			auto& _order = m_struct_property_order.second;
			_vec.insert(std::pair(_member_name, std::pair(_binding_block.members[i].size, _binding_block.members[i].offset)));
			// 保存结构顺序
			_order.insert(std::make_pair(i, _member_name));
			//m_block_size += _binding_block.members[i].size;	// 为了方便绑定数据，把实际大小设置为带对齐的大小
			m_block_size = _binding_block.padded_size;
			m_block_padding_size = _binding_block.padded_size;
		}
		p_uniform_buffer = { _binding_name,{} };	// 占位
		m_block_member_count = _binding_block.member_count;
	}
	else if (_binding_type == VK_DESCRIPTOR_TYPE_SAMPLER ||
		_binding_type == VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE ||
		_binding_type == VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER||
		_binding_type == VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT)
		p_texture_ext = { _binding_name,{} };	// 占位

	m_binding_name = _binding_name;
	m_binding_id = _binding_id;
	m_descriptor_type = _binding_type;
	m_set_layout_binding.binding = _binding_id;
	m_set_layout_binding.descriptorCount = m_is_array ? 8 : p_info->count;
	m_set_layout_binding.descriptorType = _binding_type;
	m_set_layout_binding.pImmutableSamplers = nullptr;
	m_set_layout_binding.stageFlags = 0;
}

void ThirdParty::SlotData::setupData()
{
	// 初始化 m_infos
	if (m_descriptor_type == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER)
		m_infos = std::vector<VkDescriptorBufferInfo>{};
	if (m_descriptor_type == VK_DESCRIPTOR_TYPE_SAMPLER ||
		m_descriptor_type == VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE ||
		m_descriptor_type == VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER||
		m_descriptor_type == VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT)
		m_infos = std::vector<VkDescriptorImageInfo>{};
	// 实例化，如果不是Array,则实例化为1
	if (!m_is_array) {
		// 实例化UBO 空槽
		if (m_descriptor_type == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER) {
			auto& _binding_name = p_uniform_buffer.first;
			auto& _binding_vector = p_uniform_buffer.second;
			UniformBuffer* _ubo = new UniformBuffer(m_block_size, _binding_name);
			_binding_vector.push_back(_ubo);
			// 新建bufferInfo
			VkDescriptorBufferInfo _buffer_info;
			_buffer_info.buffer = _ubo->getVkBuffer();
			_buffer_info.offset = 0;
			_buffer_info.range = VK_WHOLE_SIZE;
			std::get<std::vector<VkDescriptorBufferInfo>>(m_infos).push_back(_buffer_info);
		}
		// 实例化TextureEXT 空槽, 类型为Image2D ,剔除Cube类型
		if (m_descriptor_type == VK_DESCRIPTOR_TYPE_SAMPLER ||
			m_descriptor_type == VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE ||
			m_descriptor_type == VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER||
			m_descriptor_type == VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT) {
			auto& _binding_name = p_texture_ext.first;
			auto& _binding_vector = p_texture_ext.second;

			auto& p_info = p_binding_property;
			TextureEXT* _texture_ext = nullptr;
			// 立方体贴图
			if (p_info->image.dim == SpvDimCube) {
				m_is_cube = true;
				_texture_ext = TextureEXT::getTextureEXT(TEXTURE_SKYBOX_DEFAULT);
			}
			else if (p_info->image.dim == SpvDim2D) {
				_texture_ext = TextureEXT::getTextureEXT(TEXTURE_DEFAULT);
				// 设置槽为待插入状态
				m_need_data = true;
			}
			if (_texture_ext) {
				_binding_vector.push_back(_texture_ext);
				// 新建bufferInfo
				VkDescriptorImageInfo _image_info;
				_image_info.sampler = _texture_ext->getVkSampler();
				_image_info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
				_image_info.imageView = _texture_ext->getVkImageView();
				std::get<std::vector<VkDescriptorImageInfo>>(m_infos).push_back(_image_info);
			}
		}
	}
}

std::string ThirdParty::SlotData::getSpvOpType2String(SpvOp _OpType)
{
	switch (_OpType)
	{
	case SpvOpTypeFloat: return "float";
	case SpvOpTypeBool: return "bool";
	case SpvOpTypeInt: return "int";
	default: return "none";
	}
}

const std::pair<std::string, std::vector<std::pair<std::string, std::pair<size_t, size_t>>>> ThirdParty::SlotData::getMemberStructOrdered()
{
	std::pair<std::string, std::vector<std::pair<std::string, std::pair<size_t, size_t>>>> _rel;
	_rel.first = m_struct_property_order.first;
	auto& _ite = _rel.second;
	for (auto _mem_ite : m_struct_property_order.second) {
		auto _struct_prop_ite = m_struct_property.second.find(_mem_ite.second);
		_ite.push_back(std::make_pair((*_struct_prop_ite).first, (*_struct_prop_ite).second));
	}
	return _rel;
}

const uint32_t ThirdParty::SlotData::getBindingCount()
{
	auto _size_0 = p_datas.size();
	auto _size_1 = p_texture_ext.second.size();
	return std::max(_size_0, _size_1);
}
