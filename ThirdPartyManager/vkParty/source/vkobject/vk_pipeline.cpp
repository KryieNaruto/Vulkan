#include "vkobject/vk_pipeline.h"

ThirdParty::Pipeline::Pipeline(const std::string& _label)
	:vkObject(_label)
{
	p_create_info = nullptr;
	p_input_assembly_state_info = nullptr;
	p_multisample_satae_info = nullptr;
	p_raster_state_info = nullptr;
	p_vertex_input_state_info = nullptr;
	p_viewport_state_info = nullptr;
	p_dynamic_state_info = nullptr;
	p_color_blend_state_info = nullptr;
	p_depth_stencil_state_info = nullptr;
	if (!p_create_info) {
		p_create_info = new VkGraphicsPipelineCreateInfo;
		p_create_info->sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
		p_create_info->pNext = nullptr;
		p_create_info->flags = 0;
	}

	// ÉèÖÃTemplate
	if(!p_template)
		p_template = ThirdParty::imgui::Template::newTemplate();
	{
		// Input
		auto _input_0 = new imgui::SlotTemplateInput;
		_input_0->setName("Shader Stage");
		auto _input_1 = new imgui::SlotTemplateInput;
		_input_1->setName("Render Pass");
		auto _input_2 = new imgui::SlotTemplateInput;
		_input_2->setName("Pipeline Layout");
		// Output
		auto _output_0 = new imgui::SlotTemplateOutput;
		_output_0->setName("Pipeline");

		p_template->addSlotTemplates({_input_0,_input_1,_input_2},true);
		p_template->addSlotTemplates({_output_0},false);
	}
}

ThirdParty::Pipeline::~Pipeline()
{
	if (p_create_info) delete p_create_info;
	if (m_pipeline != VK_NULL_HANDLE) vkDestroyPipeline(m_vkInfo.m_device, m_pipeline, m_vkInfo.p_allocate);
}

ThirdParty::Pipeline* ThirdParty::Pipeline::addVertexInputBinding(const VkVertexInputBindingDescription& _binding, const VkVertexInputAttributeDescription& _attribute)
{
	m_bindings.push_back(_binding);
	m_attributes.push_back(_attribute);
	return this;
}

ThirdParty::Pipeline* ThirdParty::Pipeline::setVertexInputBinding(const std::vector<VkVertexInputBindingDescription>& _bindings /*= {}*/, const std::vector<VkVertexInputAttributeDescription>& _attributes /*= {}*/)
{
	if (!p_vertex_input_state_info) {
		p_vertex_input_state_info = new VkPipelineVertexInputStateCreateInfo;
		p_vertex_input_state_info->sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
		p_vertex_input_state_info->pNext = NULL;
		p_vertex_input_state_info->flags = 0;
	}
	if (!_bindings.empty())
		this->m_bindings = _bindings;
	if(!_attributes.empty())
		this->m_attributes = _attributes;
	// ÅÅ³ýstride = 0 ;
	if (m_bindings.size() == 1) {
		if (m_bindings[0].stride == 0)
			m_bindings.pop_back();
	}
	p_vertex_input_state_info->vertexBindingDescriptionCount = static_cast<uint32_t>(m_bindings.size());
	p_vertex_input_state_info->pVertexBindingDescriptions = m_bindings.data();
	p_vertex_input_state_info->vertexAttributeDescriptionCount = static_cast<uint32_t>(m_attributes.size());
	p_vertex_input_state_info->pVertexAttributeDescriptions = m_attributes.data();
	m_flags[VERTEX_INPUT] = true;
	return this;
}

ThirdParty::Pipeline* ThirdParty::Pipeline::setInputAssembly(VkPrimitiveTopology _topology /*= VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP*/, VkBool32 _primitiveRestartEnable /*= VK_FALSE*/)
{
	if (!p_input_assembly_state_info) {
		p_input_assembly_state_info = new VkPipelineInputAssemblyStateCreateInfo;
		p_input_assembly_state_info->sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
		p_input_assembly_state_info->pNext = nullptr;
		p_input_assembly_state_info->flags = 0;
	}
	p_input_assembly_state_info->topology = _topology;
	p_input_assembly_state_info->primitiveRestartEnable = _primitiveRestartEnable;
	m_flags[INPUT_ASSEMBLY] = true;
	return this;
}

