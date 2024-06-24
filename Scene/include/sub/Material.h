#pragma once
#include "scene_obj.h"
#include <Resource/resource_global.h>
#include <unordered_map>
#include <set>
namespace Core::Resource {

    // 前向声明 
    class ENGINE_API_SCENE Scene;

class ENGINE_API_SCENE Material :
    public SceneObject,
    public RMaterial
{
public:
    /// <summary>
    /// 生成的结构体节点，key ： this；         Value ： 结构体与initTemplate函数, 返回CommonStruct 用于反向更新Struct的input
    /// </summary>
    inline static std::unordered_map<Material*, std::unordered_map<std::string, std::function<Core::CommonStruct*(void)>>> g_struct_nodes;
    /// 生成的结构体节点，key ： this；         Value ： 结构体与initTemplate函数, 返回DataStruct 用于反向更新Struct的input
    inline static std::unordered_map<Material*, std::unordered_map<std::string, std::function<Core::DataStruct*(void)>>> g_datas_nodes;
    inline static std::unordered_map<Shader*, std::set<Material*>> g_shader_materials;
    inline static const auto& getShaderMaterials() { return g_shader_materials; }
    static void setShaderMaterial(Shader* _shader, Material* _mat);
    static void reloadMaterial(Shader* _shader);
public:
    // 空材质，用于材质编辑器的creating material
    Material();
    Material(RMaterial* _rmat);
    Material(Shader* _shader,const std::string& _name = "Material");
    virtual ~Material();
    virtual void propertyEditor() override;

    Material* addEmptyTexture(uint32_t _set_index, const std::string& _binding_name);
    ThirdParty::SlotData* getBiniding(uint32_t _set_index, uint32_t _binding_index);
    ThirdParty::SlotData* getBiniding(uint32_t _set_index, const std::string& _binding_name);

    std::vector<VkDescriptorSet> m_preview; // 预览材质的最终渲染目标;
public:
    inline Shader* getShader() { return p_shader; }
    inline auto getSlots() { return p_slots; }

private:
    void initTemplate() override;
    bool m_hover = false;   // 用于材质视图的鼠标操作；
public:
    // material editor专属函数，用于将节点创建为draw func;
    void initDrawFunc();
    void initObjFromTemplate() override;
    inline bool isHover() { return m_hover; }
};

}
