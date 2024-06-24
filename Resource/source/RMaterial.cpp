#include "RMaterial.h"
#include <Core/include/core_util_function.h>
#include <Core/include/core_global.h>
#include <RTexture.h>
#include <ThirdPartyManager/third_party_manager_global.h>
Core::Resource::RMaterial::RMaterial(const std::string& _name /*= "RMaterial"*/)
{
	this->m_res_name = _name;
	this->m_json_title = "RMaterial";
	this->p_id_generate->init(RESOURCE_TYPE::RMATERIAL);
	this->m_uuid = this->p_id_generate->generateUUID();

	Core::Core_Pool::addResource(m_uuid, this);
}

Core::Resource::RMaterial::~RMaterial()
{
	for (const auto& _slot : p_slots)
		delete _slot.second;
}

Json::Value Core::Resource::RMaterial::serializeToJSON()
{
	return Json::Value();
}

void Core::Resource::RMaterial::deserializeToObj(Json::Value& _root)
{

}

Core::Resource::RMaterial* Core::Resource::RMaterial::bind(VkCommandBuffer _cmd, bool _update_solt)
{
	// ��ȡ��ǰdescriptorSet
	m_descriptor_sets_current_frame.clear();
	for (const auto& [_set,_slot] : p_slots) {
		if (_update_solt)
			_slot->update(_cmd);	// �ŵ��ⲿ���£����Mesh����һ��descriptorʱ��������ڲ����»���ʾset�ѱ����µ�����
		m_descriptor_sets_current_frame.push_back(_slot->getVkDescriptorSet());
	}
	if (p_shader) {
		static void* _p_pipeline_current = nullptr;
		const auto& _p_pipeline = Core::Resource::Pool::getThirdParty_Pipeline(p_shader->getName());
		// ��ѯ����ɫ���Ƿ񴴽�����
		if (_p_pipeline) {
			_p_pipeline_current = _p_pipeline;
			p_shader->bind(_cmd, _p_pipeline->getVkPipeline(), m_descriptor_sets_current_frame);
		}
		else if(!_p_pipeline) {
			Core::error("Pipeline has not been created:%s", p_shader->getFileName().c_str());
			return nullptr;
		}
	}
	return this;
}

void Core::Resource::RMaterial::setup(bool _reload)
{
	if (p_shader_old != p_shader || _reload) {
		// ��ȡshader input stride ��flag ��RModelʹ��
		const auto& _attri = p_shader->getVkVertexInputAttributeDescription();
		m_vertex_input_stride = 0;
		int location = 1;
		int input_flag = 0;
		for (const auto& _binding : _attri) {
			auto _location = _binding.location + 1;
			m_vertex_input_stride += ThirdParty::getFormatSize(_binding.format);
			input_flag += _location * ThirdParty::getFormatType(_binding.format);
		}
		m_vertex_input_flag = (VERTEX_INPUT_FLAG)input_flag;

		// Slots
		{
			// ��������
			if (!p_slots.empty()) {
				for (const auto& _slot : p_slots) delete _slot.second;
				p_slots.clear();
			}
			for (const auto& _slot : p_shader->getSlots())
			{
				ThirdParty::Slot* p_slot = new ThirdParty::Slot(_slot);
				p_slots[p_slot->getSetID()] = p_slot;
			}
		}

		// Slot Data
		if(_reload){
			bindTextures(p_reload_textures);
		}
		m_initialized = true;
	}
	else m_initialized = true;
}

Core::Resource::RMaterial* Core::Resource::RMaterial::setShader(Shader* _shader)
{
	p_shader_old = p_shader;
	p_shader = _shader;
	m_initialized = false;
	setup(true);
	return this;
}

size_t Core::Resource::RMaterial::getVertexInputStride()
{
	return m_vertex_input_stride;
}

Core::Resource::VERTEX_INPUT_FLAG Core::Resource::RMaterial::getVertexInputFlag()
{
	return m_vertex_input_flag;
}

Core::Resource::RMaterial* Core::Resource::RMaterial::bindData(uint32_t _set_id, uint32_t _binding_id, uint32_t _member_id, void* _data, uint32_t _index /*= 0*/)
{
	// SLOT
	auto _slot = p_slots[_set_id];
	// SLOT DATA
	auto _slot_data = _slot->getSlotData(_binding_id);
	// ��������
	_slot_data->bindData(_member_id, _data, _index);
	return this;
}

