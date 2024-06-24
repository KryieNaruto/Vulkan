#pragma once
#include "base_editor.h"
#include <Resource/resource_global.h>

namespace Editor {
    //===========================
    // ��ʾ����
    //===========================
    class ENGINE_API_EDITOR SceneEditor :public Editor::ZEditor
    {
    public:
        SceneEditor();
        virtual ~SceneEditor();
        virtual void onInitialize() override;
        virtual void onStart() override;
        virtual void onRender() override;
        virtual void onResize() override;

        // ����
        Core::Resource::Scene* p_scene = nullptr;
    protected:
        ThirdParty::Sampler* m_render_sampler = nullptr;
		std::vector<VkDescriptorSet> m_render_descriptor_sets;		
    private:
        void checkOnResize();
        // ����uv, canvas:������С�� tex_size: ͼ���С, Ĭ�ϻ���<ͼ��
        ImVec2 calcuUV0(ImVec2 _canvas, const VkExtent3D& _tex_size);
        ImVec2 calcuUV1(ImVec2 _canvas, const VkExtent3D& _tex_size);


        // �����С
        ImVec2 m_render_size_current = { 0,0 };
        ImVec2 m_render_pos_current = { 0,0 };
        ImVec2 m_render_size = { 0,0 };
        ImVec2 m_render_pos  = { 0,0 };
        // �����С vk
        VkExtent3D m_render_extent = { 0,0,1 };
        
        bool m_resizing = false;
        // ��һ����Ⱦ����ȡIMGUI�Ĵ��ڴ�С����Ԥ���趨ֵ
        bool m_render_first = true;
        // m_render_texture �Ƿ��������
        bool m_render_initialized = false;
        // ���ڴ�С�Ƿ�ı�
        bool m_render_resize = false;
        
    };

}

