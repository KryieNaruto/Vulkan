#pragma once
#include "RShader.h"
#include <ThirdParty/spirv-reflect/spirv_reflect.h>

namespace Core::Resource {

    /************************************************************************/
    /* RShader的再封装，主要是完成反射的工作                                     */
    /* 将spvreflectresult 转化成 vkstruct                                    */
    /************************************************************************/
    /**
     * <Node属性>
     * *  继承自RShader的属性
     * *   1 Output
     *          DescriptorSet Layout : const std::vector<VkDescriptorSetLayout>
     */
    class ENGINE_API_RESOURCE Shader :
        public RShader
    {
    public:
		inline static std::unordered_map<std::string, Shader*>* p_shaders = nullptr;                  // 所有经过预加载的shader

        Shader(const std::string& _name);
        // 默认不透明管线，开启深度测试
        Shader(const std::string& _name, const std::unordered_map<ThirdParty::ShaderType, std::string>& shaderSources, bool _opaque = true, bool _depth_test = true, bool _multi_sample = true, uint32_t _subpass_index = 0, VkRenderPass _render_pass = VK_NULL_HANDLE);
        virtual ~Shader();
        // 多Shader 构成管线
        Shader(const std::string& _name, const std::vector<std::unordered_map<ThirdParty::ShaderType, std::string>>& _shader_sources, bool _opaque = true, bool _depth_test = true, bool _multi_sample = true, bool _precreate_pipeline = true, VkRenderPass _render_pass = VK_NULL_HANDLE);
        // 重新读取
        Shader* reload(bool _has_reload = false);
        // 绑定
        virtual void bind(VkCommandBuffer _cmd, VkPipeline _pipeline, const std::vector<VkDescriptorSet>& _sets) override;
        // 读取shader binary数据，转化为vk对象,创建最基础的pipeline
        void setupShader();
        // 解析数据，不创建vk对象
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
        // Location,获取入口点的输入
        void reflectStageInputVariables(const SpvReflectShaderModule& _module, const Core::RESOURCE_TYPE& _stage);
        // Uniform
        void reflectDescriptorSets(const SpvReflectShaderModule& _module);
        void reflectDescriptorSets(Core::RESOURCE_TYPE _type, const SpvReflectShaderModule& _module);
        // Constant
        void reflectPushConstant(const SpvReflectShaderModule& _module);

    private:
        // 反射对象
        std::vector<SpvReflectInterfaceVariable*> m_spv_reflect_input_variables;
        std::vector<SpvReflectDescriptorSet*> m_spv_reflect_descriptor_sets;
        std::vector<SpvReflectBlockVariable*> m_spv_reflect_push_constants;

        // 带索引的反射对象
        std::map <Core::RESOURCE_TYPE, std::vector<SpvReflectDescriptorSet*>> m_spv_reflect_descriptor_sets_map;
        std::map <uint32_t, std::vector<SpvReflectDescriptorSet*>> m_spv_reflect_descriptor_sets_map_setIndex;
    public:     // API 1.2
        inline const std::vector<ThirdParty::Slot*>& getSlots() { return p_slots; }
    private:    // API 1.2
        std::vector<ThirdParty::Slot*> p_slots;
    protected:    // API 1.1
        // vk对象
        VkPipelineLayout m_pipeline_layout;
        std::vector<VkDescriptorSetLayout> m_descriptor_set_layouts;
        std::vector<VkPushConstantRange> m_push_constant_ranges;
        // vertex input binding, 目前我只允许有一个。将所有顶点数据封装传入vk
        VkVertexInputBindingDescription m_vertex_input_binding;
        // 绘制模式
        VkPrimitiveTopology m_topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        std::vector<VkVertexInputAttributeDescription> m_vertex_input_attributes;
        // 辅助对象
        // key : setIndex
        std::unordered_map<uint32_t, std::vector<VkDescriptorSetLayoutBinding>> m_descriptor_set_layout_bindings;
        std::unordered_map<uint32_t, std::unordered_map<size_t, size_t>> m_descriptor_set_uniform_buffer_block_size;

        // 重建用
        std::unordered_map<ThirdParty::ShaderType, std::string> m_shaderSources;
        VkRenderPass m_render_pass = VK_NULL_HANDLE;
		bool m_reload_in_ME = false;    // 在材质编辑器中重置
        // spirv的位置
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
        VkCullModeFlagBits m_cull_flags = VK_CULL_MODE_NONE;    // 剔除
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

