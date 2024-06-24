#pragma once
#include "vk_object.h"

namespace ThirdParty {

    // 渲染流程，只包含渲染流程功能，不提供创建与销毁RenderPass功能
    class ENGINE_API_THIRDPARTY RenderPass :
        public vkObject
    {
    public:
        RenderPass(VkRenderPass _renderpass,const std::string& _label = "RenderPass");
        virtual ~RenderPass();
        void beginRenderPass(VkCommandBuffer _commandBuffer,VkSubpassContents _contents = VK_SUBPASS_CONTENTS_INLINE);
        void endRenderPass();
        inline VkRenderPass& getVkRenderPass() { return m_render_pass; }

        // begin 前设定
        inline RenderPass* setFrameBuffer(VkFramebuffer _framebuffer) {
            m_flag[(int)RENDERPASS_BEGIN_FLAG::FRAME_BUFFER] = true;
            t_framebuffer = _framebuffer;
            return this;
        }
        inline RenderPass* setRenderArea(const VkRect2D& _renderArea) {
			m_flag[(int)RENDERPASS_BEGIN_FLAG::RENDER_AREA] = true;
            t_render_area = _renderArea;
			return this;
        }
        inline RenderPass* setClearValues(const std::vector<VkClearValue>& _clear_values) {
			m_flag[(int)RENDERPASS_BEGIN_FLAG::CLEAR_VALUE] = true;
            t_clear_values = _clear_values;
			return this;
        }

    private:
        enum class RENDERPASS_BEGIN_FLAG {
            //RENDERPASS = 0x00,
            FRAME_BUFFER = 0x01,
            RENDER_AREA = 0x02,
            CLEAR_VALUE = 0x03,
        };
        VkFramebuffer t_framebuffer = VK_NULL_HANDLE;
        VkRect2D t_render_area;
        std::vector<VkClearValue> t_clear_values;
        VkCommandBuffer t_commandBuffer = VK_NULL_HANDLE;

        bool m_flag[16] = { false };
        VkRenderPass m_render_pass = VK_NULL_HANDLE;
        VkRenderPassBeginInfo* p_begin_info = nullptr;
    };

}

