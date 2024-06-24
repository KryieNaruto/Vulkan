#pragma once
#include <Core/include/ZClass.h>
#include <ThirdParty/json/json.h>
#include <ThirdPartyManager/third_party_manager_global.h>
#include "resource_inner_global.h"
namespace Core::Resource {
    class ENGINE_API_RESOURCE ZResource :
        public ZClass
    {
    public:
        /** m_initialized = false,需要子类来初始化标志位 */
        ZResource();
        virtual ~ZResource();
        virtual Json::Value serializeToJSON() = 0;
        virtual void deserializeToObj(Json::Value& root) = 0;
        virtual inline std::string& getJsonTitle() { return this->m_json_title; }
        RObjectProperty* getProperty() { return p_property; }
        inline auto getTemplate() {return p_template;}
        inline auto& getInputSlotTemplate() { if(p_template) return p_template->p_inputs;}
        inline auto& getOutputSlotTemplate() { if(p_template) return p_template->p_outputs;}
        // 从节点初始化物体
        virtual void initObjFromTemplate();
    private:
    protected:
        std::string m_res_name = "";
        std::string m_json_title = "ZResource";
        std::string getResourcePath(const std::string& _path);
		ThirdParty::VkManager& m_vk_manager = ThirdParty::vk_manager;
		ThirdParty::Core::ApplicationVulkanInfo& m_vkInfo = ThirdParty::Core::vkInfo;
        RObjectProperty* p_property = nullptr;
        // 初始化节点，如inputs/ outputs
        virtual void initTemplate();
    public:  // Material Editor
        ThirdParty::imgui::Template* p_template = nullptr;

    };
}