Core::Resource::RMaterial* Core::Resource::RMaterial::bindData(uint32_t _set_id, const std::string& _binding_name, uint32_t _member_id, void* _data, uint32_t _index /*= 0*/)
{
	// SLOT
	auto _slot = p_slots[_set_id];
	// SLOT DATA
	auto _slot_data = _slot->getSlotData(_binding_name);
	// ��������
	_slot_data->bindData(_member_id, _data, _index);
	return this;
}

Core::Resource::RMaterial* Core::Resource::RMaterial::bindData(uint32_t _set_id, uint32_t _binding_id, std::string _member_name, void* _data, uint32_t _index /*= 0*/)
{
	// SLOT
	auto _slot = p_slots[_set_id];
	// SLOT DATA
	auto _slot_data = _slot->getSlotData(_binding_id);
	// ��������
	_slot_data->bindData(_member_name, _data, _index);
	return this;
}

Core::Resource::RMaterial* Core::Resource::RMaterial::bindData(uint32_t _set_id, uint32_t _binding_id, void* _data, uint32_t _index)
{
	// SLOT
	auto _slot = p_slots[_set_id];
	// SLOT DATA
	auto _slot_data = _slot->getSlotData(_binding_id);
	// ��������
	_slot_data->bindData(_data, _index);
	return this;
}

Core::Resource::RMaterial* Core::Resource::RMaterial::bindData(uint32_t _set_id, uint32_t _binding_id, VkSampler _sampler, uint32_t index /*= 0*/)
{
	// SLOT
	auto _slot = p_slots[_set_id];
	// SLOT DATA
	auto _slot_data = _slot->getSlotData(_binding_id);
	// ��������
	_slot_data->bindData(_sampler, index);
	return this;
}

Core::Resource::RMaterial* Core::Resource::RMaterial::bindTexture(uint32_t _set_id, uint32_t _binding_id, ThirdParty::TextureEXT* _tex, uint32_t index /*= 0*/)
{
	// SLOT
	auto _slot = p_slots[_set_id];
	// SLOT DATA
	auto _slot_data = _slot->getSlotData(_binding_id);
	// ��������
	_slot_data->bindData(_tex, index);
	return this;
}

void Core::Resource::RMaterial::initTemplate()
{
}

ThirdParty::Sampler* Core::Resource::RMaterial::getSampler(const std::string& _name /*= SAMPLER*/)
{
	return (ThirdParty::Sampler*)Core::Core_Pool::g_all_samplers[_name];
}

Core::Resource::RMaterial* Core::Resource::RMaterial::bindTextures(const std::vector<RTexture*>& _texs)
{
	if (_texs.size() == 0) return this;
	p_reload_textures = _texs;
	// �����������SlotData��
	int index = 0;
	for (const auto& [_set_id,_slot] : p_slots) {
		for (const auto& [_slot_name, _slot_data] : _slot->getSlotDatas()) {
			// �жϵ�ǰ����Ƿ���ҪTexture
			if (_slot_data->isTexture()) {
				if (index >= _texs.size()) continue;
				// ����Ѿ����ع���������ֱ��ʹ��
				auto _t = _texs[index]->getPath();
				auto _loaded = ThirdParty::TextureEXT::getTextureEXT(_t);
				// �Ѿ�����
				if (_loaded != nullptr) {
					auto _ptr = _slot_data->bindData(_loaded, index);
					_slot_data->bindData(getSampler()->getVkSampler(), index);	// ����sampler
					// �����Sampler�������ĵ�ǰTexture
					if (_ptr != nullptr)
						index++;
				}
				// δ���ع�
				else {
					auto _ptr = _slot_data->bindData(_t, index);
					_slot_data->bindData(getSampler()->getVkSampler(), index);	// ����sampler
					// �����Sampler�������ĵ�ǰTexture
					if (_ptr != nullptr)
						index++;
				}
			}
		}
	}
	// ��¼��ǰMaterial ��������
	m_texture_count = index;
	return this;
}

Core::Resource::RMaterial* Core::Resource::RMaterial::reload()
{
	if (p_shader->reload(true)) {
		setup(true);
	}
	return this;
}

