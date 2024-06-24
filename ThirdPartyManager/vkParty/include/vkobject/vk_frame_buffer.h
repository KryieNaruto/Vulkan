#pragma once
#include "vk_object.h"

namespace ThirdParty {

    class ENGINE_API_THIRDPARTY FrameBuffer :
        public vkObject
    {
    public:
        FrameBuffer(const std::string& _label = "FrameBuffer");
        virtual ~FrameBuffer();
        virtual VkFramebuffer createFrameBuffer(
			const VkExtent2D&   _size,
			VkRenderPass        _renderPass,
            const std::vector<VkImageView>&  _pAttachments,
            uint32_t _layers = 1
        );
        FrameBuffer* recreate(
            const VkExtent2D&   _size,
            const std::vector<VkImageView>&  _pAttachments);
        inline VkFramebuffer& getVkFramebuffer() { return m_frame_buffer; }
    private:
        VkFramebuffer m_frame_buffer = VK_NULL_HANDLE;
        VkFramebufferCreateInfo* p_createInfo = nullptr;

    };

}

