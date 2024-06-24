#pragma once
#include "vk_object.h"

namespace ThirdParty {

    class ENGINE_API_THIRDPARTY Image:
        public vkObject
    {
    public:
        Image(const std::string& _label = "Image") :vkObject(_label) {};
        virtual ~Image();
        Image* createImage(
            VkImageUsageFlags        _usage,
            VkExtent3D               _extent,
            VkFormat                 _format = VK_FORMAT_R8G8B8A8_UNORM,
            //-----------------------------------
            uint32_t                 _mipLevels = 1,
            VkSampleCountFlagBits    _samples = VK_SAMPLE_COUNT_1_BIT,
            uint32_t                 _queueFamilyIndexCount = 0,
            const uint32_t*          _pQueueFamilyIndices = nullptr,
            VkSharingMode            _sharingMode = VK_SHARING_MODE_EXCLUSIVE,
            VkImageTiling            _tiling = VK_IMAGE_TILING_OPTIMAL,
            //-----------------------------------
            VkImageType              _imageType = VK_IMAGE_TYPE_2D,
            uint32_t                 _arrayLayers = 1,
            VkImageLayout            _initialLayout = VK_IMAGE_LAYOUT_UNDEFINED
            );
        Image* recreate(VkExtent3D _extent);
        VkImage& getVkImage();
    protected:
    private:
        size_t m_size = 0;
        size_t m_offset = 0;
        VkImageCreateInfo* p_create_info = nullptr;
        VkImage m_image = VK_NULL_HANDLE;
        VkDeviceMemory m_image_memory = VK_NULL_HANDLE;
    };

}

