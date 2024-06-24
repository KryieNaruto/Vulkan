#pragma once
#include "vk_texture.h"
#include "vk_buffer.h"
#include <ThirdParty/glm/glm.hpp>

namespace ThirdParty {
	struct ENGINE_API_THIRDPARTY TextureFileInfo
	{
		int m_width;
		int m_height;
		int m_channels;
        float m_aspect;
		std::string m_path;
	};
    /**
     *  Texture 从数据中创建vk对象
     */
    class ENGINE_API_THIRDPARTY TextureEXT:
        public Texture
    {
    public:
        TextureEXT();
        // Cube 6面纹理, 顺序是前后左右上下
        TextureEXT(const std::vector<std::string>& _paths,const std::string& _name = "");
        // Cube 6面空纹理
        TextureEXT(const VkExtent3D& _extent, VkFormat _format, VkImageTiling _tiling, VkImageUsageFlags _usage, VkMemoryPropertyFlags _prop, VkImageLayout _layout, uint32_t _layer_count, VkSampleCountFlags _sample_count, const std::string& _name, uint32_t _mip_map = 1);
        // 复制构造
        TextureEXT(const TextureEXT& _tex);
        // 创建指定大小的空图像
        TextureEXT(const VkExtent3D& _extent, VkFormat _format, VkImageTiling _tiling, VkImageUsageFlags _usage, VkMemoryPropertyFlags _prop, VkImageLayout _layout, VkSampleCountFlags _sample_count = VK_SAMPLE_COUNT_1_BIT, const std::string& _name = "");
        // 仅创建一个包含sampler的空槽
        TextureEXT(VkSampler _sampler,bool _only_sampler = false);
        // 从文件中加载
        TextureEXT(std::string& _path,const std::string& _name = "");
        // 赋值操作符
        TextureEXT& operator=(const TextureEXT& _tex);

        ~TextureEXT();

        // 对空图像填充数据
        void copyData(void* data);
        // 复制vk图像
        void copyTextureEXT(TextureEXT* _tex);
        // 复制所有信息
        void copyAllTextureEXT(TextureEXT* _src);
        // 重新加载
        void reload(std::string& _path);
        // 返回是否进行了加载
        inline bool load(std::string& _path) { loadDataFromFile(_path); return m_only_sampler; }
        // 从MaterialEditor中加载
        void initObjFromTemplate() override;
    protected:
        Buffer* p_stage = nullptr;
        TextureFileInfo* p_file_info = nullptr;
        std::vector<TextureFileInfo*> p_cube_file_info;
        VkDeviceSize m_size = 0;
        VkDeviceSize m_offset = 0;
        VkImageCreateInfo* p_image_create_info = nullptr;
        std::vector<VkImageView> m_imageview_cube;
        VkImageViewCreateInfo* p_image_view_create_info = nullptr;
        VkDeviceMemory m_image_memory = VK_NULL_HANDLE;
        // 如果Texture是Array，则会为每个array 创建一个view
        std::vector<VkImageView> m_image_view_array;
    protected:
        void initTemplate() override;
    private:
        uint32_t m_mipLevels = 0;
        bool m_only_sampler = false;
        // 从文件中加载图像并完成布局变换
        TextureEXT* loadDataFromFile(std::string& _path);
        // 从文件中加载天空盒
        TextureEXT* loadDataFromFiles(const std::vector<std::string>& _paths);

        // 是否是其他TextureEXT的复制
        bool m_copy = false;
        bool m_is_cube = false;
        uint32_t m_layers = 1;

        // recreate 用
        VkImageAspectFlags m_aspect;

    public:
		inline static std::unordered_map<std::string, VkDescriptorSet> _sets;	// 路径， TexID
		inline static std::unordered_map<void*, VkDescriptorSet> _final_sets;	// TextureEXT, TexID
		// 返回是否包含纹理
		inline bool hasTexture() { return !m_only_sampler; }
		// 是否是Cube
		inline bool isCube() { return m_is_cube; }
		inline bool isArray() { return m_layers > 1 || m_is_cube; }
		virtual void recreate(const VkExtent3D& _extent);
		// 如果没有填_path，则返回默认1x1x1的图像 
		inline static TextureEXT* getTextureEXT(const std::string& _path = "") {
			if (_path == "") return (TextureEXT*)Core::g_textureEXT_loaded["default"];
			if (Core::g_textureEXT_loaded.find(_path) != Core::g_textureEXT_loaded.end())
				return (TextureEXT*)Core::g_textureEXT_loaded[_path];
			else
				return nullptr;
		}
		inline const std::vector<TextureFileInfo*> getPaths() {
			if (isArray()) return p_cube_file_info;
			else return { p_file_info };
		}
		inline const std::vector<VkImageView> getVkImageViews() {
			if (isArray() || m_image_view_array.size() > 1)
				return m_image_view_array;
			else return { m_image_view };
		}
		inline glm::vec2 getSize() {
			if (isCube() || isArray()) return { p_cube_file_info[0]->m_width, p_cube_file_info[0]->m_height };
			else return { p_file_info->m_width,p_file_info->m_height };
		}
		inline float getAspect() {
			if (isCube() || isArray()) return p_cube_file_info[0]->m_aspect;
			else return p_file_info->m_aspect;
		}
        inline VkFormat& getVkFormat() {
            if (m_format != p_image_create_info->format)
                m_format = p_image_create_info->format;
            return m_format;
        }
    };
}

