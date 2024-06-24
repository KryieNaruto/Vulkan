#pragma once
#include "vk_object.h"

namespace ThirdParty {
    
    class  ENGINE_API_THIRDPARTY ImageView :
        public vkObject
    {
    public:
        inline ImageView(const std::string& _label = "Image View") :vkObject(_label) {}
        virtual ~ImageView();
        ImageView* createImageView(
            VkImage                    _image,
            VkFormat                   _format = VK_FORMAT_R8G8B8A8_UNORM,
            VkImageSubresourceRange    _subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT,0,1,0,1 },
            VkImageViewType            _viewType = VK_IMAGE_VIEW_TYPE_2D,
            VkComponentMapping         _components = { VK_COMPONENT_SWIZZLE_IDENTITY,
                                                        VK_COMPONENT_SWIZZLE_IDENTITY ,
                                                        VK_COMPONENT_SWIZZLE_IDENTITY ,
                                                        VK_COMPONENT_SWIZZLE_IDENTITY }
        );
        inline VkImageView& getVkImageView() { return m_image_view; }
        ImageView* recreate(VkImage _image);
    private:
    protected:
        VkImageViewCreateInfo* p_create_info = nullptr;
        VkImageView m_image_view = VK_NULL_HANDLE;
    };
}

