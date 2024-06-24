#pragma once
#include "common/include/third_party_manager.h"
#include <ThirdParty/glm/glm.hpp>
#include <ThirdParty/SDL2/SDL.h>
#include <ThirdParty/SDL2/SDL_vulkan.h>
#include <chrono>
namespace ThirdParty {

	//=====================================
	// SDL ����
	//=====================================
	class ENGINE_API_THIRDPARTY SDLManager :
		public Manager<SDLManager>
	{
	public:
		SDLManager();
		~SDLManager() override;

		// ͨ�� Manager �̳�
		//************************************
		// Method:    init
		// FullName:  ThirdParty::SDLManager::init
		// Access:    public 
		// Returns:   ThirdParty::SDLManager*
		// Qualifier: ��ʼ��SDL,����SDL����,��ȡvulkan instance extension,��ȡvulkan swapchain
		//************************************
		SDLManager* init() override;

		//************************************
		// Method:    getManager
		// FullName:  ThirdParty::SDLManager::getManager
		// Access:    public 
		// Returns:   ThirdParty::SDLManager*
		// Qualifier: ����SDL�������
		//************************************
		SDLManager* getManager() override;

		//************************************
		// Method:    destroy
		// FullName:  ThirdParty::SDLManager::destroy
		// Access:    public 
		// Returns:   void
		// Qualifier: SDL ��Դ����,SDL_Quit,SDL_destroyWindow
		//************************************
		void destroy() override;

		void windowResize() override;

		// ��������
		void onStartListener();
		// ÿ֡�����ڲ�����
		void onUpdate();
		// ����
		// ��������
		bool keyDown(SDL_Scancode _code);
		// ��������
		bool keyUp(SDL_Scancode _code);
		// ����һ�ΰ���
		bool keyClick(SDL_Scancode _code);
		// ˫��
		bool keyDoubleClick(SDL_Scancode _code);
		// ���
		bool mouseDown(Uint8 _code);
		bool mouseUp(Uint8 _code);
		bool mouseClick(Uint8 _code);
		bool mouseDoubleClick(Uint8 _code);
		bool mouseWheel();
		const std::vector<int> getMousePos();
		const std::vector<int> getMouseMove();
		const glm::vec2 getMousePos_vec2();
		const glm::vec2 getLastMousePos_vec2();
		int getMouseWheelMove();
	private:
		SDL_Window* p_sdlwindow = nullptr;
		SDL_Event* p_sdlevent = nullptr;

		bool m_key_down[256] = { false };
		bool m_key_up[256] = { false };
		bool m_key_click[256] = { false };
		bool m_key_double_click[256] = { false };

		std::chrono::steady_clock::time_point m_time_current;
		const size_t m_time_offset = 200;	// 0.2 s

		// ����¼�
		bool m_mouse_down[8] = { false };
		bool m_mouse_up[8] = { false };
		bool m_mouse_click[8] = { false };
		bool m_mouse_double_click[8] = { false };
		const size_t m_time_offset_mouse = 100;	// 0.1 s
		// ���λ��
		int m_x_pos = 0;
		int m_x_pos_last = 0;
		int m_y_pos = 0;
		int m_y_pos_last = 0;
		// ������
		int m_wheel_pos = 0;
		int m_wheel_pos_last = 0;
	};

	extern ENGINE_API_THIRDPARTY SDLManager sdlManager;
}

