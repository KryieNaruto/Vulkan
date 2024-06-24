#include "Shader.h"
#include <RRenderPass.h>
#include <Core/include/core_global.h>
#include <set>
Core::Resource::Shader::~Shader()
{
	m_spv_reflect_input_variables.clear();
	m_spv_reflect_descriptor_sets.clear();
	m_spv_reflect_push_constants.clear();
	for (auto& [stage, _a] : m_reflect_shaderModules)
		spvReflectDestroyShaderModule(&_a);
	if (m_pipeline_layout != VK_NULL_HANDLE)
		vkDestroyPipelineLayout(m_vkInfo.m_device, m_pipeline_layout, m_vkInfo.p_allocate);
	for (const auto& _slot : p_slots)
		delete _slot;
	p_slots.clear();
}

void Core::Resource::Shader::setupShader()
{
	analysisShader();
	// 等待解析完成，统一创建SetLayout
	for (const auto& _slot : p_slots) {
		_slot->createDescriptorSetLayout();
		m_descriptor_set_layouts.push_back(_slot->getVkDescriptorSetLayout());
	}

	// 创建vkpipelineLayout
	{
		VkPipelineLayoutCreateInfo _createInfo = { VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO };
		_createInfo.setLayoutCount = m_descriptor_set_layouts.size();
		_createInfo.pSetLayouts = m_descriptor_set_layouts.data();
		_createInfo.pushConstantRangeCount = m_push_constant_ranges.size();
		_createInfo.pPushConstantRanges = m_push_constant_ranges.data();
		const auto& _b = ThirdParty::vk_manager.createPipelineLayout(_createInfo);
		if (_b != VK_NULL_HANDLE)
			m_pipeline_layout = _b;
	}

	// 如果编译通过
	// 创建Pipeline
	if (m_success) {
		VkPipelineColorBlendAttachmentState _b= {
			VK_FALSE ,
			VK_BLEND_FACTOR_ONE ,VK_BLEND_FACTOR_ZERO ,VK_BLEND_OP_ADD,
			VK_BLEND_FACTOR_ONE,VK_BLEND_FACTOR_ZERO,VK_BLEND_OP_ADD,
			VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT
		};
		// 透明
		if (!m_opaque) {
			_b = {
				VK_TRUE,
				VK_BLEND_FACTOR_ONE ,VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA ,VK_BLEND_OP_ADD,
				VK_BLEND_FACTOR_ONE,VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,VK_BLEND_OP_ADD,
				VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT
			};
		}

		std::vector<VkDynamicState> _dynamic = { VK_DYNAMIC_STATE_VIEWPORT ,VK_DYNAMIC_STATE_SCISSOR };
		_dynamic.insert(_dynamic.begin(), m_dynamic.begin(), m_dynamic.end());

		ThirdParty::Pipeline* _p_pipeline = new ThirdParty::Pipeline(getFileName());
		_p_pipeline->setVertexInputBinding(
			{ m_vertex_input_binding },
			m_vertex_input_attributes)
			->setInputAssembly(m_topology)
			->setViewportAndScissor(
				{ 0,0,
				(float)ThirdParty::Core::windowInfo.m_window_width,
				(float)ThirdParty::Core::windowInfo.m_window_height,0.0f,1.0f },
				{ {0,0},ThirdParty::Core::vkInfo.m_swapchain_extent }
			)
			->setRasterization(VK_POLYGON_MODE_FILL, m_cull_flags, VK_FRONT_FACE_COUNTER_CLOCKWISE, 1.f, 0, 0, m_depth_bias)
			->setMultisample(m_multi_sample ? ThirdParty::Core::vkInfo.m_sample_count : VK_SAMPLE_COUNT_1_BIT)
			->setDepthStencil(m_depth_test, m_depth_write, m_depth_compare)
			->setColorBlend({ _b })
			->setDynamic(_dynamic)
			->setRenderPass
			(m_render_pass == VK_NULL_HANDLE ? ((RRenderPass*)Core::Resource::Pool::getRRenderPass(RENDER_PASS_0))->getVkRenderPass() : m_render_pass, m_subpass_index)
			->setShaderStages(getShaderStages())
			->setPipelineLayout(m_pipeline_layout)
			->createGraphicsPipeline();
		Core::Resource::Pool::addThirdParty_PipelineInfoPool(getName(), _p_pipeline);
	}

}

