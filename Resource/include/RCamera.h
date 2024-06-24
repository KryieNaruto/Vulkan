#pragma once
#include "ZResource.h"
#include "resource_inner_global.h"
#include "resource_preload.h"
#include <ThirdPartyManager/third_party_manager_global.h>
namespace Core::Resource {
    
    /************************************************************************/
    /* Camera����                                                           */
    /* ����ʹ����������ϵ                                                   */
    /* ����������X ������������Y����Ļ���Z                           */
    /* VK��NDC����ϵ������������ϵ�Ļ����ϣ�Y�ᷭת                         */
    /************************************************************************/

    class ENGINE_API_RESOURCE RCamera :
        public ZResource
    {
    public:
        inline static std::set<RCamera*> g_all_cameras;
        // Sub Camera�������ջ�����С�����ڼ����С
        RCamera(const std::string& _name);
        RCamera(const VkExtent3D& _target /* Ŀ�껭����С */, const std::string& _name = "MainCamera");
        virtual ~RCamera();
		virtual Json::Value serializeToJSON() override;
		virtual void deserializeToObj(Json::Value& _root) override;
    
        inline void focus() { m_focus = true; }
        inline void unfocus() { m_focus = false; }
        // ��������
        virtual void onUpdate();
        // ����
        virtual void onRender();
        // ��ʼ����
        virtual void onRenderStart();
        // ��������
        virtual void onRenderEnd();
        // ��������
        virtual VkCommandBuffer onRenderStartImmediately();
        // ��������,���ύ
        virtual void onRenderEndImmediately(VkCommandBuffer _cmd);
        // ������С�ı�
        virtual void onResize(const VkExtent3D& _canvas);
        // ����ǰ֡����������������ʹ������Դ����ִ��
        void skipOnceCommandBuffer();
        // �ػ浱ǰ֡��������vk��Դ�滻����������Imgui�����ڼ���á�
        void repaint();
        
        RCamera* setClearValues(const std::vector<VkClearValue>& _value);
        // ���ض���
        inline VkImageView& getVkImageView(const uint32_t _current_image) { return m_image_view_output[_current_image]->getVkImageView(); }
        inline ThirdParty::Texture* getTexture(const uint32_t _current_image) { return m_texture_output[_current_image]; }
        // ����cmd[i]
        inline VkCommandBuffer getVkCommandBuffer(int i) { 
            return m_command_buffer[i]->getVkCommandBuffer();
        }
        // ����cmd[current_image]
        inline VkCommandBuffer getVkCommandBuffer() { 
            return m_command_buffer[m_vkInfo.m_current_image]->getVkCommandBuffer();
        }
        inline VkFence& getVkFence() { return m_fences[m_vkInfo.m_current_frame]->getVkFence(); }
        inline VkSemaphore& getWaitSemaphore() { return m_wait_semaphores[m_vkInfo.m_current_frame]->getVkSemaphore(); }
        inline VkSemaphore& getSignalSemaphore() { return m_signal_semaphores[m_vkInfo.m_current_frame]->getVkSemaphore(); }
        // ������ʱframebuffer
        inline RCamera* setTempFrameBuffer(ThirdParty::FrameBuffer* _framebuffer) { p_temp_framebuffer_output = _framebuffer; return this; }
        // ������ʱrender area
        inline RCamera* setTempRenderArea(const VkRect2D& _rect) { m_temp_render_area = _rect; m_temp_render_area_bool = true; return this; }
        // ������С�Ƿ�ı�
        inline bool& isResize() { return m_canvas_is_resize; }
        // ������һ��
        bool isFirst();
        // �Ƿ���Ի���
        inline void isReady(uint32_t _index,bool _b = true) { m_canvas_render_finish[_index] = _b; }
        inline bool isReady() { return !m_canvas_is_resize && m_render_initialized; }
        // ��������
        inline CameraProperty* getProperty() { return (CameraProperty*)p_property; }
        // ��ȡ���ͼ
        ThirdParty::TextureEXT* getDepthTexture() { return p_depth_texture; }
    protected:
        CameraProperty* p_camera_property = nullptr;
        std::vector<ThirdParty::CommandBuffer*> m_command_buffer;
        std::vector<ThirdParty::FrameBuffer*>   m_framebuffer_output;
        ThirdParty::FrameBuffer* p_temp_framebuffer_output = nullptr;
        bool m_temp_render_area_bool = false;
        VkRect2D m_temp_render_area = {};
        std::vector<ThirdParty::Image*>     m_image_output;         // image �������ڴ�
        std::vector<ThirdParty::ImageView*> m_image_view_output;    // imageView ��ͼ
        std::vector<ThirdParty::Texture*>   m_texture_output;       // image / imageView ���
        // ���ͼ
        ThirdParty::TextureEXT* p_depth_texture = nullptr;          // depth attachment
        // ��ɫ���������ڽ���shader��������ز���
        ThirdParty::TextureEXT* p_color_texture = nullptr;
        ThirdParty::RenderPass* p_render_pass = nullptr;
        // ͬ������
        std::vector<ThirdParty::Fence*> m_fences;
        std::vector<ThirdParty::Semaphore*> m_wait_semaphores;
        std::vector<ThirdParty::Semaphore*> m_signal_semaphores;
        std::vector<VkClearValue> t_clear_values;

    protected:
        // �ڲ���ʼ��
        virtual void setup();
        // �������
        virtual void onInput();

    protected:
        // �Ƿ���SubCamera����
        bool m_is_sub_camera = false;
        bool m_render_initialized = false;
        // �����Ƿ����
        bool m_focus = false;
        // ������С�Ƿ�ı�
        bool m_canvas_is_resize = false;
        // �����Ƿ���Ⱦ���
        bool m_canvas_render_finish[4] = { false };
        // �Ƿ�������һ��
        bool m_skip_first[4] = { true };

        // ��������
        bool m_key_press[256] = { false };
        // �������²�����
        bool m_key_click[256] = { false };
    };

}

