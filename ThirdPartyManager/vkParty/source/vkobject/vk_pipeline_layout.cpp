#include "vkobject/vk_pipeline_layout.h"

ThirdParty::PipelineLayout::PipelineLayout(const std::string &_label)
	:vkObject(_label)
{
	if (!p_create_info) {
		p_create_info = new VkPipelineLayoutCreateInfo;
		p_create_info->sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		p_create_info->flags = 0;
		p_create_info->pNext = 0;
	}
	initTemplate();
}

ThirdParty::PipelineLayout::PipelineLayout(const std::vector<VkDescriptorSetLayout>& _layouts, const std::vector<VkPushConstantRange>& _push_constants /*= {}*/, const std::string& _label /*= "Pipeline Layout"*/)
	:vkObject(_label),m_descriptor_set_layouts(_layouts),m_push_constant_ranges(_push_constants)
{
	if (!p_create_info) {
		p_create_info = new VkPipelineLayoutCreateInfo;
		p_create_info->sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		p_create_info->flags = 0;
		p_create_info->pNext = 0;
	}

	p_create_info->setLayoutCount = static_cast<uint32_t>(m_descriptor_set_layouts.size());
	p_create_info->pSetLayouts = m_descriptor_set_layouts.data();
	p_create_info->pushConstantRangeCount = static_cast<uint32_t>(m_push_constant_ranges.size());
	p_create_info->pPushConstantRanges = m_push_constant_ranges.data();
	m_result = vkCreatePipelineLayout(m_vkInfo.m_device, p_create_info, m_vkInfo.p_allocate, &m_pipeline_layout);
	if(checkVkResult(m_result, ("Failed to create pipeline layout:" + _label).c_str()))
		initTemplate();
}

ThirdParty::PipelineLayout::~PipelineLayout()
{
	if (p_create_info) delete p_create_info;
	if (m_pipeline_layout != VK_NULL_HANDLE) {
		vkDestroyPipelineLayout(m_vkInfo.m_device, m_pipeline_layout, m_vkInfo.p_allocate);
		m_pipeline_layout = VK_NULL_HANDLE;
	}
}

void ThirdParty::PipelineLayout::initTemplate() {
	vkObject::initTemplate();
	{
		auto _input_0 = new ThirdParty::imgui::SlotTemplateInput;
		_input_0->setName("DescriptorSet Layout");
		auto _input_1 = new ThirdParty::imgui::SlotTemplateInput;
		_input_1->setName("Push ConstantRange");

		auto _output_0 = new ThirdParty::imgui::SlotTemplateOutput;
		_output_0->setName("Pipeline Layout");
		_output_0->p_data = &m_pipeline_layout;

		p_template->addSlotTemplates({_input_0,_input_1},true);
		p_template->addSlotTemplates({_output_0},false);
	}
}
