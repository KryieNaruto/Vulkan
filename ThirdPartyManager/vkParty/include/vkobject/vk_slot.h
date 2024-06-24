#pragma once
#include "vk_object.h"
#include <spirv-reflect/spirv_reflect.h>
#include "vk_slot_data.h"
#include <set>

namespace ThirdParty {
    /**
     * Shader 数据插槽 Set的数据映射
     * 包含Uniform/ Texture
     * 通过vk spirv 反射信息自动生成
     * 包含创建UBO 结构体
     * 
     * 以Shader阶段分配
     * 
     * 超究极无敌集大成之作
     */
    class ENGINE_API_THIRDPARTY Slot:
        public vkObject
    {
    public:
        Slot(SpvReflectDescriptorSet* _reflect_info);
        Slot(Slot* _slot);
        ~Slot();

        // 更新vk set
        void update(VkCommandBuffer _cmd);
        
    private:
        std::vector<VkDescriptorSet> m_sets;
        // set id
        uint32_t m_set_id;

        // 插槽数据
		/// [Binding Name] [Data]
		std::unordered_map<std::string, SlotData*> p_slot_datas;
        // [Binding Name] [Binding ID]
        std::map<uint32_t,std::string> p_binding_name;
        // SlotData中最大数组 [只有数组情况下生效]
        uint32_t m_max_descriptor_count = 0;
        uint32_t m_max_descriptor_count_last = 0;
        // SlotData中是否有数组
        bool m_has_array = false;
        /// VkDescriptorSetLayoutBinding
        std::vector<VkDescriptorSetLayoutBinding> m_descriptor_set_layout_bindings;
        /// Slot的VkDescriptorBindingFlagsEXT
        std::vector<VkDescriptorBindingFlagsEXT> m_descriptor_binding_flags_ext;

        // Set的反射信息
        SpvReflectDescriptorSet* p_reflect_descriptor_set_info = nullptr;

        /// binding 属性
        /// [Binding Name] [Binding Property]
        std::unordered_map<std::string, SpvReflectDescriptorBinding*> p_binding_properties;

        VkDescriptorSetLayoutCreateInfo* p_create_info = nullptr;
        VkDescriptorSetLayoutBindingFlagsCreateInfoEXT* p_setLayoutBindingFlags = nullptr;
        VkDescriptorSetLayout m_descriptor_layout = VK_NULL_HANDLE;
        // 是否析构
        bool m_is_copy = false;
    private:
        // 解析反射信息, 将binding信息析出
        void setupSpvReflect();
        // 详细解析Binding
        void setupBindingProperty();
        // 创建descriptorSetLayout
        void setupSetLayout();
        // 创建Set
        void setupSet();

    public:
        inline const VkDescriptorSetLayout& getVkDescriptorSetLayout() { return m_descriptor_layout; }
		inline const VkDescriptorSet& getVkDescriptorSet() { return m_sets[m_vkInfo.m_current_frame]; }
        inline const uint32_t& getSetID() { return m_set_id; }
        inline SlotData* getSlotData(uint32_t _binding_id) { return p_slot_datas[p_binding_name[_binding_id]]; }
        inline SlotData* getSlotData(const std::string& _binding_name) { return p_slot_datas[_binding_name]; }
        inline const std::string& getBindingName(int _binding_id) { return p_binding_name[_binding_id]; }
        inline const std::unordered_map<std::string, SlotData*>& getSlotDatas() { return p_slot_datas; }
        inline const std::map<uint32_t,std::string>& getBindingNames() { return p_binding_name; }
        // 设置binding的ShaderStage 
		Slot* setShaderStage(uint32_t _binding, VkShaderStageFlags _stage);
		Slot* setShaderStage(std::string _binding_name, VkShaderStageFlags _stage);
        // 添加新插槽
        Slot* addSlotData(SpvReflectDescriptorBinding* _binding_info);
        // 重建DescriptorSetLayout
        // 在使用addSlotData和setShaderStage函数后使用
        Slot* recreateDescriptorSetLayout();
        Slot* createDescriptorSetLayout();
        // 返回所有Binding ，方便重复检测
        std::set<uint32_t> getBindingIndices();
    };
}

