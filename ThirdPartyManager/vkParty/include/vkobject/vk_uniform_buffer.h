#pragma once
#include "vk_buffer.h"

namespace ThirdParty {

    /************************************************************************/
    /* 
    * ¶àÖ¡²¢ÐÐ
    Uniform Buffer
        Usage : VK_BUFFER_UNIFORM_BUFFER_BIT
        Memory: VK_MEMORY_PROPERTY_HOST_VISIABLE_BIT|VK_MEMORY_PROPERTY_HOST_COHERENT_BIT*/
        /************************************************************************/

    class ENGINE_API_THIRDPARTY UniformBuffer :
        public vkObject
    {
    public:
        UniformBuffer(size_t _size);
        UniformBuffer(size_t _size, const std::string _label);
        virtual ~UniformBuffer();
        UniformBuffer* bindData(void* _data);
        UniformBuffer* bindData(void* _data, size_t _offset, size_t _size);
		inline const VkBuffer& getVkBuffer() { return p_buffers[m_vkInfo.m_current_frame]->getVkBuffer(); }
        inline const size_t& getBufferSize() { return p_buffers[m_vkInfo.m_current_frame]->getBufferSize(); }
        inline const size_t& getBufferOffset() { return p_buffers[m_vkInfo.m_current_frame]->getBufferOffset(); }
    private:
        std::vector<Buffer*> p_buffers;
    };
}

