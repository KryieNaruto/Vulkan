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

        // ���¼���shader
        RMaterial* reload();

        RMaterial* setShader(Shader* _shader);
        Shader* getShader() { return p_shader; }
        // ��DescriptorSet
        // _update_solt ��ʾ�Ƿ��һ�θ���Set,������ڶ��Mesh����Set,��ֻ�ܸ��µ�һ��Mesh��Set����ʱ_update_solt��Ϊfalse
        RMaterial* bind(VkCommandBuffer _cmd,bool _update_solt = true);

        // ������������
        uint32_t& getTextureCount() { return m_texture_count; }
        // RTexture�������ļ��б�m_textures��ֻ��Sampler��Texture�ۣ���������Ǽ����ļ���vk
        RMaterial* bindTextures(const std::vector<RTexture*>& _texs);
        // ����location attribute stride�ļ�����
        size_t getVertexInputStride();
        // ��VERTEX_INPUT_FLAG��Ӧ
        VERTEX_INPUT_FLAG getVertexInputFlag();
        inline static bool m_reloading = false;
        // ��ȫ�ֻ�ȡSampler
        static ThirdParty::Sampler* getSampler(const std::string& _name = SAMPLER);
        inline const std::string& getName() { return m_res_name; }
        inline const std::vector<RTexture*>& getTextures() { return p_reload_textures; }
		inline bool& isReady() { return m_is_ready; }
    protected:
        Shader* p_shader = nullptr;
        // ����shader ����ubo ��descriptorSet
        // Ĭ������Bindless����ɫ��������Ϊ����Uniform
        void setup(bool _reload = false);
    protected:
		bool m_is_ready = false;
        // RModel ����RMaterial ͨ��Shader ��ȡ��vertex input��ʽ����vbo
		size_t m_vertex_input_stride = 0;
		VERTEX_INPUT_FLAG m_vertex_input_flag = (VERTEX_INPUT_FLAG)0;
        Shader* p_shader_old = nullptr;
        // ��������
        uint32_t m_texture_count = 0;
        // vkDescriptorSet
        std::vector<VkDescriptorSet> m_descriptor_sets_current_frame;
        // �ؽ��� Texture
        std::vector<RTexture*> p_reload_textures;

    protected:    // API 1.2
        // VkSet�ľ���ʵ��
        // SLOT[Set ID]
        std::unordered_map<uint32_t ,ThirdParty::Slot*> p_slots;
    public:     // API 1.2
		// API 1.2 ͨ��SLOT ������
        RMaterial* bindData(uint32_t _set_id, uint32_t _binding_id, void* data, uint32_t index = 0);
        RMaterial* bindData(uint32_t _set_id, uint32_t _binding_id, VkSampler _sampler, uint32_t index = 0);
        RMaterial* bindData(uint32_t _set_id, uint32_t _binding_id, std::string _member_name, void* data, uint32_t _index = 0);
        RMaterial* bindData(uint32_t _set_id, const std::string& _binding_name, uint32_t _member_id, void* data, uint32_t _index = 0);
        RMaterial* bindData(uint32_t _set_id, uint32_t _binding_id, uint32_t _member_id, void* _data, uint32_t _index = 0);
        RMaterial* bindTexture(uint32_t _set_id, uint32_t _binding_id, ThirdParty::TextureEXT* _tex, uint32_t index = 0);

        // ͨ�� ZResource �̳�
        void initTemplate() override;
    };

}