void Core::Resource::Shader::reflectStageInputVariables(const SpvReflectShaderModule& _module, const Core::RESOURCE_TYPE& _stage)
{
	const auto& _entry_point = m_entry_points[_stage];
	uint32_t var_count = 0;
	spvReflectEnumerateEntryPointInputVariables(&_module, _entry_point.c_str(), &var_count, NULL);
	if (var_count == 0) return;
	m_spv_reflect_input_variables.resize(var_count);
	spvReflectEnumerateEntryPointInputVariables(&_module, _entry_point.c_str(), &var_count, m_spv_reflect_input_variables.data());
	// 如果第一个为-1或0xff 则去除
	for (auto ite = m_spv_reflect_input_variables.begin(); ite < m_spv_reflect_input_variables.end(); ite++) {
		if ((*ite)->location < 0 || (*ite)->location >= UINT_MAX)
			m_spv_reflect_input_variables.erase(ite);
		return;
	}

}

void Core::Resource::Shader::reflectDescriptorSets(const SpvReflectShaderModule& _module)
{
	auto& sets = m_spv_reflect_descriptor_sets;
	uint32_t var_count = 0;
	spvReflectEnumerateDescriptorSets(&_module, &var_count, nullptr);
	if (var_count == 0) return;
	sets.resize(var_count);
	spvReflectEnumerateDescriptorSets(&_module, &var_count, sets.data());
}

void Core::Resource::Shader::reflectDescriptorSets(Core::RESOURCE_TYPE _type, const SpvReflectShaderModule& _module)
{
	auto& sets = m_spv_reflect_descriptor_sets_map;
	uint32_t var_count = 0;
	spvReflectEnumerateDescriptorSets(&_module, &var_count, nullptr);
	if (var_count == 0) return;
	sets[_type].resize(var_count);
	spvReflectEnumerateDescriptorSets(&_module, &var_count, sets[_type].data());

	// 更新总descriptor set
	m_spv_reflect_descriptor_sets_map_setIndex.clear();
	for (const auto& _sets_ite : sets) {
		for (const auto& _set : _sets_ite.second) {
			m_spv_reflect_descriptor_sets_map_setIndex[_set->set].push_back(_set);
		}
	}
}

void Core::Resource::Shader::reflectPushConstant(const SpvReflectShaderModule& _module)
{
	auto& ranges = m_spv_reflect_push_constants;
	uint32_t var_count = 0;
	spvReflectEnumeratePushConstants(&_module, &var_count, nullptr);
	if (var_count == 0) return;
	ranges.resize(var_count);
	spvReflectEnumeratePushConstants(&_module, &var_count, ranges.data());
}

Core::Resource::Shader::Shader(const std::string& _name)
	:RShader(_name)
{
	m_json_title = "Shader";
	initTemplate();
}

Core::Resource::Shader::Shader(
	const std::string& _name,
	const std::unordered_map<ThirdParty::ShaderType, std::string>& shaderSources,
	bool _opaque, bool _depth_test, bool _multi_sample,uint32_t _subpass_index, VkRenderPass _render_pass)
	:RShader(_name), m_depth_test(_depth_test), m_opaque(_opaque),
	m_shaderSources(shaderSources), m_render_pass(_render_pass), m_multi_sample(_multi_sample),
	m_subpass_index(_subpass_index)
{
	m_json_title = "Shader";
	if (_name == "") m_res_name = getFileName();
	compilerOrGetVulkanBinaries(shaderSources);
	if (m_success) {
		createModules();
		getShaderStages();
		setupShader();
	}
}

void Core::Resource::Shader::bind(VkCommandBuffer _cmd, VkPipeline _pipeline, const std::vector<VkDescriptorSet>& _sets)
{
	if (_pipeline != VK_NULL_HANDLE)
		vkCmdBindPipeline(_cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, _pipeline);
	if (_sets.size() > 0)
		vkCmdBindDescriptorSets(_cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipeline_layout, 0, _sets.size(), _sets.data(), 0, 0);
}

