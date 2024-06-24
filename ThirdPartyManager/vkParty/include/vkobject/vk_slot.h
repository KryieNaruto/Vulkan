#pragma once
#include "vk_object.h"
#include <spirv-reflect/spirv_reflect.h>
#include "vk_slot_data.h"
#include <set>

namespace ThirdParty {
    /**
     * Shader ���ݲ�� Set������ӳ��
     * ����Uniform/ Texture
     * ͨ��vk spirv ������Ϣ�Զ�����
     * ��������UBO �ṹ��
     * 
     * ��Shader�׶η���
     * 
     * �������޵м����֮��
     */
    class ENGINE_API_THIRDPARTY Slot:
        public vkObject
    {
    public:
        Slot(SpvReflectDescriptorSet* _reflect_info);
        Slot(Slot* _slot);
        ~Slot();

        // ����vk set
        void update(VkCommandBuffer _cmd);
        
    private:
        std::vector<VkDescriptorSet> m_sets;
        // set id
        uint32_t m_set_id;

        // �������
		/// [Binding Name] [Data]
		std::unordered_map<std::string, SlotData*> p_slot_datas;
        // [Binding Name] [Binding ID]
        std::map<uint32_t,std::string> p_binding_name;
        // SlotData��������� [ֻ�������������Ч]
        uint32_t m_max_descriptor_count = 0;
        uint32_t m_max_descriptor_count_last = 0;
        // SlotData���Ƿ�������
        bool m_has_array = false;
        /// VkDescriptorSetLayoutBinding
        std::vector<VkDescriptorSetLayoutBinding> m_descriptor_set_layout_bindings;
        /// Slot��VkDescriptorBindingFlagsEXT
        std::vector<VkDescriptorBindingFlagsEXT> m_descriptor_binding_flags_ext;

        // Set�ķ�����Ϣ
        SpvReflectDescriptorSet* p_reflect_descriptor_set_info = nullptr;

        /// binding ����
        /// [Binding Name] [Binding Property]
        std::unordered_map<std::string, SpvReflectDescriptorBinding*> p_binding_properties;

        VkDescriptorSetLayoutCreateInfo* p_create_info = nullptr;
        VkDescriptorSetLayoutBindingFlagsCreateInfoEXT* p_setLayoutBindingFlags = nullptr;
        VkDescriptorSetLayout m_descriptor_layout = VK_NULL_HANDLE;
        // �Ƿ�����
        bool m_is_copy = false;
    private:
        // ����������Ϣ, ��binding��Ϣ����
        void setupSpvReflect();
        // ��ϸ����Binding
        void setupBindingProperty();
        // ����descriptorSetLayout
        void setupSetLayout();
        // ����Set
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
        // ����binding��ShaderStage 
		Slot* setShaderStage(uint32_t _binding, VkShaderStageFlags _stage);
		Slot* setShaderStage(std::string _binding_name, VkShaderStageFlags _stage);
        // ����²��
        Slot* addSlotData(SpvReflectDescriptorBinding* _binding_info);
        // �ؽ�DescriptorSetLayout
        // ��ʹ��addSlotData��setShaderStage������ʹ��
        Slot* recreateDescriptorSetLayout();
        Slot* createDescriptorSetLayout();
        // ��������Binding �������ظ����
        std::set<uint32_t> getBindingIndices();
    };
}

