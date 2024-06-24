#include <cstring>
#include "vkobject/vk_descriptor_set.h"


ThirdParty::DescriptorSet* ThirdParty::DescriptorSet::bindData(const size_t& _binding, const VkDescriptorType& _type, const std::variant<std::vector<VkDescriptorBufferInfo>, std::vector<VkDescriptorImageInfo>>& _infos)
{
	const auto _index = m_vkInfo.m_current_frame;
	// 如果当前Binding 为设置，则进行初始化
	auto _set_info = m_infos.find(_binding);
	if (_set_info == m_infos.end()) {
		m_infos[_binding] = {};
		m_infos[_binding].resize(m_vkInfo.m_frame_count);
		_set_info = m_infos.find(_binding);
	}
	auto _write_info = m_write_info.find(_binding);
	if (_write_info == m_write_info.end()) {
		m_write_info[_binding] = {};
		m_write_info[_binding].resize(m_vkInfo.m_frame_count);
		_write_info = m_write_info.find(_binding);
		for (auto& _w : _write_info->second) {
			_w.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			_w.pNext = 0;
			_w.dstArrayElement = 0;
			_w.pTexelBufferView = 0;

			_w.dstBinding = _binding;
			_w.descriptorType = _type;
		}
	}

	// 设置每帧的write info信息并更新set
	_set_info->second[_index] = _infos;
	auto& _i = _set_info->second[_index];
	auto& _w = _write_info->second[_index];
	// 设置
	_w.dstSet = m_descriptor_sets[_index];
	{
		switch (_i.index())
		{
			// Buffer
		case 0:
			_w.descriptorCount = std::get<std::vector<VkDescriptorBufferInfo>>(_infos).size();
			_w.pBufferInfo = std::get<std::vector<VkDescriptorBufferInfo>>(_infos).data();
			_w.pImageInfo = nullptr;
			break;
		case 1:
			_w.descriptorCount = std::get<std::vector<VkDescriptorImageInfo>>(_infos).size();
			_w.pImageInfo = std::get<std::vector<VkDescriptorImageInfo>>(_infos).data();
			_w.pBufferInfo = nullptr;
			break;
		default:
			break;
		}
	}
	vkUpdateDescriptorSets(m_vkInfo.m_device, 1, &_w, 0, 0);
	return this;
}

ThirdParty::DescriptorSet::DescriptorSet(VkDescriptorSetLayout _layout, const std::string& _label)
	:vkObject(_label)
{
	if (!p_alloc_info) {
		p_alloc_info = new VkDescriptorSetAllocateInfo;
		p_alloc_info->sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		p_alloc_info->descriptorPool = m_vkInfo.m_descriptor_pool;
		p_alloc_info->pNext = 0;
	}
	VkDescriptorSetVariableDescriptorCountAllocateInfo count_info = { VK_STRUCTURE_TYPE_DESCRIPTOR_SET_VARIABLE_DESCRIPTOR_COUNT_ALLOCATE_INFO };
	std::vector<uint32_t> counts;
	if (m_vkInfo.m_bindless) {
		counts.resize(m_vkInfo.m_frame_count);
		memset(counts.data(), m_vkInfo.m_max_ubo_descriptor_count, m_vkInfo.m_frame_count);
		count_info.descriptorSetCount = m_vkInfo.m_frame_count;
		count_info.pDescriptorCounts = counts.data();
		p_alloc_info->pNext = &count_info;
	}
	m_descriptor_sets.resize(m_vkInfo.m_frame_count);
	std::vector<VkDescriptorSetLayout> _layouts(m_vkInfo.m_frame_count, _layout);
	p_alloc_info->descriptorSetCount = m_vkInfo.m_frame_count;
	p_alloc_info->pSetLayouts = _layouts.data();
	m_result = vkAllocateDescriptorSets(m_vkInfo.m_device, p_alloc_info, m_descriptor_sets.data());
	checkVkResult(m_result, "Failed to create VkDescriptorSet");
}

ThirdParty::DescriptorSet::DescriptorSet(VkDescriptorSetLayout _layout, uint32_t _descriptorCount)
{
	m_label = "DescriptorSet";
	if (!p_alloc_info) {
		p_alloc_info = new VkDescriptorSetAllocateInfo;
		p_alloc_info->sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		p_alloc_info->descriptorPool = m_vkInfo.m_descriptor_pool;
		p_alloc_info->pNext = 0;
	}
	VkDescriptorSetVariableDescriptorCountAllocateInfo count_info ={ VK_STRUCTURE_TYPE_DESCRIPTOR_SET_VARIABLE_DESCRIPTOR_COUNT_ALLOCATE_INFO };
	std::vector<uint32_t> counts;
	if (m_vkInfo.m_bindless) {
		counts.resize(m_vkInfo.m_frame_count);
		std::fill_n(counts.begin(), m_vkInfo.m_frame_count, _descriptorCount);
		count_info.descriptorSetCount = m_vkInfo.m_frame_count;
		count_info.pDescriptorCounts = counts.data();
		p_alloc_info->pNext = &count_info;
	}
	m_descriptor_sets.resize(m_vkInfo.m_frame_count);
	std::vector<VkDescriptorSetLayout> _layouts(m_vkInfo.m_frame_count, _layout);
	p_alloc_info->descriptorSetCount = m_vkInfo.m_frame_count;
	p_alloc_info->pSetLayouts = _layouts.data();
	m_result = vkAllocateDescriptorSets(m_vkInfo.m_device, p_alloc_info, m_descriptor_sets.data());
	checkVkResult(m_result, "Failed to create VkDescriptorSet");

}

ThirdParty::DescriptorSet::~DescriptorSet()
{
	if (p_alloc_info) delete p_alloc_info;
	if (!m_descriptor_sets.empty())
		vkFreeDescriptorSets(m_vkInfo.m_device, m_vkInfo.m_descriptor_pool, m_descriptor_sets.size(), m_descriptor_sets.data());
}
