#pragma once
#include "vk_object.h"
#include <spirv-reflect/spirv_reflect.h>
#include "vk_uniform_buffer.h"
#include "vk_texture_ext.h"
#include <variant>
#include <set>

namespace ThirdParty {

    /**
     * Binding��ӳ��
     */
    class ENGINE_API_THIRDPARTY SlotData:
        public vkObject
    {
    public:
        SlotData(SpvReflectDescriptorBinding* _binding_info);
        SlotData(SlotData* _slot_data);
        ~SlotData();

        // ���Struct member ,_index :��������
        // ����������
        SlotData* bindData(std::string _member, void* _data, uint32_t _index = 0);
        // ��λ������
        SlotData* bindData(uint32_t _member_index, void* _data, uint32_t _index = 0);
        // �Ե���λ��Ϊ����
        SlotData* bindData(void* _data, uint32_t _index = 0);
        // ������,����������ΪSampler ����nullptr
        SlotData* bindData(std::string _img_path, uint32_t _index = 0);
        SlotData* bindData(TextureEXT* _tex_ext, uint32_t _index);
        SlotData* bindData(VkSampler _sampler, uint32_t _index);

        // ����UBO /TEX���ݣ� updateSet ��Slot����
        void update();
        const VkWriteDescriptorSet& getVkWriteDescriptorSet(VkDescriptorSet _dst_set);
        // ��������Ƿ���Texture
        bool isTexture();
        // ��������Ƿ���Uniform
        bool isUniformBuffer();
        // ����Ƿ������
        bool isEmpty();
        bool isEmpty(bool _set_empty);
        bool isCube();
        // �����Ƿ�ı�
        bool isChange();
        // �趨Stage
        SlotData* setShaderStage(VkShaderStageFlags _stage) { m_set_layout_binding.stageFlags |= _stage; return this; }
        // ��ȡ�ı�����������������
        std::set<uint32_t> getChangeIndex();
    protected:
        std::string m_binding_name;
        uint32_t m_binding_id;
        VkDescriptorSetLayoutBinding m_set_layout_binding;
        VkDescriptorBindingFlagsEXT m_binding_flags_ext = 0;
        // ���ݲ�� [Binding Name, UBO]
        std::pair<std::string, std::vector<UniformBuffer*>> p_uniform_buffer;
        // ���ݲ�� [Binding Name, Texture]
        std::pair<std::string, std::vector<TextureEXT*>> p_texture_ext;
        // Write infos
        std::vector<VkWriteDescriptorSet*> p_write_infos;
        // Buffer/Image Info
        std::variant<std::vector<VkDescriptorBufferInfo>, std::vector<VkDescriptorImageInfo>> m_infos;
    private:
        // binding ��������
        SpvReflectDescriptorBinding* p_binding_property = nullptr;
        // ��������
        VkDescriptorType m_descriptor_type = VK_DESCRIPTOR_TYPE_SAMPLER;
        // �Ƿ�������
        bool m_is_array = false;
        // �ṹ���С
        uint32_t m_block_size = 0;
        uint32_t m_block_padding_size = 0;
        uint32_t m_block_member_count = 0;
        // �ṹ������
        // [Struct Name] [Member Name] [size,offset]
        std::pair<std::string,
        std::unordered_map<std::string, std::pair<size_t,size_t>>> m_struct_property;
        // ��¼˳��
        std::pair<std::string,
            std::unordered_map<int, std::string>> m_struct_property_order;
        // �ṹ���Ա����
        // [MemberName][TypeName]
        std::unordered_map<std::string, std::string> m_struct_type_name;

        // Struct����
        std::vector<void*> p_datas;
        bool m_is_copy = false;
        bool m_need_data = false;        // ��ǰ��������Ƿ�Ϊ�գ���Ҫ����Texture�Ĳ���
        bool m_is_cube = false;
        std::set<uint32_t> m_texture_change_index;   // ����ı�����������Ϊ�գ���û�������޸�
        uint32_t m_member_index = 0;    // ��ǰ���õ�λ������
    private:
        void setup();
        // ����UBO TEX...
        void setupData();
        // ��ȡSpvOpType��ʵ��ӳ�����ͣ���Ҫ��ȡ��������
        std::string getSpvOpType2String(SpvOp _OpType);
    public:
        inline const VkDescriptorType& getVkDEscriptorType() { return m_descriptor_type; }
        inline const VkShaderStageFlags& getVkShaderStage() { return m_set_layout_binding.stageFlags; }
        inline const VkDescriptorSetLayoutBinding getVkDescriptorSetLayoutBinding() { return m_set_layout_binding; }
		inline const std::pair<std::string,std::unordered_map<std::string, std::pair<size_t, size_t>>>& getMemberStruct() 
        { return m_struct_property; }
        inline const std::pair<std::string, std::vector<TextureEXT*>>& getTextureEXT() { return p_texture_ext; }
        inline std::string& getBindingName() { return m_binding_name; }
        inline std::string& getMemberType(const std::string& _member) { return m_struct_type_name[_member]; }
        inline const uint32_t getBinding() { return m_binding_id; }
        inline const uint32_t getDescriptorCount() { return m_set_layout_binding.descriptorCount; }
        const std::pair<std::string, std::vector<std::pair<std::string, std::pair<size_t, size_t>>>> getMemberStructOrdered();
        const uint32_t getBindingCount();
        // ���ص�ǰ����������������
        inline const uint32_t getTextureIndex() { return p_texture_ext.second.size(); }
        // ���ص�ǰ��������
        inline const uint32_t getTextureCount() { return p_texture_ext.second.size(); }
        inline const bool isArray() { return m_is_array; }
        inline VkDescriptorBindingFlagsEXT& getVkDescriptorBindingFlagsEXT() { return m_binding_flags_ext; }
    };

}

