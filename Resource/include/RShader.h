#pragma once
#include "ZResource.h"
#include "resource_inner_global.h"
#include <ThirdParty/vulkan/vulkan.h>
#include <vector>
#include <map>
#include <unordered_map>
#include <ThirdPartyManager/third_party_manager_global.h>
#include <ThirdParty/spirv-reflect/spirv_reflect.h>
namespace Core::Resource {

    /*
    // RShader
    // ShaderԴ�ļ��ĵ�һ�ζ�ȡ����
    /// <Node����>
    /// n Inputs
            Vertex Shader: const char*
            Fragment Shader: const char*
    /// 2 Outputs
            RShader: RShader*
            Shader Stage: std::vector<VkPipelineShaderStageCreateInfo>&
    */
    class ENGINE_API_RESOURCE RShader :
        public ZResource
    {
    public:
        RShader(const std::string& _shaderName = "Shader");
        // ���ر���glsl�ļ�Ϊspirv,shader·��������·��/resource�µ����·�� 
        RShader* compilerOrGetVulkanBinaries(const std::unordered_map<ThirdParty::ShaderType, std::string>& shaderSources);
        // ����shaderModule
        RShader* createModules();
        // Ĭ����ڵ�Ϊmain
        std::vector<VkPipelineShaderStageCreateInfo>& getShaderStages(const std::vector<std::string>& _entry_points = {"main"});
        // ��ȡ�ļ���,Ĭ�Ϸ��ض�����ɫ������
        inline std::string getFileName() { return getShaderFileName(VERTEX_SHADER); }
        inline std::string getName() { return m_res_name; }
        virtual void bind(VkCommandBuffer _cmd, VkPipeline _pipeline, const std::vector<VkDescriptorSet>& _sets);

        virtual ~RShader();
        virtual Json::Value serializeToJSON() override;
        virtual void deserializeToObj(Json::Value& _root) override;
    public:
        // DEBUG: ����shader����
        void reflectShaderModule();
    protected:
        bool m_success = false;   // ������
        std::vector<VkPipelineShaderStageCreateInfo> m_shader_stages;
        std::unordered_map<RESOURCE_TYPE, std::vector<uint32_t>> m_vulkan_spirv_cache;
        std::unordered_map<RESOURCE_TYPE, std::string> m_paths;         // Դ�ļ�·����δ������
        std::unordered_map<RESOURCE_TYPE, std::string> m_spirv_paths;   // SPIRV�ļ�·����������
        std::unordered_map<RESOURCE_TYPE, VkShaderModule> m_shaderModules;
        std::unordered_map<RESOURCE_TYPE, SpvReflectShaderModule> m_reflect_shaderModules;  // ����module
        std::unordered_map<RESOURCE_TYPE, std::string> m_entry_points;  // ��ڵ�

		// ThirdParty::ShaderType ->  Core::RESOURCE_TYPE
        RESOURCE_TYPE enum_shaderType_to_RESOURCE_TYPE(ThirdParty::ShaderType _type);
        // string -> Core::RESOURCE_TYPE
        RESOURCE_TYPE string_to_RESOURCE_TYPE(const std::string& _type);
        // ��ȡspv����·��
        std::string getShaderCacheDirectory();
        // ��ȡ�ļ���
        std::string getShaderFileName(RESOURCE_TYPE _type);
        void initTemplate() override;

        // �����ؽ�
        bool m_reloading = false;
    public:
        VkShaderStageFlagBits getShaderStageBit(RESOURCE_TYPE _type);
        const char* getShaderEntryPoint(RESOURCE_TYPE _type);
    };

}

