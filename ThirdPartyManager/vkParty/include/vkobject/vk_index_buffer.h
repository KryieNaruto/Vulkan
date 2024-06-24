#pragma once
#include "vk_buffer.h"

namespace ThirdParty {

    /************************************************************************/
    /* 
    Index Buffer
        Usage : VK_BUFFER_INDEX_BUFFER_BIT
        Memory: VK_MEMORY_PROPERTY_HOST_VISIABLE_BIT|VK_MEMORY_PROPERTY_HOST_COHERENT_BIT*/
        /************************************************************************/

    class ENGINE_API_THIRDPARTY IndexBuffer
        : public vkObject
    {
    public:
        IndexBuffer(size_t _size);
        IndexBuffer(size_t _size, const std::string);
        virtual ~IndexBuffer();
        IndexBuffer* bindData(void* _data);

        inline VkBuffer getVkBuffer() { return p_buffer->getVkBuffer(); }
        inline VkDeviceSize getBufferOffset() { return p_buffer->getBufferOffset(); }
    private:
        Buffer* p_buffer = nullptr;
    };

}

