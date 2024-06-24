#pragma once
#include "ZResource.h"
#include "resource_inner_global.h"
#include "resource_preload.h"
#include <ThirdPartyManager/third_party_manager_global.h>
namespace Core::Resource {
    
    /************************************************************************/
    /* Camera基类                                                           */
    /* 引擎使用左手坐标系                                                   */
    /* 向右正方向：X ，向上正方向：Y，屏幕向里：Z                           */
    /* VK的NDC坐标系是在左手坐标系的基础上，Y轴翻转                         */
    /************************************************************************/

    class ENGINE_API_RESOURCE RCamera :
        public ZResource
    {
    public:
        inline static std::set<RCamera*> g_all_cameras;
        // Sub Camera，不接收画布大小，后期计算大小
        RCamera(const std::string& _name);
        RCamera(const VkExtent3D& _target /* 目标画布大小 */, const std::string& _name = "MainCamera");
        virtual ~RCamera();
		virtual Json::Value serializeToJSON() override;
		virtual void deserializeToObj(Json::Value& _root) override;
    
        inline void focus() { m_focus = true; }
        inline void unfocus() { m_focus = false; }
        // 更新数据
        virtual void onUpdate();
        // 绘制
        virtual void onRender();
        // 开始绘制
        virtual void onRenderStart();
        // 结束绘制
        virtual void onRenderEnd();
        // 立即绘制
        virtual VkCommandBuffer onRenderStartImmediately();
        // 结束绘制,并提交
        virtual void onRenderEndImmediately(VkCommandBuffer _cmd);
        // 画布大小改变
        virtual void onResize(const VkExtent3D& _canvas);
        // 将当前帧命令销毁跳过，以使过期资源不被执行
        void skipOnceCommandBuffer();
        // 重绘当前帧，将过期vk资源替换，仅允许在Imgui绘制期间调用。
        void repaint();
        
        RCamera* setClearValues(const std::vector<VkClearValue>& _value);
        // 返回对象
        inline VkImageView& getVkImageView(const uint32_t _current_image) { return m_image_view_output[_current_image]->getVkImageView(); }
        inline ThirdParty::Texture* getTexture(const uint32_t _current_image) { return m_texture_output[_current_image]; }
        // 返回cmd[i]
        inline VkCommandBuffer getVkCommandBuffer(int i) { 
            return m_command_buffer[i]->getVkCommandBuffer();
        }
        // 返回cmd[current_image]
        inline VkCommandBuffer getVkCommandBuffer() { 
            return m_command_buffer[m_vkInfo.m_current_image]->getVkCommandBuffer();
        }
        inline VkFence& getVkFence() { return m_fences[m_vkInfo.m_current_frame]->getVkFence(); }
        inline VkSemaphore& getWaitSemaphore() { return m_wait_semaphores[m_vkInfo.m_current_frame]->getVkSemaphore(); }
        inline VkSemaphore& getSignalSemaphore() { return m_signal_semaphores[m_vkInfo.m_current_frame]->getVkSemaphore(); }
        // 设置临时framebuffer
        inline RCamera* setTempFrameBuffer(ThirdParty::FrameBuffer* _framebuffer) { p_temp_framebuffer_output = _framebuffer; return this; }
        // 设置临时render area
        inline RCamera* setTempRenderArea(const VkRect2D& _rect) { m_temp_render_area = _rect; m_temp_render_area_bool = true; return this; }
        // 画布大小是否改变
        inline bool& isResize() { return m_canvas_is_resize; }
        // 跳过第一次
        bool isFirst();
        // 是否可以绘制
        inline void isReady(uint32_t _index,bool _b = true) { m_canvas_render_finish[_index] = _b; }
        inline bool isReady() { return !m_canvas_is_resize && m_render_initialized; }
        // 返回属性
        inline CameraProperty* getProperty() { return (CameraProperty*)p_property; }
        // 获取深度图
        ThirdParty::TextureEXT* getDepthTexture() { return p_depth_texture; }
    protected:
        CameraProperty* p_camera_property = nullptr;
        std::vector<ThirdParty::CommandBuffer*> m_command_buffer;
        std::vector<ThirdParty::FrameBuffer*>   m_framebuffer_output;
        ThirdParty::FrameBuffer* p_temp_framebuffer_output = nullptr;
        bool m_temp_render_area_bool = false;
        VkRect2D m_temp_render_area = {};
        std::vector<ThirdParty::Image*>     m_image_output;         // image ，包含内存
        std::vector<ThirdParty::ImageView*> m_image_view_output;    // imageView 视图
        std::vector<ThirdParty::Texture*>   m_texture_output;       // image / imageView 结合
        // 深度图
        ThirdParty::TextureEXT* p_depth_texture = nullptr;          // depth attachment
        // 颜色附件，用于接受shader输出，多重采样
        ThirdParty::TextureEXT* p_color_texture = nullptr;
        ThirdParty::RenderPass* p_render_pass = nullptr;
        // 同步对象
        std::vector<ThirdParty::Fence*> m_fences;
        std::vector<ThirdParty::Semaphore*> m_wait_semaphores;
        std::vector<ThirdParty::Semaphore*> m_signal_semaphores;
        std::vector<VkClearValue> t_clear_values;

    protected:
        // 内部初始化
        virtual void setup();
        // 摄像操作
        virtual void onInput();

    protected:
        // 是否是SubCamera类型
        bool m_is_sub_camera = false;
        bool m_render_initialized = false;
        // 摄像是否可用
        bool m_focus = false;
        // 画布大小是否改变
        bool m_canvas_is_resize = false;
        // 画面是否渲染完成
        bool m_canvas_render_finish[4] = { false };
        // 是否跳过第一次
        bool m_skip_first[4] = { true };

        // 按键按下
        bool m_key_press[256] = { false };
        // 按键按下并弹起
        bool m_key_click[256] = { false };
    };

}

