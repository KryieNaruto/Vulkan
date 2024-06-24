#include "RShader.h"
#include <ThirdPartyManager/third_party_manager_global.h>
#include <Core/include/core_util_function.h>
#include <Core/include/core_global.h>
#include <fstream>

Core::Resource::RShader::RShader(const std::string &_shaderName /*= "Shader"*/) {
    this->m_res_name = _shaderName;
    this->m_json_title = "RShader";
    this->p_id_generate->init(RESOURCE_TYPE::SHADER);
    this->m_uuid = this->p_id_generate->generateUUID();
    this->m_initialized = true;

    Core::Core_Pool::addResource(m_uuid, this);
	initTemplate();
}

Json::Value Core::Resource::RShader::serializeToJSON() {
    Json::Value sub;
    sub["m_res_name"] = m_res_name;
    sub["m_json_title"] = m_json_title;
    Json::Value sub_sub;
    for (const auto &path: m_paths)
        sub_sub[path.first] = path.second;
    sub["m_paths"] = sub_sub;
    return sub;
}

void Core::Resource::RShader::deserializeToObj(Json::Value &_root) {
    this->m_res_name = _root["m_res_name"].asString();
    this->m_json_title = _root["m_json_title"].asString();
    Json::Value sub = _root["m_paths"];
    for (const auto &s: sub)
        this->m_paths.insert(std::make_pair(Core::RESOURCE_TYPE(s[0].asUInt()), s[1].asString()));
}

Core::Resource::RShader::~RShader() {
    for (auto &shader: m_shaderModules)
        ThirdParty::vk_manager.destroyShaderModule(shader.second);
}

std::vector<VkPipelineShaderStageCreateInfo> &Core::Resource::RShader::getShaderStages(
    const std::vector<std::string> &_entry_points) {
    std::vector<VkPipelineShaderStageCreateInfo> &_rel = m_shader_stages;
    if (!_rel.empty() && !m_reloading) return _rel;
    if (m_reloading) _rel.clear();
    uint32_t i = 0;
    for (const auto &[stage,mod]: m_shaderModules) {
        int index = i;
        if (index >= _entry_points.size()) index = 0;
        m_entry_points[stage] = _entry_points[index];
        VkPipelineShaderStageCreateInfo c = {VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO};
        c.flags = 0;
        c.pNext = 0;
        c.module = mod;
        c.stage = getShaderStageBit(stage);
        c.pName = m_entry_points[stage].c_str();
        _rel.push_back(c);
        i++;
    }

    return _rel;
}

VkShaderStageFlagBits Core::Resource::RShader::getShaderStageBit(RESOURCE_TYPE _type) {
    switch (_type) {
        case Core::VERTEX_SHADER:
            return VK_SHADER_STAGE_VERTEX_BIT;
        case Core::GEOMETRY_SHADER:
            return VK_SHADER_STAGE_GEOMETRY_BIT;
        case Core::FRAGMENT_SHADER:
            return VK_SHADER_STAGE_FRAGMENT_BIT;
        default:
            break;
    }
}

const char *Core::Resource::RShader::getShaderEntryPoint(RESOURCE_TYPE _type) {
    switch (_type) {
        case Core::VERTEX_SHADER:
            return "VS_main";
            break;
        case Core::FRAGMENT_SHADER:
            return "PS_main";
            break;
        default:
            break;
    }
}

Core::Resource::RShader *Core::Resource::RShader::compilerOrGetVulkanBinaries(
    const std::unordered_map<ThirdParty::ShaderType, std::string> &shaderSources) {
    auto &shader_data = m_vulkan_spirv_cache;
    shader_data.clear();
    for (auto &&[stage, source]: shaderSources) {
        // 保存路径信息
        auto path = getResourcePath(source);
        m_paths[enum_shaderType_to_RESOURCE_TYPE(stage)] = path;

        std::string shaderName = std::filesystem::path(path).replace_extension().filename().string();
        std::filesystem::path cachePath = getShaderCacheDirectory() + shaderName + "\\" +
                                          ThirdParty::getShaderTypeStr(stage) + ".spv";
        if (m_reloading) {
            if (!m_spirv_paths[enum_shaderType_to_RESOURCE_TYPE(stage)].empty())
                cachePath = m_spirv_paths[enum_shaderType_to_RESOURCE_TYPE(stage)];
        }
        std::ifstream in(cachePath, std::ios::in | std::ios::binary);
        auto &data = shader_data[enum_shaderType_to_RESOURCE_TYPE(stage)];
        // 如果已经编译，则直接加载,[不在重建状态]，否则编译
        if (in.is_open() && !m_reloading) {
            in.seekg(0, std::ios::end);
            auto size = in.tellg();
            in.seekg(0, std::ios::beg);

            data.resize(size / sizeof(uint32_t));
            in.read((char *) data.data(), size);
            m_success = true;
            m_spirv_paths[enum_shaderType_to_RESOURCE_TYPE(stage)] = cachePath.string();
        }
        // 编译当前文件
        else {
            if (!m_reloading) m_spirv_paths[enum_shaderType_to_RESOURCE_TYPE(stage)] = cachePath.string();
            data = ThirdParty::shader_compiler->compileVulkanShader(path, stage, cachePath.string());
            if (!data.empty()) m_success = true;
            else {
                m_success = false;
                return this;
            }
        }

        // 获取反射module
        if (m_success) {
            m_reflect_shaderModules[enum_shaderType_to_RESOURCE_TYPE(stage)] = ThirdParty::shader_compiler->
                    createReflectShaderModule(data);
        }
    }

    return this;
}

