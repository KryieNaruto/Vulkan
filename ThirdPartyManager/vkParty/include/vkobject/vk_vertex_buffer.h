#pragma once
#include "vk_buffer.h"

namespace ThirdParty {
    
    /************************************************************************/
    /* Vertex Buffer
        Usage : VK_BUFFER_USAGE_VERTEX_BUFFER_BIT|VK_BUFFER_TRANSFER_DST_BIT
        Memory: VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
    Stage Buffer
        Usage : VK_BUFFER_TRANSFER_SRC_BIT
        Memory: VK_MEMORY_PROPERTY_HOST_VISIABLE_BIT|VK_MEMORY_PROPERTY_HOST_COHERENT_BIT*/
    /************************************************************************/

    class ENGINE_API_THIRDPARTY VertexBuffer
        : public vkObject
    {
    public:
        VertexBuffer(size_t _size);
        VertexBuffer(size_t _size, const std::string _label);
		virtual ~VertexBuffer();
        void bindData(void* data);
        inline VkBuffer getVkBuffer() { return p_buffer->getVkBuffer(); }
        inline VkDeviceSize getBufferOffset() { return p_buffer->getBufferOffset(); }
    private:
        Buffer* p_buffer = nullptr;
		Buffer* p_stage_buffer = nullptr;
    };

}

