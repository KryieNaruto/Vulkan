#pragma once
#include "ZResource.h"
#include "Shader.h"
#include <ThirdPartyManager/third_party_manager_global.h>
#include <Core/include/core_global.h>
#include "RTexture.h"
#include <set>

namespace Core::Resource {
    
    class ENGINE_API_RESOURCE RMaterial :
        public ZResource
    {
    public:
        RMaterial(const std::string& _name = "RMaterial");
		virtual ~RMaterial();
		virtual Json::Value serializeToJSON() override;
		virtual void deserializeToObj(Json::Value& _root) override;

        // 重新加载shader
        RMaterial* reload();

        RMaterial* setShader(Shader* _shader);
        Shader* getShader() { return p_shader; }
        // 绑定DescriptorSet
        // _update_solt 表示是否第一次更新Set,如果存在多个Mesh共用Set,则只能更新第一个Mesh的Set，此时_update_solt置为false
        RMaterial* bind(VkCommandBuffer _cmd,bool _update_solt = true);

        // 返回纹理数量
        uint32_t& getTextureCount() { return m_texture_count; }
        // RTexture是纹理文件列表，m_textures是只有Sampler的Texture槽，这个函数是加载文件到vk
        RMaterial* bindTextures(const std::vector<RTexture*>& _texs);
        // 返回location attribute stride的计算结果
        size_t getVertexInputStride();
        // 与VERTEX_INPUT_FLAG对应
        VERTEX_INPUT_FLAG getVertexInputFlag();
        inline static bool m_reloading = false;
        // 从全局获取Sampler
        static ThirdParty::Sampler* getSampler(const std::string& _name = SAMPLER);
        inline const std::string& getName() { return m_res_name; }
        inline const std::vector<RTexture*>& getTextures() { return p_reload_textures; }
		inline bool& isReady() { return m_is_ready; }
    protected:
        Shader* p_shader = nullptr;
        // 根据shader 创建ubo 和descriptorSet
        // 默认启用Bindless，着色器纹理作为最后的Uniform
        void setup(bool _reload = false);
    protected:
		bool m_is_ready = false;
        // RModel 根据RMaterial 通过Shader 获取的vertex input格式创建vbo
		size_t m_vertex_input_stride = 0;
		VERTEX_INPUT_FLAG m_vertex_input_flag = (VERTEX_INPUT_FLAG)0;
        Shader* p_shader_old = nullptr;
        // 纹理数量
        uint32_t m_texture_count = 0;
        // vkDescriptorSet
        std::vector<VkDescriptorSet> m_descriptor_sets_current_frame;
        // 重建用 Texture
        std::vector<RTexture*> p_reload_textures;

    protected:    // API 1.2
        // VkSet的具体实例
        // SLOT[Set ID]
        std::unordered_map<uint32_t ,ThirdParty::Slot*> p_slots;
    public:     // API 1.2
		// API 1.2 通过SLOT 绑定数据
        RMaterial* bindData(uint32_t _set_id, uint32_t _binding_id, void* data, uint32_t index = 0);
        RMaterial* bindData(uint32_t _set_id, uint32_t _binding_id, VkSampler _sampler, uint32_t index = 0);
        RMaterial* bindData(uint32_t _set_id, uint32_t _binding_id, std::string _member_name, void* data, uint32_t _index = 0);
        RMaterial* bindData(uint32_t _set_id, const std::string& _binding_name, uint32_t _member_id, void* data, uint32_t _index = 0);
        RMaterial* bindData(uint32_t _set_id, uint32_t _binding_id, uint32_t _member_id, void* _data, uint32_t _index = 0);
        RMaterial* bindTexture(uint32_t _set_id, uint32_t _binding_id, ThirdParty::TextureEXT* _tex, uint32_t index = 0);

        // 通过 ZResource 继承
        void initTemplate() override;
    };

}

