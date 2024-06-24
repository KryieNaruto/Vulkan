#pragma once
#include "vk_object.h"

namespace ThirdParty {
    /**
     * 管线布局，包含创建与销毁
     * <Node 属性>
     * * 2 Inputs
     *		DescriptorSet Layouts: const std::vector<VkDescriptorSetLayout>
     *		Push ConstantRange	 : const std::vector<VkPushConstantRange>
     * * 1 Outputs
     *		Pipeline Layout : VkPipelineLayout
     */
    class ENGINE_API_THIRDPARTY PipelineLayout :
        public vkObject
    {
    public:
        PipelineLayout(const std::string& _label = "Pipeline Layout");
        PipelineLayout(const std::vector<VkDescriptorSetLayout>& _layouts, const std::vector<VkPushConstantRange>& _push_constants = {}, const std::string & _label = "Pipeline Layout");
        virtual ~PipelineLayout();
        inline VkPipelineLayout getPipelineLayout() { return m_pipeline_layout; }

    private:
        std::vector<VkDescriptorSetLayout> m_descriptor_set_layouts;
        std::vector<VkPushConstantRange> m_push_constant_ranges;
        VkPipelineLayout m_pipeline_layout = VK_NULL_HANDLE;
        VkPipelineLayoutCreateInfo* p_create_info = nullptr;

        void initTemplate() override;
    };
}

