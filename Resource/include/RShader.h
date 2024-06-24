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
    // Shader源文件的第一次读取编译
    /// <Node属性>
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
        // 加载编译glsl文件为spirv,shader路径：绝对路径/resource下的相对路径 
        RShader* compilerOrGetVulkanBinaries(const std::unordered_map<ThirdParty::ShaderType, std::string>& shaderSources);
        // 创建shaderModule
        RShader* createModules();
        // 默认入口点为main
        std::vector<VkPipelineShaderStageCreateInfo>& getShaderStages(const std::vector<std::string>& _entry_points = {"main"});
        // 获取文件名,默认返回顶点着色器名称
        inline std::string getFileName() { return getShaderFileName(VERTEX_SHADER); }
        inline std::string getName() { return m_res_name; }
        virtual void bind(VkCommandBuffer _cmd, VkPipeline _pipeline, const std::vector<VkDescriptorSet>& _sets);

        virtual ~RShader();
        virtual Json::Value serializeToJSON() override;
        virtual void deserializeToObj(Json::Value& _root) override;
    public:
        // DEBUG: 反射shader内容
        void reflectShaderModule();
    protected:
        bool m_success = false;   // 编译结果
        std::vector<VkPipelineShaderStageCreateInfo> m_shader_stages;
        std::unordered_map<RESOURCE_TYPE, std::vector<uint32_t>> m_vulkan_spirv_cache;
        std::unordered_map<RESOURCE_TYPE, std::string> m_paths;         // 源文件路径，未被编译
        std::unordered_map<RESOURCE_TYPE, std::string> m_spirv_paths;   // SPIRV文件路径，被编译
        std::unordered_map<RESOURCE_TYPE, VkShaderModule> m_shaderModules;
        std::unordered_map<RESOURCE_TYPE, SpvReflectShaderModule> m_reflect_shaderModules;  // 反射module
        std::unordered_map<RESOURCE_TYPE, std::string> m_entry_points;  // 入口点

		// ThirdParty::ShaderType ->  Core::RESOURCE_TYPE
        RESOURCE_TYPE enum_shaderType_to_RESOURCE_TYPE(ThirdParty::ShaderType _type);
        // string -> Core::RESOURCE_TYPE
        RESOURCE_TYPE string_to_RESOURCE_TYPE(const std::string& _type);
        // 获取spv缓存路径
        std::string getShaderCacheDirectory();
        // 获取文件名
        std::string getShaderFileName(RESOURCE_TYPE _type);
        void initTemplate() override;

        // 重载重建
        bool m_reloading = false;
    public:
        VkShaderStageFlagBits getShaderStageBit(RESOURCE_TYPE _type);
        const char* getShaderEntryPoint(RESOURCE_TYPE _type);
    };

}

