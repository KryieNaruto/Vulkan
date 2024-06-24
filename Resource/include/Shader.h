#pragma once
#include "RShader.h"
#include <ThirdParty/spirv-reflect/spirv_reflect.h>

namespace Core::Resource {

    /************************************************************************/
    /* RShader���ٷ�װ����Ҫ����ɷ���Ĺ���                                     */
    /* ��spvreflectresult ת���� vkstruct                                    */
    /************************************************************************/
    /**
     * <Node����>
     * *  �̳���RShader������
     * *   1 Output
     *          DescriptorSet Layout : const std::vector<VkDescriptorSetLayout>
     */
    class ENGINE_API_RESOURCE Shader :
        public RShader
    {
    public:
		inline static std::unordered_map<std::string, Shader*>* p_shaders = nullptr;                  // ���о���Ԥ���ص�shader

        Shader(const std::string& _name);
        // Ĭ�ϲ�͸�����ߣ�������Ȳ���
        Shader(const std::string& _name, const std::unordered_map<ThirdParty::ShaderType, std::string>& shaderSources, bool _opaque = true, bool _depth_test = true, bool _multi_sample = true, uint32_t _subpass_index = 0, VkRenderPass _render_pass = VK_NULL_HANDLE);
        virtual ~Shader();
        // ��Shader ���ɹ���
        Shader(const std::string& _name, const std::vector<std::unordered_map<ThirdParty::ShaderType, std::string>>& _shader_sources, bool _opaque = true, bool _depth_test = true, bool _multi_sample = true, bool _precreate_pipeline = true, VkRenderPass _render_pass = VK_NULL_HANDLE);
        // ���¶�ȡ
        Shader* reload(bool _has_reload = false);
        // ��
        virtual void bind(VkCommandBuffer _cmd, VkPipeline _pipeline, const std::vector<VkDescriptorSet>& _sets) override;
        // ��ȡshader binary���ݣ�ת��Ϊvk����,�����������pipeline
        void setupShader();
        // �������ݣ�������vk����
        void analysisShader();
        inline VkPipelineLayout getPipelineLayout() { return m_pipeline_layout; }
        inline bool& isReady() { return m_success; }
        inline const VkVertexInputBindingDescription& getVkVertexInputBindingDescription() { return m_vertex_input_binding; }
        inline const std::vector<VkVertexInputAttributeDescription>& getVkVertexInputAttributeDescription() 
        { return m_vertex_input_attributes; }
        inline const std::unordered_map<uint32_t, std::unordered_map<size_t, size_t>>& getUniformBufferBlockSizes() 
        { return m_descriptor_set_uniform_buffer_block_size; }
        inline const std::unordered_map<uint32_t, std::vector<VkDescriptorSetLayoutBinding>>& getDescriptorSetLayoutBindings()
        { return m_descriptor_set_layout_bindings; }
        inline const std::vector<VkDescriptorSetLayout>& getDescriptorSetLayouts()
        { return m_descriptor_set_layouts; }
        inline bool& isReloadInMaterialEditor() { return m_reload_in_ME; }
    protected:
        // Location,��ȡ��ڵ������
        void reflectStageInputVariables(const SpvReflectShaderModule& _module, const Core::RESOURCE_TYPE& _stage);
        // Uniform
        void reflectDescriptorSets(const SpvReflectShaderModule& _module);
        void reflectDescriptorSets(Core::RESOURCE_TYPE _type, const SpvReflectShaderModule& _module);
        // Constant
        void reflectPushConstant(const SpvReflectShaderModule& _module);

    private:
        // �������
        std::vector<SpvReflectInterfaceVariable*> m_spv_reflect_input_variables;
        std::vector<SpvReflectDescriptorSet*> m_spv_reflect_descriptor_sets;
        std::vector<SpvReflectBlockVariable*> m_spv_reflect_push_constants;

        // �������ķ������
        std::map <Core::RESOURCE_TYPE, std::vector<SpvReflectDescriptorSet*>> m_spv_reflect_descriptor_sets_map;
        std::map <uint32_t, std::vector<SpvReflectDescriptorSet*>> m_spv_reflect_descriptor_sets_map_setIndex;
    public:     // API 1.2
        inline const std::vector<ThirdParty::Slot*>& getSlots() { return p_slots; }
    private:    // API 1.2
        std::vector<ThirdParty::Slot*> p_slots;
    protected:    // API 1.1
        // vk����
        VkPipelineLayout m_pipeline_layout;
        std::vector<VkDescriptorSetLayout> m_descriptor_set_layouts;
        std::vector<VkPushConstantRange> m_push_constant_ranges;
        // vertex input binding, Ŀǰ��ֻ������һ���������ж������ݷ�װ����vk
        VkVertexInputBindingDescription m_vertex_input_binding;
        // ����ģʽ
        VkPrimitiveTopology m_topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        std::vector<VkVertexInputAttributeDescription> m_vertex_input_attributes;
        // ��������
        // key : setIndex
        std::unordered_map<uint32_t, std::vector<VkDescriptorSetLayoutBinding>> m_descriptor_set_layout_bindings;
        std::unordered_map<uint32_t, std::unordered_map<size_t, size_t>> m_descriptor_set_uniform_buffer_block_size;

        // �ؽ���
        std::unordered_map<ThirdParty::ShaderType, std::string> m_shaderSources;
        VkRenderPass m_render_pass = VK_NULL_HANDLE;
		bool m_reload_in_ME = false;    // �ڲ��ʱ༭��������
        // spirv��λ��
        std::unordered_map<ThirdParty::ShaderType, std::string> m_spirv_paths;

    public:
        Shader* setDepthTest(bool _v) { m_depth_test = _v; return this; }
        Shader* setDepthWrite(bool _v) { m_depth_write = _v; return this; }
        Shader* setOpaque(bool _v) { m_opaque = _v; return this; }
        Shader* setMultiSampler(bool _v) { m_multi_sample = _v; return this; }
        Shader* setCullMode(VkCullModeFlagBits _v) { m_cull_flags = _v; return this; }
        Shader* setDepthCompareOp(VkCompareOp _v) { m_depth_compare = _v; return this; }
        Shader* setDepthBias(VkBool32 _v) { m_depth_bias = _v; return this; }
        Shader* addDynamic(VkDynamicState _v) { m_dynamic.insert(_v); return this; }
        Shader* setSubpassIndex(uint32_t _v) { m_subpass_index = (_v); return this; }
        Shader* setRenderPass(VkRenderPass _v) { m_render_pass = _v; return this; }
        Shader* setTopology(VkPrimitiveTopology _v) { m_topology = _v; return this; }
        Shader* createShader(const std::unordered_map<ThirdParty::ShaderType, std::string>& _shaderSource);
    protected:
        bool m_depth_test = true;
        bool m_depth_write = true;
        bool m_opaque = true;
        bool m_multi_sample = true;
        VkCullModeFlagBits m_cull_flags = VK_CULL_MODE_NONE;    // �޳�
        VkCompareOp m_depth_compare = VK_COMPARE_OP_LESS_OR_EQUAL;
        VkBool32 m_depth_bias = VK_FALSE;
        inline static std::set<VkDynamicState> m_dynamic;
        uint32_t m_subpass_index = 0;

        // Override
        void initTemplate() override;
    public:
        void initObjFromTemplate() override;
    };

}