ThirdParty::Pipeline* ThirdParty::Pipeline::setViewportAndScissor(const VkViewport& _viewport, const VkRect2D& _scissor)
{
	if (!p_viewport_state_info) {
		p_viewport_state_info = new VkPipelineViewportStateCreateInfo;
		p_viewport_state_info->sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
		p_viewport_state_info->pNext = nullptr;
		p_viewport_state_info->flags = 0;
	}
	m_viewports.push_back(_viewport);
	m_scissors.push_back(_scissor);
	p_viewport_state_info->viewportCount = static_cast<uint32_t>(m_viewports.size());
	p_viewport_state_info->pViewports = m_viewports.data();
	p_viewport_state_info->scissorCount = static_cast<uint32_t>(m_scissors.size());
	p_viewport_state_info->pScissors = m_scissors.data();
	m_flags[VIEWPORT] = true;
	return this;
}

ThirdParty::Pipeline* ThirdParty::Pipeline::setRasterization(VkPolygonMode _polygonMode,VkCullModeFlags _cullMode /*= VK_CULL_MODE_BACK_BIT*/, VkFrontFace _frontFace /*= VK_FRONT_FACE_CLOCKWISE*/, float _lineWidth /*= 1.0f*/, VkBool32 _depthClampEnable /*= VK_FALSE*/, VkBool32 _rasterizerDisardEnable /*= VK_FALSE*/, VkBool32 _depthBiasEnable /*= VK_FALSE*/, const std::vector<float>& _depthBias /*= {0.0f,0.0f,0.0f}*/)
{
	if (!p_raster_state_info) {
		p_raster_state_info = new VkPipelineRasterizationStateCreateInfo;
		p_raster_state_info->sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
		p_raster_state_info->pNext = nullptr;
		p_raster_state_info->flags = 0;
	}
	p_raster_state_info->depthBiasClamp = _depthClampEnable;
	p_raster_state_info->rasterizerDiscardEnable = _rasterizerDisardEnable;
	p_raster_state_info->lineWidth = _lineWidth;
	p_raster_state_info->polygonMode = _polygonMode;
	p_raster_state_info->cullMode = _cullMode;
	p_raster_state_info->frontFace = _frontFace;
	p_raster_state_info->depthBiasClamp = _depthBiasEnable;
	p_raster_state_info->depthBiasConstantFactor = _depthBias[0];
	p_raster_state_info->depthBiasClamp = _depthBias[1];
	p_raster_state_info->depthBiasSlopeFactor = _depthBias[2];
	m_flags[RASTERIZATION] = true;
	return this;
}

ThirdParty::Pipeline* ThirdParty::Pipeline::setMultisample(VkSampleCountFlags _sampleCount /*= VK_SAMPLE_COUNT_1_BIT*/, VkBool32 _sampleShadingEnable /*= VK_FALSE*/, float _minSampleShading /*= 1.0f*/, VkSampleMask* _pSampleMask /*= nullptr*/, VkBool32 _alphaToCoverageEnable /*= VK_FALSE*/, VkBool32 _alphaToOneEable /*= VK_FALSE*/)
{
	if (!p_multisample_satae_info) {
		p_multisample_satae_info = new VkPipelineMultisampleStateCreateInfo;
		p_multisample_satae_info->sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
		p_multisample_satae_info->pNext = nullptr;
		p_multisample_satae_info->flags = 0;
	}
	p_multisample_satae_info->sampleShadingEnable = _sampleShadingEnable;
	p_multisample_satae_info->rasterizationSamples = (VkSampleCountFlagBits)_sampleCount;
	p_multisample_satae_info->minSampleShading = _minSampleShading;
	p_multisample_satae_info->pSampleMask = _pSampleMask;
	p_multisample_satae_info->alphaToCoverageEnable = _alphaToCoverageEnable;
	p_multisample_satae_info->alphaToOneEnable = _alphaToOneEable;
	m_flags[MULTISAMPLE] = true;
	return this;
}

