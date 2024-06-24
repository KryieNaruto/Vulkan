#pragma once
#include "scene_obj.h"
#include <Resource/resource_global.h>
#include <unordered_map>
#include <set>
namespace Core::Resource {

    // ǰ������ 
    class ENGINE_API_SCENE Scene;

class ENGINE_API_SCENE Material :
    public SceneObject,
    public RMaterial
{
public:
    /// <summary>
    /// ���ɵĽṹ��ڵ㣬key �� this��         Value �� �ṹ����initTemplate����, ����CommonStruct ���ڷ������Struct��input
    /// </summary>
    inline static std::unordered_map<Material*, std::unordered_map<std::string, std::function<Core::CommonStruct*(void)>>> g_struct_nodes;
    /// ���ɵĽṹ��ڵ㣬key �� this��         Value �� �ṹ����initTemplate����, ����DataStruct ���ڷ������Struct��input
    inline static std::unordered_map<Material*, std::unordered_map<std::string, std::function<Core::DataStruct*(void)>>> g_datas_nodes;
    inline static std::unordered_map<Shader*, std::set<Material*>> g_shader_materials;
    inline static const auto& getShaderMaterials() { return g_shader_materials; }
    static void setShaderMaterial(Shader* _shader, Material* _mat);
    static void reloadMaterial(Shader* _shader);
public:
    // �ղ��ʣ����ڲ��ʱ༭����creating material
    Material();
    Material(RMaterial* _rmat);
    Material(Shader* _shader,const std::string& _name = "Material");
    virtual ~Material();
    virtual void propertyEditor() override;

    Material* addEmptyTexture(uint32_t _set_index, const std::string& _binding_name);
    ThirdParty::SlotData* getBiniding(uint32_t _set_index, uint32_t _binding_index);
    ThirdParty::SlotData* getBiniding(uint32_t _set_index, const std::string& _binding_name);

    std::vector<VkDescriptorSet> m_preview; // Ԥ�����ʵ�������ȾĿ��;
public:
    inline Shader* getShader() { return p_shader; }
    inline auto getSlots() { return p_slots; }

private:
    void initTemplate() override;
    bool m_hover = false;   // ���ڲ�����ͼ����������
public:
    // material editorר�����������ڽ��ڵ㴴��Ϊdraw func;
    void initDrawFunc();
    void initObjFromTemplate() override;
    inline bool isHover() { return m_hover; }
};

}