std::string Core::Resource::RShader::getShaderCacheDirectory() {
    if (Core::g_project_info)
        return Core::g_project_info->m_resource_path + "\\spirv\\";
    else {
        const auto &path = m_paths[Core::RESOURCE_TYPE::VERTEX_SHADER];
        auto str = path.substr(0, path.find_last_of('\\'));
        str = str.substr(0, str.find_last_of("\\"));
        return str + "\\spirv\\";
    }
}

Core::RESOURCE_TYPE Core::Resource::RShader::enum_shaderType_to_RESOURCE_TYPE(ThirdParty::ShaderType _type) {
    switch (_type) {
        case ThirdParty::vertex:
            return RESOURCE_TYPE::VERTEX_SHADER;
        case ThirdParty::geometry:
            return RESOURCE_TYPE::GEOMETRY_SHADER;
        case ThirdParty::fragment:
            return RESOURCE_TYPE::FRAGMENT_SHADER;
        default:
            return RESOURCE_TYPE::UNDEFINED;
    }
}

Core::Resource::RShader *Core::Resource::RShader::createModules() {
    for (const auto &[stage, binary]: m_vulkan_spirv_cache) {
        const auto &module = ThirdParty::vk_manager.createShaderModule(binary);
        if (this->m_shaderModules.find(stage) != this->m_shaderModules.end()) {
            Core::info("Shader module %p has been loaded", &binary[0]);
        }
        m_shaderModules[stage] = module;
    }
    return this;
}

void Core::Resource::RShader::reflectShaderModule() {
    for (const auto &[stage, binary]: m_vulkan_spirv_cache)
        ThirdParty::shader_compiler->reflectInfo(getShaderFileName(stage), binary);
}

std::string Core::Resource::RShader::getShaderFileName(RESOURCE_TYPE _type) {
    auto _path_ite = m_paths.find(_type); if (_path_ite == m_paths.end()) return "Invaild Shader Path: is NULL!";
    return std::filesystem::path(_path_ite->second).filename().string();
}

void Core::Resource::RShader::initTemplate() {
    // Template
    if (p_template) {
        p_template->m_header_color = IM_COL32(255, 165, 0, 200);
        //Input
        auto _input_0 = new ThirdParty::imgui::SlotTemplateInput;
        _input_0->setName("Vertex Shader");
        _input_0->setName2("vertex");
        auto _input_1 = new ThirdParty::imgui::SlotTemplateInput;
        _input_1->setName("Geometry Shader");
        _input_1->setName2("geometry");
        auto _input_2 = new ThirdParty::imgui::SlotTemplateInput;
        _input_2->setName("Fragment Shader");
        _input_2->setName2("fragment");
        p_template->addInputSlotTemplates({ _input_0,_input_1,_input_2 });
    }
}

void Core::Resource::RShader::bind(VkCommandBuffer _cmd, VkPipeline _pipeline,
                                   const std::vector<VkDescriptorSet> &_sets) {
}

Core::RESOURCE_TYPE Core::Resource::RShader::string_to_RESOURCE_TYPE(const std::string& _type)
{
    if (_type == "vertex")
        return RESOURCE_TYPE::VERTEX_SHADER;
    if (_type == "geometry")
        return RESOURCE_TYPE::GEOMETRY_SHADER;
    if (_type == "fragment")
        return RESOURCE_TYPE::FRAGMENT_SHADER;
}