Core::Resource::Shader* Core::Resource::Shader::reload(bool _has_reload)
{
	if (_has_reload)
		if (m_success) return this;
		else return nullptr;
	m_reloading = true;
	compilerOrGetVulkanBinaries(m_shaderSources);
	if (m_success) {
		// 销毁原有vk对象
		{
			vkDeviceWaitIdle(m_vkInfo.m_device);
			// vklayout
			ThirdParty::vkManager.destroyVkObject(m_pipeline_layout);
			// shader module
			for (const auto& _m : m_shaderModules)
				vkDestroyShaderModule(m_vkInfo.m_device, _m.second, m_vkInfo.p_allocate);
			m_shaderModules.clear();
			// 销毁原有pipeline
			auto _name = getName();
			if (Core::Core_Pool::g_all_pipelines.find(_name) != Core::Core_Pool::g_all_pipelines.end())
			{
				for (auto _ite = ThirdParty::Core::g_vk_objects.begin(); _ite < ThirdParty::Core::g_vk_objects.end(); _ite++)
					if (*_ite == Core::Core_Pool::g_all_pipelines[_name]) {
						ThirdParty::Core::g_vk_objects.erase(_ite);
						break;
					}
				delete (ThirdParty::Pipeline*)Core::Core_Pool::g_all_pipelines[_name];
				Core::Core_Pool::g_all_pipelines.erase(Core::Core_Pool::g_all_pipelines.find(_name));
			}
		}
		createModules();
		getShaderStages();
		setupShader();
		m_reloading = false;
		return this;
	}
	else return nullptr;
}

Core::Resource::Shader* Core::Resource::Shader::createShader(const std::unordered_map<ThirdParty::ShaderType, std::string>& _shaderSource)
{
	m_shaderSources = _shaderSource;
	if (m_res_name == "")
		m_res_name = getFileName();
	compilerOrGetVulkanBinaries(m_shaderSources);
	if (m_success) {
		createModules();
		getShaderStages();
		setupShader();
	}
	return this;
}

void Core::Resource::Shader::analysisShader()
{
	for (const auto& [stage, _module] : m_reflect_shaderModules) {
		if (stage == Core::VERTEX_SHADER) { reflectStageInputVariables(_module, stage); }
		reflectDescriptorSets(stage, _module);
		reflectPushConstant(_module);
	}

	// 创建vertex input binding description
	{
		m_vertex_input_attributes.clear();
		// 创建vertex input arrtibute description
		int _stride = 0;
		for (const auto& input : m_spv_reflect_input_variables) {
			size_t _offset = input->numeric.vector.component_count * input->numeric.scalar.width / 8;
			VkVertexInputAttributeDescription _attr;
			_attr.binding = 0;
			_attr.location = input->location;
			_attr.format = VkFormat(input->format);
			_attr.offset = _stride;
			_stride += _offset;
			m_vertex_input_attributes.push_back(_attr);
		}
		m_vertex_input_binding.binding = 0;
		m_vertex_input_binding.stride = _stride;
		m_vertex_input_binding.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
	}

	// 获取VkPushConstantRange
	{
		m_push_constant_ranges.clear();
		for (const auto& range : m_spv_reflect_push_constants) {
			VkPushConstantRange _range;
			_range.offset = range->absolute_offset;
			_range.size = range->size;
			_range.stageFlags = VK_SHADER_STAGE_ALL;

			m_push_constant_ranges.push_back(_range);
		}
	}

	// 创建Slot
	{
		if (!m_descriptor_set_layouts.empty()) {
			m_descriptor_set_layouts.clear();
		}
		// 处理重载
		if (!p_slots.empty()) {
			for (const auto& _slot : p_slots) delete _slot;
			p_slots.clear();
		}
		// 跳过已经加载的Set Binding
		std::map<uint32_t, std::set<uint32_t>> _set_id_has_loaded;
		for (const auto& [_stage, _sets] : m_spv_reflect_descriptor_sets_map) {
			for (const auto& _set : _sets) {
				// 如果Set binding 已经加载则跳过生成Slot,转而增加一次Stage
				if (_set_id_has_loaded.contains(_set->set)) {
					auto _bindings_has_loaded = _set_id_has_loaded[_set->set];
					for (size_t _binding_index = 0; _binding_index < _set->binding_count; _binding_index++) {
						auto _binding = _set->bindings[_binding_index];
						// Set 已经加载，且Binding也加载
						if (_bindings_has_loaded.contains(_binding->binding)) {
							p_slots[_set->set]
								->setShaderStage(_binding->binding, getShaderStageBit(_stage));
						}
						// Set 已经加载，但是Binding是新的
						else {
							p_slots[_set->set]
								->addSlotData(_binding)
								->setShaderStage(_binding->binding, getShaderStageBit(_stage));
							// 保存binding到loaded
							_set_id_has_loaded[_set->set].insert(_binding_index);
						}
					}
				}
				// Set 从未加载过
				else {
					ThirdParty::Slot* p_slot = new ThirdParty::Slot(_set);
					for (const auto& _binding : p_slot->getBindingIndices()) {
						p_slot->setShaderStage(_binding, getShaderStageBit(_stage));
					}
					p_slots.push_back(p_slot);
					_set_id_has_loaded[_set->set] = p_slot->getBindingIndices();
				}
			}
		}
	}
}