ThirdParty::Pipeline* ThirdParty::Pipeline::setDepthStencil(bool _depth_test, bool _depth_write, VkCompareOp _cmp_op)
{
	if (!p_depth_stencil_state_info) {
		p_depth_stencil_state_info = new VkPipelineDepthStencilStateCreateInfo();
		p_depth_stencil_state_info->sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
		p_depth_stencil_state_info->depthTestEnable = _depth_test;
		p_depth_stencil_state_info->depthWriteEnable = _depth_write;
		p_depth_stencil_state_info->depthCompareOp = _cmp_op;
		p_depth_stencil_state_info->depthBoundsTestEnable = VK_FALSE;
		p_depth_stencil_state_info->minDepthBounds = 0.0f;
		p_depth_stencil_state_info->maxDepthBounds = 1.0f;
		p_depth_stencil_state_info->stencilTestEnable = VK_FALSE;
		p_depth_stencil_state_info->front = {};
		p_depth_stencil_state_info->back = {};
	}
	m_flags[DEPTH_STENCIL] = true;
	return this;
}

ThirdParty::Pipeline* ThirdParty::Pipeline::setColorBlend(
	const std::vector<VkPipelineColorBlendAttachmentState>& _color_blends,
	VkBool32 _logicOpEnable,
	VkLogicOp _logicOp,
	const std::vector<float> _blendConstants)
{
	if (!p_color_blend_state_info) {
		p_color_blend_state_info = new VkPipelineColorBlendStateCreateInfo;
		p_color_blend_state_info->sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
		p_color_blend_state_info->pNext = nullptr;
		p_color_blend_state_info->flags = 0;
	}
	m_color_blends = _color_blends;
	p_color_blend_state_info->logicOpEnable = _logicOpEnable;
	p_color_blend_state_info->logicOp = _logicOp;
	p_color_blend_state_info->attachmentCount = static_cast<uint32_t>(m_color_blends.size());
	p_color_blend_state_info->pAttachments = m_color_blends.data();
	p_color_blend_state_info->blendConstants[0] = _blendConstants[0];
	p_color_blend_state_info->blendConstants[1] = _blendConstants[1];
	p_color_blend_state_info->blendConstants[2] = _blendConstants[2];
	p_color_blend_state_info->blendConstants[3] = _blendConstants[3];
	m_flags[COLOR_BLEND] = true;
	return this;
}

ThirdParty::Pipeline* ThirdParty::Pipeline::setTessellation(VkPipelineTessellationStateCreateInfo& _state)
{
	if (!p_tessellation_state_info) {
		p_tessellation_state_info = new VkPipelineTessellationStateCreateInfo;
		p_tessellation_state_info->sType = VK_STRUCTURE_TYPE_PIPELINE_TESSELLATION_STATE_CREATE_INFO;
		p_tessellation_state_info->pNext = nullptr;
	}
	p_tessellation_state_info->patchControlPoints = _state.patchControlPoints;
	return this;
}

ThirdParty::Pipeline* ThirdParty::Pipeline::setDynamic(const std::vector<VkDynamicState>& _dynamics)
{
	if (!p_dynamic_state_info) {
		p_dynamic_state_info = new VkPipelineDynamicStateCreateInfo;
		p_dynamic_state_info->sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
		p_dynamic_state_info->pNext = nullptr;
		p_dynamic_state_info->flags = 0;
	}
	m_dynamics = _dynamics;
	p_dynamic_state_info->dynamicStateCount = static_cast<uint32_t>(m_dynamics.size());
	p_dynamic_state_info->pDynamicStates = m_dynamics.data();
	m_flags[DYNAMIC] = true;
	return this;
}

ThirdParty::Pipeline* ThirdParty::Pipeline::setPipelineLayout(VkPipelineLayout _pipeline_layout)
{
	this->m_pipeline_layout = _pipeline_layout;
	m_flags[PIPELINE_LAYOUT] = true;
	return this;
}

