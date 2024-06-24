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
     *  Texture �������д���vk����
     */
    class ENGINE_API_THIRDPARTY TextureEXT:
        public Texture
    {
    public:
        TextureEXT();
        // Cube 6������, ˳����ǰ����������
        TextureEXT(const std::vector<std::string>& _paths,const std::string& _name = "");
        // Cube 6�������
        TextureEXT(const VkExtent3D& _extent, VkFormat _format, VkImageTiling _tiling, VkImageUsageFlags _usage, VkMemoryPropertyFlags _prop, VkImageLayout _layout, uint32_t _layer_count, VkSampleCountFlags _sample_count, const std::string& _name, uint32_t _mip_map = 1);
        // ���ƹ���
        TextureEXT(const TextureEXT& _tex);
        // ����ָ����С�Ŀ�ͼ��
        TextureEXT(const VkExtent3D& _extent, VkFormat _format, VkImageTiling _tiling, VkImageUsageFlags _usage, VkMemoryPropertyFlags _prop, VkImageLayout _layout, VkSampleCountFlags _sample_count = VK_SAMPLE_COUNT_1_BIT, const std::string& _name = "");
        // ������һ������sampler�Ŀղ�
        TextureEXT(VkSampler _sampler,bool _only_sampler = false);
        // ���ļ��м���
        TextureEXT(std::string& _path,const std::string& _name = "");
        // ��ֵ������
        TextureEXT& operator=(const TextureEXT& _tex);

        ~TextureEXT();

        // �Կ�ͼ���������
        void copyData(void* data);
        // ����vkͼ��
        void copyTextureEXT(TextureEXT* _tex);
        // ����������Ϣ
        void copyAllTextureEXT(TextureEXT* _src);
        // ���¼���
        void reload(std::string& _path);
        // �����Ƿ�����˼���
        inline bool load(std::string& _path) { loadDataFromFile(_path); return m_only_sampler; }
        // ��MaterialEditor�м���
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
        // ���Texture��Array�����Ϊÿ��array ����һ��view
        std::vector<VkImageView> m_image_view_array;
    protected:
        void initTemplate() override;
    private:
        uint32_t m_mipLevels = 0;
        bool m_only_sampler = false;
        // ���ļ��м���ͼ����ɲ��ֱ任
        TextureEXT* loadDataFromFile(std::string& _path);
        // ���ļ��м�����պ�
        TextureEXT* loadDataFromFiles(const std::vector<std::string>& _paths);

        // �Ƿ�������TextureEXT�ĸ���
        bool m_copy = false;
        bool m_is_cube = false;
        uint32_t m_layers = 1;

        // recreate ��
        VkImageAspectFlags m_aspect;

    public:
		inline static std::unordered_map<std::string, VkDescriptorSet> _sets;	// ·���� TexID
		inline static std::unordered_map<void*, VkDescriptorSet> _final_sets;	// TextureEXT, TexID
		// �����Ƿ��������
		inline bool hasTexture() { return !m_only_sampler; }
		// �Ƿ���Cube
		inline bool isCube() { return m_is_cube; }
		inline bool isArray() { return m_layers > 1 || m_is_cube; }
		virtual void recreate(const VkExtent3D& _extent);
		// ���û����_path���򷵻�Ĭ��1x1x1��ͼ�� 
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

