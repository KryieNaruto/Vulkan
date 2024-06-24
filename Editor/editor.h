#pragma once
#include "include/base_editor.h"

namespace Editor {
    //==========================
    // 最终主窗体
    //==========================
    class ENGINE_API_EDITOR Editor :
        public ZEditor
    {
    public:
        Editor();
        virtual ~Editor();
		virtual void onStart() override;
		virtual void onRender() override;

        virtual void save();

        /** 窗口事件处理 */
        virtual bool onEventProcess(bool& _renderReady);
        /** 渲染前准备 */
        virtual void onRenderStart();
        /** IMGUI绘制 */
        virtual void onImGuiRender();
        /** 提交IMGUI绘制指令并呈现 */
        virtual void onFrameRenderAndPresent();
	private:
		ThirdParty::CommandBuffer* p_main_command_buffer = nullptr;
		ThirdParty::Queue* p_graphics_queue = nullptr;
		ThirdParty::Pipeline* p_graphics_pipeline = nullptr;
    protected:
    };
}