void Core::Resource::Shader::initTemplate() {
	{
		// Output
		auto _output_0 = new ThirdParty::imgui::SlotTemplateOutput();
		_output_0->setName("Shader");
		_output_0->p_data = this;
		_output_0->m_functional = [this]()->void {
			if (ImGui::Button("Reload")) {
				m_reload_in_ME = true;
				m_reloading = true;
			}
			};
		p_template->addOutputSlotTemplates({ _output_0 });
	}
}

void Core::Resource::Shader::initObjFromTemplate()
{
	bool _need_reload = false;
	// 如果已经加载，则重新加载
	if (!m_paths.empty()) {
		_need_reload = true;
		if (_need_reload) {
			m_spv_reflect_input_variables.clear();
			m_spv_reflect_descriptor_sets.clear();
			m_spv_reflect_push_constants.clear();
			for (auto& [stage, _a] : m_reflect_shaderModules)
				spvReflectDestroyShaderModule(&_a);
			for (const auto& _slot : p_slots)
				delete _slot;
			p_slots.clear();

			// 销毁原有vk对象
			{
				vkDeviceWaitIdle(m_vkInfo.m_device);
				// vklayout
				ThirdParty::vkManager.destroyVkObject(m_pipeline_layout);
				// shader module
				for (const auto& _m : m_shaderModules)
					vkDestroyShaderModule(m_vkInfo.m_device, _m.second, m_vkInfo.p_allocate);
				m_shaderModules.clear();
				m_shader_stages.clear();
				// 销毁原有pipeline
				auto _name = getName();
				if (Core::Core_Pool::g_all_pipelines.find(_name) != Core::Core_Pool::g_all_pipelines.end())
				{
					for (auto _ite = ThirdParty::Core::g_vk_objects.begin(); _ite < ThirdParty::Core::g_vk_objects.end(); _ite++)
						if (*_ite == Core::Core_Pool::g_all_pipelines[_name]) {
							ThirdParty::Core::g_vk_objects.erase(_ite);
							break;
						}
					delete (ThirdParty::Pipeline*)Core::Core_Pool::g_all_pipelines[_name];
					Core::Core_Pool::g_all_pipelines.erase(Core::Core_Pool::g_all_pipelines.find(_name));
				}
			}
		}
	}

	// 从input节点中获取glsl文件并保存
	auto _inputs = p_template->p_inputs;
	// 获取文件路径
	for (auto _slot : _inputs)
	{
		if (_slot->p_data == nullptr) continue;
		auto _name = _slot->m_name_2;
		auto _path = *static_cast<std::string*>(_slot->p_data);
		m_paths[string_to_RESOURCE_TYPE(_name)] = _path;
		m_shaderSources[ThirdParty::string_to_ShaderType(_name)] = _path;
	}
	compilerOrGetVulkanBinaries(m_shaderSources);
	if (m_success) {
		createModules();
		getShaderStages();
		setupShader();
	}
}