ThirdParty::Pipeline* ThirdParty::Pipeline::setRenderPass(VkRenderPass _renderpass, uint32_t _subpass_index)
{
	this->m_renderpass = _renderpass;
	this->m_subpass_index = _subpass_index;
	m_flags[RENDERPASS] = true;
	return this;
}

ThirdParty::Pipeline* ThirdParty::Pipeline::setShaderStages(const std::vector<VkPipelineShaderStageCreateInfo>& _shader_stages)
{
	this->m_shader_stages = _shader_stages;
	m_flags[SHADER_STAGE] = true;
	return this;
}

bool ThirdParty::Pipeline::createGraphicsPipeline()
{
	bool available = true;
	if (!m_flags[PIPELINE_CREATE_STAGE::COLOR_BLEND])	 { Core::warn("[Pipeline] Color blend has not been set."); available &= false; }
	if (!m_flags[PIPELINE_CREATE_STAGE::DEPTH_STENCIL])  { Core::warn("[Pipeline] Depth stencil has not been set."); available &= false; }
	if (!m_flags[PIPELINE_CREATE_STAGE::DYNAMIC])		 { Core::warn("[Pipeline] Dynamic has not been set."); available &= false; }
	if (!m_flags[PIPELINE_CREATE_STAGE::INPUT_ASSEMBLY]) { Core::warn("[Pipeline] Input assembly has not been set."); available &= false; }
	if (!m_flags[PIPELINE_CREATE_STAGE::MULTISAMPLE])	 { Core::warn("[Pipeline] Multisample has not been set."); available &= false; }
	if (!m_flags[PIPELINE_CREATE_STAGE::PIPELINE_LAYOUT]){ Core::warn("[Pipeline] Pipeline layout has not been set."); available &= false; }
	if (!m_flags[PIPELINE_CREATE_STAGE::RASTERIZATION])  { Core::warn("[Pipeline] Rasterization has not been set."); available &= false; }
	if (!m_flags[PIPELINE_CREATE_STAGE::VERTEX_INPUT])	 { Core::warn("[Pipeline] Vertex input has not been set."); available &= false; }
	if (!m_flags[PIPELINE_CREATE_STAGE::VIEWPORT])		 { Core::warn("[Pipeline] Viewport has not been set."); available &= false; }
	if (!m_flags[PIPELINE_CREATE_STAGE::SHADER_STAGE])	 { Core::warn("[Pipeline] Shader stages has not been set."); available &= false; }
	if (!m_flags[PIPELINE_CREATE_STAGE::RENDERPASS])	 { Core::warn("[Pipeline] Renderpass has not been set."); available &= false; }
	if (!available) return VK_ERROR_UNKNOWN;
	if (!this->p_create_info) {
		this->p_create_info = new VkGraphicsPipelineCreateInfo;
		p_create_info->sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
		p_create_info->flags = 0;
		p_create_info->pNext = 0;
	}
	p_create_info->stageCount = static_cast<uint32_t>(m_shader_stages.size());
	p_create_info->pStages = m_shader_stages.data();
	p_create_info->pVertexInputState = p_vertex_input_state_info;
	p_create_info->pInputAssemblyState = p_input_assembly_state_info;
	p_create_info->pViewportState = p_viewport_state_info;
	p_create_info->pRasterizationState = p_raster_state_info;
	p_create_info->pMultisampleState = p_multisample_satae_info;
	p_create_info->pDepthStencilState = p_depth_stencil_state_info;
	p_create_info->pColorBlendState = p_color_blend_state_info;
	p_create_info->pDynamicState = p_dynamic_state_info;
	p_create_info->pTessellationState = p_tessellation_state_info;
	p_create_info->layout = m_pipeline_layout;
	p_create_info->renderPass = m_renderpass;
	p_create_info->subpass = m_subpass_index;
	p_create_info->basePipelineHandle = VK_NULL_HANDLE;
	p_create_info->basePipelineIndex = -1;
	
	m_result = vkCreateGraphicsPipelines(Core::vkInfo.m_device, VK_NULL_HANDLE, 1, p_create_info, nullptr, &m_pipeline);
	return checkVkResult(m_result, ("Failed to create pipeline:"+m_label).c_str());
}
