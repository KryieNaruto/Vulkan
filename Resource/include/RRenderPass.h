#pragma once
#include "ZResource.h"

namespace Core::Resource {
    
    /************************************************************************/
    /* RenderPass的再封装，包含renderpass的创建与销毁功能                   */
    /************************************************************************/

    /**
     * <Node>
     *  1 Output
     *      RenderPass : VkRenderPass
     */
    class ENGINE_API_RESOURCE RRenderPass :
        public ZResource
    {
    public:
        RRenderPass();
        virtual ~RRenderPass();
        inline VkRenderPass getVkRenderPass() { return m_render_pass; }
        inline ThirdParty::RenderPass* getpRenderPass() { return p_render_pass; }

        RRenderPass* addAttachment(
            VkImageLayout                   _finalLayout,
            VkImageLayout                   _initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
            VkFormat                        _format = VK_FORMAT_R8G8B8A8_UNORM,
            VkSampleCountFlagBits           _samples = VK_SAMPLE_COUNT_1_BIT,
            VkAttachmentDescriptionFlags    _flags = 0,
            VkAttachmentLoadOp              _loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
            VkAttachmentStoreOp             _storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
            VkAttachmentLoadOp              _stencilLoadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
            VkAttachmentStoreOp             _stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE
        );
        inline RRenderPass* addAttachment(const VkAttachmentDescription& _a) { m_attachment_descs.push_back(_a); return this; }
        RRenderPass* addNewSubpass();
        RRenderPass* addDependency(
            uint32_t                _srcSubpass,
            uint32_t                _dstSubpass,
            VkPipelineStageFlags    _srcStageMask,
            VkPipelineStageFlags    _dstStageMask,
            VkAccessFlags           _srcAccessMask,
            VkAccessFlags           _dstAccessMask,
            VkDependencyFlags       _dependencyFlags);
        // 默认RRenderPass.m_subpass最后一个
        RRenderPass* setSubpassProperty_addColorAttachment(uint32_t _att_index, VkImageLayout _layout, int _sub_index = -1);
        RRenderPass* setSubpassProperty_addDpethStencilAttachment(uint32_t _att_index, VkImageLayout _layout, int _sub_index = -1);
        RRenderPass* setSubpassProperty_addResolveAttachment(uint32_t _att_index, VkImageLayout _layout, int _sub_index = -1);
        RRenderPass* setSubpassProperty_addInputAttachment(uint32_t _att_index, VkImageLayout _layout, int _sub_index = -1);
        void create();

        static RRenderPass* getRRenderPass(const std::string& _name);
        static void addRRenderPass(const std::string& _name,RRenderPass* _renderpass);
    protected:
        VkRenderPassCreateInfo* p_createInfo = nullptr;
        VkRenderPass m_render_pass = VK_NULL_HANDLE;
        ThirdParty::RenderPass* p_render_pass = nullptr;

        // attachment
        std::vector<VkAttachmentDescription> m_attachment_descs;
        // subpass
        std::vector<ThirdParty::Subpass*> m_subpass;
        /// subpass : dependencies
        std::vector<VkSubpassDependency> m_subpass_dependenies;
        /// subpass : descriptions
        std::vector<VkSubpassDescription> m_subpass_descriptions;

        void initTemplate() override;

    private:
		virtual Json::Value serializeToJSON() override;
		virtual void deserializeToObj(Json::Value& root) override;
    };

}

