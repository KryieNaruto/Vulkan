#include "vkobject/vk_slot.h"
#include "vk_party_manager.h"
#include <algorithm>
ThirdParty::Slot::Slot(SpvReflectDescriptorSet* _reflect_info)
	:p_reflect_descriptor_set_info(_reflect_info)
{
	setupSpvReflect();
	setupBindingProperty();
	//setupSetLayout();
}

ThirdParty::Slot::Slot(Slot* _slot)
{
	m_is_copy = true;
	// 复制构造
	{
		this->m_set_id = _slot->m_set_id;
		this->m_descriptor_layout = _slot->m_descriptor_layout;
		this->m_has_array = _slot->m_has_array;
	}

	for (const auto& _slot_data_ite : _slot->p_slot_datas) {
		auto _binding_name = _slot_data_ite.first;
		auto _slot_data = _slot_data_ite.second;
		// 创建Slot Data副本
		auto _slot_data_ = new SlotData(_slot_data);
		p_slot_datas[_binding_name] = _slot_data_;
		p_binding_name[_slot_data->getBinding()] = _binding_name;
	}
	// 创建set
	setupSet();
}

ThirdParty::Slot::~Slot()
{
	if (!m_sets.empty()) {
		vkDeviceWaitIdle(m_vkInfo.m_device);
		vkFreeDescriptorSets(m_vkInfo.m_device, m_vkInfo.m_descriptor_pool, m_sets.size(), m_sets.data());
		m_sets.clear();
	}
	if (p_create_info)
		delete p_create_info;
	if (p_setLayoutBindingFlags)
		delete p_setLayoutBindingFlags;
	if (m_descriptor_layout != VK_NULL_HANDLE && !m_is_copy)
		ThirdParty::vkManager.destroyVkObject(m_descriptor_layout);
	for (const auto& _slot_data : p_slot_datas)
		delete _slot_data.second;
}

void ThirdParty::Slot::update(VkCommandBuffer _cmd)
{
	if (m_sets.empty()) return;
	std::vector<VkWriteDescriptorSet> _write_infos;
	for (const auto& [_binding_name, _binding] : p_slot_datas) {
		// 更新Descriptor 数据
		_binding->update();
		// 如果当前_binding 是Array, 则判断variable count是否符合数量
		if (_binding->isArray() && (_binding->getBindingCount() > m_max_descriptor_count)) {
			m_max_descriptor_count = _binding->getBindingCount();
			setupSet();		// 重建DescriptorSet
			update(_cmd);	// 重新更新数据
			return;
		}
		auto _write_info = _binding->getVkWriteDescriptorSet(m_sets[m_vkInfo.m_current_frame]);
		if (_write_info.descriptorCount > 0)
			_write_infos.push_back(_write_info);
	}

	// 更新Set
	vkUpdateDescriptorSets(m_vkInfo.m_device, _write_infos.size(), _write_infos.data(), 0, 0);
}

ThirdParty::Slot* ThirdParty::Slot::setShaderStage(uint32_t _binding, VkShaderStageFlags _stage)
{
	p_slot_datas[p_binding_name[_binding]]->setShaderStage(_stage);
	return this;
}

ThirdParty::Slot* ThirdParty::Slot::addSlotData(SpvReflectDescriptorBinding* _binding_info)
{
	auto _binding_index = _binding_info->binding;
	SlotData* p_data = new SlotData(_binding_info);
	p_binding_name[_binding_index] = p_data->getBindingName();
	p_slot_datas[p_binding_name[_binding_index]] = p_data;
	// 获取SetLayoutBinding
	m_descriptor_set_layout_bindings.push_back(p_data->getVkDescriptorSetLayoutBinding());
	// 获取BindingFlagsEXT
	m_descriptor_binding_flags_ext.push_back(p_data->getVkDescriptorBindingFlagsEXT());
	// 是否是数组SlotData
	m_has_array = m_has_array | p_data->isArray();

	return this;
}

ThirdParty::Slot* ThirdParty::Slot::recreateDescriptorSetLayout()
{
	auto& _bindings = m_descriptor_set_layout_bindings;
	auto& _binding_flags = m_descriptor_binding_flags_ext;

	if (m_descriptor_layout != VK_NULL_HANDLE) {
		vkDeviceWaitIdle(m_vkInfo.m_device);
		ThirdParty::vkManager.destroyVkObject(m_descriptor_layout);
	}
	if (!_bindings.empty()) {
		_bindings.clear();
		_binding_flags.clear();
		for (const auto& [_name,_slot_data]: p_slot_datas) {
			_bindings.push_back(_slot_data->getVkDescriptorSetLayoutBinding());
			_binding_flags.push_back(_slot_data->getVkDescriptorBindingFlagsEXT());
			m_has_array = m_has_array | _slot_data->isArray();
		}

	}

	if (!p_create_info)
		p_create_info = new VkDescriptorSetLayoutCreateInfo;
	if (!p_setLayoutBindingFlags)
		p_setLayoutBindingFlags = new VkDescriptorSetLayoutBindingFlagsCreateInfoEXT;
	// setLayout create info
	{
		p_create_info->sType = { VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO };
		p_create_info->bindingCount = _bindings.size();
		p_create_info->pBindings = _bindings.data();
		p_create_info->pNext = 0;
		p_create_info->flags = 0;
	};
	// 开启bindless
	if (m_has_array && m_vkInfo.m_bindless) {
		p_setLayoutBindingFlags->sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO_EXT;
		p_setLayoutBindingFlags->bindingCount = _binding_flags.size();
		p_setLayoutBindingFlags->pBindingFlags = _binding_flags.data();
		p_setLayoutBindingFlags->pNext = 0;
		p_create_info->pNext = p_setLayoutBindingFlags;
	}

	m_descriptor_layout = ThirdParty::vkManager.createDescriptorSetLayout(*p_create_info);

	return this;
}

