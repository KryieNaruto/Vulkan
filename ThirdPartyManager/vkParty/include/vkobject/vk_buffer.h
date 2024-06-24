#pragma once
#include "vk_object.h"

namespace ThirdParty {

    /************************************************************************/
    /* Buffer ��װ������Buffer�������٣�memory����ȡ����                */
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
        // �������ݵ���һ��VkBuffer��2�ߴ�С������ͬ
        Buffer* copyData(Buffer* _buffer);
        Buffer* copyData(VkBuffer _dstBuffer);
        // �������ݵ�vkimage
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

