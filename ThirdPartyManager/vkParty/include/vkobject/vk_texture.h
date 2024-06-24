#pragma once
#include "vk_object.h"

namespace ThirdParty {

    //------------------------
    // VkImage VkImageView VkSampler的集合，无三者的创建与销毁功能
    // 如需要拥有三者创建功能的Texture,TODO...
    //------------------------
    class ENGINE_API_THIRDPARTY Texture:
        public vkObject
    {
    public:
        //------------------------------API 1.0
    public:
        inline Texture(const std::string& _label = "Texture") :vkObject(_label) {};
        inline Texture(VkImage _image, VkImageView _imageView, VkSampler _sampler = VK_NULL_HANDLE, const std::string& _label = "Texture")
            :vkObject(_label), m_image(_image), m_image_view(_imageView), m_sampler(_sampler) {}
        virtual ~Texture();
        inline VkImage& getVkImage() { return m_image; }
        inline VkImageView& getVkImageView() { return m_image_view; }
        inline VkSampler& getVkSampler() { return m_sampler; }
        inline VkImageLayout& getVkImageLayout() { return m_layout_current; }
        inline VkFormat& getVkFormat() { return m_format; }
        inline void setVkSampler(VkSampler _sampler) { m_sampler = _sampler; }

        VkImage& transitionImageLayout(VkImageLayout _oldLayout, VkImageLayout _newLayout, VkImageSubresourceRange _res = { VK_IMAGE_ASPECT_COLOR_BIT,0,1,0,1 }, VkCommandBuffer _cmd = VK_NULL_HANDLE);
        VkImage& transitionImageLayout(VkImageLayout _newLayout, VkImageSubresourceRange _res = { VK_IMAGE_ASPECT_COLOR_BIT,0,1,0,1 }, VkCommandBuffer _cmd = VK_NULL_HANDLE);

        void recreate(VkImage _image, VkImageView _imageView);
    private:
    protected:
        VkFormat m_format = VK_FORMAT_R8G8B8A8_UNORM;
        VkImageLayout m_layout_old = VK_IMAGE_LAYOUT_UNDEFINED;
        VkImageLayout m_layout_current = VK_IMAGE_LAYOUT_UNDEFINED;

        VkImage m_image = VK_NULL_HANDLE;
        VkImageView m_image_view = VK_NULL_HANDLE;
        VkSampler m_sampler = VK_NULL_HANDLE;
    };
}

