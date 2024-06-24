#pragma once
#include "Camera.h"

using TextureEXT = ThirdParty::TextureEXT;
using FrameBuffer = ThirdParty::FrameBuffer;
using CommandBuffer = ThirdParty::CommandBuffer;

namespace Core::Resource {

    // 渲染到指定Attachment
    // 否定RCamera的RRenderPass/ Framebuffer / Color Attachment / Depth Attachment, 转而接受任意TextureEXT 为attachment容器
    // RCamera 添加了一个m_is_sub_camera标志位来跳过原本的RenderPass...等对象的创建

    class SubCamera :
        public Camera
    {
    public:
        SubCamera(const std::string& _name = "SubCamera");
        virtual ~SubCamera();
        //--------------------------------- Target
        // target必须是继承了SceneObject
        SubCamera* setPosAndTarget(SceneObject* _target);
        SubCamera* setPosAndTarget(ObjectProperty* _target_property);
        SubCamera* setPosAndTarget(const glm::vec3& _pos, const glm::vec3& _target);
        SubCamera* setTempViewMatrix(const glm::mat4& _view);
        SubCamera* setTempProjectionMatrix(const glm::mat4& _proj);
        SceneObject* getSceneObjectTarget();
        //--------------------------------- RenderPass
        SubCamera* setRRenderPass(RRenderPass* _renderpass);
        //--------------------------------- FrameBuffer
        SubCamera* setCanvasSize(const VkExtent2D& _extent);
        SubCamera* setCanvasSize(const VkExtent3D& _extent);
        SubCamera* addAttachment(TEXTURE_TYPE _type, TextureEXT* _texture_ext);
        SubCamera* createFramebuffer(uint32_t _frame_count = 0);
        SubCamera* createCubeFramebuffer(uint32_t _frame_count = 0,uint32_t _mip_map = 1);
        //--------------------------------- CommandBuffer
        SubCamera* createCommandBuffer(uint32_t _frame_count = 0);
        //--------------------------------- Render

        void onUpdate();
        SubCamera* endInitialize();         // 结束初始化
        inline ThirdParty::FrameBuffer* getFrameBuffer(uint32_t index) { return m_framebuffer_output[index]; }
    protected:
        SceneObject* p_target = nullptr;
        ObjectProperty* p_target_property = nullptr;
    };

}

