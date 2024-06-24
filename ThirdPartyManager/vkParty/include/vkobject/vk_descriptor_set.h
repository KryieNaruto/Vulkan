#pragma once
#include "vk_object.h"
#include <variant>

namespace ThirdParty {
    
    /************************************************************************/
    // DescriptorSet ����������������Դ��                                 
    // ��֡����
    // һ��Set�������Binding,һ��Binding��Ӧһ��ubo ��sampler����texture�������֧��Bindless������Ч��
    // һ��Set�������Binding,һ��Binding��Ӧ���ubo ��sampler��texture 
    /************************************************************************/

    class ENGINE_API_THIRDPARTY DescriptorSet :
        public vkObject
    {
    public:

        // ������ �����и���
        // ��Array����
        // VkDescriptorBufferInfo �е�offset�ǻ���buffer
        DescriptorSet* bindData(
            const size_t& _binding, 
            const VkDescriptorType& _type, 
            const std::variant<std::vector<VkDescriptorBufferInfo>, std::vector<VkDescriptorImageInfo>>& _infos);
    public:
        DescriptorSet(VkDescriptorSetLayout _layout, uint32_t _descriptorCount);
        DescriptorSet(VkDescriptorSetLayout _layout, const std::string& _label);
        virtual ~DescriptorSet();
        inline const VkDescriptorSet& getVkDescriptorSet(const int& _frame = -1)
        { return m_descriptor_sets[_frame < 0 ? m_vkInfo.m_current_frame : _frame]; }
    private:
        std::vector<VkDescriptorSet> m_descriptor_sets;
        VkDescriptorSetAllocateInfo* p_alloc_info = nullptr;

        // key : binding
        // param : type
        std::map<uint32_t, VkDescriptorType> m_types;

        // key : binding
        // param : descriptors[m_frame_count]
        std::map<uint32_t, std::vector<
            std::variant<
            std::vector<VkDescriptorBufferInfo>,
            std::vector<VkDescriptorImageInfo>
            >
            >> m_infos;
        std::map<uint32_t,std::vector<VkWriteDescriptorSet>> m_write_info;
    };

}