ThirdParty::Slot* ThirdParty::Slot::createDescriptorSetLayout()
{
	return recreateDescriptorSetLayout();
}

std::set<uint32_t> ThirdParty::Slot::getBindingIndices()
{
	std::set<uint32_t> _rel;

	for (const auto& [_id, _name] : p_binding_name)
		_rel.insert(_id);

	return _rel;
}

ThirdParty::Slot* ThirdParty::Slot::setShaderStage(std::string _binding_name, VkShaderStageFlags _stage)
{
	p_slot_datas[_binding_name]->setShaderStage(_stage);
	setupSetLayout();
	return this;
}

void ThirdParty::Slot::setupSpvReflect()
{
	auto& p_info = p_reflect_descriptor_set_info;
	
	// 获取SetID
	uint32_t _set_id = p_info->set;
	// 循环获取binding 属性
	for (size_t i = 0; i < p_info->binding_count; i++) {
		// 当前binding 属性
		auto _binding_prop = p_info->bindings[i];
		// binding id
		size_t _binding_id = _binding_prop->binding;
		// binding name
		std::string _binding_name = _binding_prop->name?_binding_prop->name: "Binding_" + std::to_string(_binding_id);
		p_binding_properties[_binding_name] = _binding_prop;
	}

	m_set_id = p_info->set;
}

void ThirdParty::Slot::setupBindingProperty()
{
	// 求出所有插槽数量
	int count = 0;
	for (const auto& _binding_prop_ite : p_binding_properties) {
		count += _binding_prop_ite.second->count;
	}
	for (const auto& _binding_prop_ite : p_binding_properties) {
		auto _binding_name = _binding_prop_ite.first;
		auto _binding_prop = _binding_prop_ite.second;

		SlotData* p_data = new SlotData(_binding_prop);
		p_slot_datas[_binding_name] = p_data;
		p_binding_name[_binding_prop->binding] = _binding_name;
		// 获取SetLayoutBinding
		m_descriptor_set_layout_bindings.push_back(p_data->getVkDescriptorSetLayoutBinding());
		// 获取BindingFlagsEXT
		m_descriptor_binding_flags_ext.push_back(p_data->getVkDescriptorBindingFlagsEXT());
		// 是否是数组SlotData
		m_has_array = m_has_array | p_data->isArray();
	}
}

void ThirdParty::Slot::setupSetLayout()
{
	if (m_descriptor_layout != VK_NULL_HANDLE) {
		vkDeviceWaitIdle(m_vkInfo.m_device);
		ThirdParty::vkManager.destroyVkObject(m_descriptor_layout);
	}

	auto& _bindings = m_descriptor_set_layout_bindings;
	auto& _binding_flags = m_descriptor_binding_flags_ext;

	if(!p_create_info)
		p_create_info = new VkDescriptorSetLayoutCreateInfo;
	if (!p_setLayoutBindingFlags)
		p_setLayoutBindingFlags = new VkDescriptorSetLayoutBindingFlagsCreateInfoEXT;
	// setLayout create info
	{
		p_create_info->sType = { VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO };
		p_create_info->bindingCount = _bindings.size();
		p_create_info->pBindings = _bindings.data();
		p_create_info->pNext = 0;
		p_create_info->flags = 0;
	};
	// 开启bindless
	if (m_has_array && m_vkInfo.m_bindless) {
		p_setLayoutBindingFlags->sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO_EXT;
		p_setLayoutBindingFlags->bindingCount = _binding_flags.size();
		p_setLayoutBindingFlags->pBindingFlags = _binding_flags.data();
		p_setLayoutBindingFlags->pNext = 0;
		p_create_info->pNext = p_setLayoutBindingFlags;
	}

	m_descriptor_layout = ThirdParty::vkManager.createDescriptorSetLayout(*p_create_info);
}

void ThirdParty::Slot::setupSet()
{
	if (!m_sets.empty()) { 
		vkDeviceWaitIdle(m_vkInfo.m_device);
		vkFreeDescriptorSets(m_vkInfo.m_device, m_vkInfo.m_descriptor_pool, m_sets.size(), m_sets.data());
		m_sets.clear();
	}

	m_sets.resize(m_vkInfo.m_frame_count);
	// 分配
	{
		std::vector<VkDescriptorSetLayout> _layouts(m_sets.size(), m_descriptor_layout);
		VkDescriptorSetVariableDescriptorCountAllocateInfo _variable_alloc_info;
		VkDescriptorSetAllocateInfo _alloc = { VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO };
		std::vector<uint32_t> _count(m_sets.size(), m_max_descriptor_count);
		_alloc.descriptorPool = m_vkInfo.m_descriptor_pool;
		_alloc.descriptorSetCount = m_sets.size();
		_alloc.pSetLayouts = _layouts.data();
		if (m_has_array) {
			_variable_alloc_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_VARIABLE_DESCRIPTOR_COUNT_ALLOCATE_INFO;
			_variable_alloc_info.descriptorSetCount = _count.size();
			_variable_alloc_info.pDescriptorCounts = _count.data();
			_variable_alloc_info.pNext = 0;
			_alloc.pNext = &_variable_alloc_info;
		}
		else { 
			_alloc.pNext = 0; 
		}

		m_result = vkAllocateDescriptorSets(m_vkInfo.m_device, &_alloc, m_sets.data());
		checkVkResult(m_result, "Failed to create Slot Descriptor Set!");
	}

}
