#pragma once
#include "vk_object.h"
#include <spirv-reflect/spirv_reflect.h>
#include "vk_uniform_buffer.h"
#include "vk_texture_ext.h"
#include <variant>
#include <set>

namespace ThirdParty {

    /**
     * Binding的映射
     */
    class ENGINE_API_THIRDPARTY SlotData:
        public vkObject
    {
    public:
        SlotData(SpvReflectDescriptorBinding* _binding_info);
        SlotData(SlotData* _slot_data);
        ~SlotData();

        // 填充Struct member ,_index :数组索引
        // 以名称索引
        SlotData* bindData(std::string _member, void* _data, uint32_t _index = 0);
        // 以位置索引
        SlotData* bindData(uint32_t _member_index, void* _data, uint32_t _index = 0);
        // 以调用位置为索引
        SlotData* bindData(void* _data, uint32_t _index = 0);
        // 绑定纹理,如果插槽类型为Sampler 返回nullptr
        SlotData* bindData(std::string _img_path, uint32_t _index = 0);
        SlotData* bindData(TextureEXT* _tex_ext, uint32_t _index);
        SlotData* bindData(VkSampler _sampler, uint32_t _index);

        // 更新UBO /TEX数据， updateSet 由Slot管理
        void update();
        const VkWriteDescriptorSet& getVkWriteDescriptorSet(VkDescriptorSet _dst_set);
        // 插槽属性是否是Texture
        bool isTexture();
        // 插槽属性是否是Uniform
        bool isUniformBuffer();
        // 插槽是否待插入
        bool isEmpty();
        bool isEmpty(bool _set_empty);
        bool isCube();
        // 纹理是否改变
        bool isChange();
        // 设定Stage
        SlotData* setShaderStage(VkShaderStageFlags _stage) { m_set_layout_binding.stageFlags |= _stage; return this; }
        // 获取改变纹理的索引，并清空
        std::set<uint32_t> getChangeIndex();
    protected:
        std::string m_binding_name;
        uint32_t m_binding_id;
        VkDescriptorSetLayoutBinding m_set_layout_binding;
        VkDescriptorBindingFlagsEXT m_binding_flags_ext = 0;
        // 数据插槽 [Binding Name, UBO]
        std::pair<std::string, std::vector<UniformBuffer*>> p_uniform_buffer;
        // 数据插槽 [Binding Name, Texture]
        std::pair<std::string, std::vector<TextureEXT*>> p_texture_ext;
        // Write infos
        std::vector<VkWriteDescriptorSet*> p_write_infos;
        // Buffer/Image Info
        std::variant<std::vector<VkDescriptorBufferInfo>, std::vector<VkDescriptorImageInfo>> m_infos;
    private:
        // binding 反射属性
        SpvReflectDescriptorBinding* p_binding_property = nullptr;
        // 数据类型
        VkDescriptorType m_descriptor_type = VK_DESCRIPTOR_TYPE_SAMPLER;
        // 是否是数组
        bool m_is_array = false;
        // 结构体大小
        uint32_t m_block_size = 0;
        uint32_t m_block_padding_size = 0;
        uint32_t m_block_member_count = 0;
        // 结构体属性
        // [Struct Name] [Member Name] [size,offset]
        std::pair<std::string,
        std::unordered_map<std::string, std::pair<size_t,size_t>>> m_struct_property;
        // 记录顺序
        std::pair<std::string,
            std::unordered_map<int, std::string>> m_struct_property_order;
        // 结构体成员类型
        // [MemberName][TypeName]
        std::unordered_map<std::string, std::string> m_struct_type_name;

        // Struct数据
        std::vector<void*> p_datas;
        bool m_is_copy = false;
        bool m_need_data = false;        // 当前插槽数据是否为空，主要用于Texture的插入
        bool m_is_cube = false;
        std::set<uint32_t> m_texture_change_index;   // 纹理改变的索引，如果为空，则没有纹理被修改
        uint32_t m_member_index = 0;    // 当前调用的位置索引
    private:
        void setup();
        // 创建UBO TEX...
        void setupData();
        // 获取SpvOpType的实际映射类型，主要获取基本类型
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
        // 返回当前纹理索引（数量）
        inline const uint32_t getTextureIndex() { return p_texture_ext.second.size(); }
        // 返回当前纹理数量
        inline const uint32_t getTextureCount() { return p_texture_ext.second.size(); }
        inline const bool isArray() { return m_is_array; }
        inline VkDescriptorBindingFlagsEXT& getVkDescriptorBindingFlagsEXT() { return m_binding_flags_ext; }
    };

}

