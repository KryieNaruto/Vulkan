#pragma once
#include "base_editor.h"
#include <Resource/resource_global.h>

namespace Editor {
    //===========================
    // 显示场景
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

        // 场景
        Core::Resource::Scene* p_scene = nullptr;
    protected:
        ThirdParty::Sampler* m_render_sampler = nullptr;
		std::vector<VkDescriptorSet> m_render_descriptor_sets;		
    private:
        void checkOnResize();
        // 计算uv, canvas:画布大小， tex_size: 图像大小, 默认画布<图像
        ImVec2 calcuUV0(ImVec2 _canvas, const VkExtent3D& _tex_size);
        ImVec2 calcuUV1(ImVec2 _canvas, const VkExtent3D& _tex_size);


        // 画面大小
        ImVec2 m_render_size_current = { 0,0 };
        ImVec2 m_render_pos_current = { 0,0 };
        ImVec2 m_render_size = { 0,0 };
        ImVec2 m_render_pos  = { 0,0 };
        // 画面大小 vk
        VkExtent3D m_render_extent = { 0,0,1 };
        
        bool m_resizing = false;
        // 第一次渲染，获取IMGUI的窗口大小而非预先设定值
        bool m_render_first = true;
        // m_render_texture 是否设置完毕
        bool m_render_initialized = false;
        // 窗口大小是否改变
        bool m_render_resize = false;
        
    };

}

