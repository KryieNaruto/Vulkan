#pragma once
#include "vk_object.h"

namespace ThirdParty {

    /************************************************************************/
    /* Buffer 封装，包含Buffer创建销毁，memory绑定与取消绑定                */
    /************************************************************************/
    class ENGINE_API_THIRDPARTY Buffer:
        public vkObject
    {
    public:
        Buffer(size_t _size, VkBufferUsageFlags _usage, VkMemoryPropertyFlags _prop);
        Buffer(const std::string& _label = "Buffer");
        virtual ~Buffer();
        Buffer* bindData(void* _data);
        Buffer* bindData(void* _data, size_t _offset, size_t _size);
        // 复制数据到另一个VkBuffer，2者大小必须相同
        Buffer* copyData(Buffer* _buffer);
        Buffer* copyData(VkBuffer _dstBuffer);
        // 复制数据到vkimage
        Buffer* copyDataToVkImage(VkImage _image, uint32_t _width, uint32_t _height, VkImageAspectFlags _aspect, uint32_t _mipLevel,uint32_t _baseLayer = 0);
        inline VkBuffer getVkBuffer() { return m_buffer; }
		inline VkDeviceMemory getVkDeviceMemory() { return m_buffer_memory; }
        inline VkDeviceSize getBufferSize() { return m_buffer_size; }
        inline VkDeviceSize getBufferOffset() { return m_buffer_offset; }
    protected:
        VkBufferUsageFlags m_buffer_usage = 0;
        VkMemoryPropertyFlags m_memory_property = 0;

        VkDeviceSize m_buffer_size = 0;
        VkDeviceSize m_buffer_offset = 0;
        VkBuffer m_buffer = VK_NULL_HANDLE;
        VkDeviceMemory m_buffer_memory = VK_NULL_HANDLE;
    };

}

