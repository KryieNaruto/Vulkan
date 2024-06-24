#pragma once
#include "include/base_editor.h"

namespace Editor {
    //==========================
    // ����������
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

        /** �����¼����� */
        virtual bool onEventProcess(bool& _renderReady);
        /** ��Ⱦǰ׼�� */
        virtual void onRenderStart();
        /** IMGUI���� */
        virtual void onImGuiRender();
        /** �ύIMGUI����ָ����� */
        virtual void onFrameRenderAndPresent();
	private:
		ThirdParty::CommandBuffer* p_main_command_buffer = nullptr;
		ThirdParty::Queue* p_graphics_queue = nullptr;
		ThirdParty::Pipeline* p_graphics_pipeline = nullptr;
    protected:
    };
}

