#include "sdl_party_manager.h"
#include "core/info/include/global_info.h"
#include "core/common/include/debug_to_console.h"
#include <stdexcept>
#include <cstring>

using namespace ThirdParty;
using namespace Core;

ENGINE_API_THIRDPARTY ThirdParty::SDLManager ThirdParty::sdlManager;
static bool has_event = false;
SDLManager::~SDLManager()
{
    destroy();
}

SDLManager::SDLManager()
{

}

SDLManager* ThirdParty::SDLManager::init()
{
    // ��ʼ��SDL:��Ƶ��ϵͳ|�¼���ϵͳ
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        Core::error("Failed to init SDL");
        char buffer[256];
        SDL_GetErrorMsg(buffer, 256);
        throw std::runtime_error(buffer);
    }
    else {
        // ��������
        g_ApplicationWindowInfo.updateWindowInfo();     // ��ʼ�������С
        this->p_sdlwindow = SDL_CreateWindow(
            g_ApplicationWindowInfo.m_title.c_str(),
            SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
            g_ApplicationWindowInfo.m_window_width,
            g_ApplicationWindowInfo.m_window_height,
            g_ApplicationWindowInfo.m_window_sdl_flags);
		if (!this->p_sdlwindow) {
			Core::error("Failed to create SDL window");
		}
        // ���´����С��Ϣ
        g_ApplicationWindowInfo.updateWindowInfo(this->p_sdlwindow);
        this->p_sdlevent = new SDL_Event();
		this->m_isInitialized = true;
        // ����SDL��Ϣ��ȫ��
        Core::g_ApplicationSDLInfo.p_sdlwindow = this->p_sdlwindow;
        Core::g_ApplicationSDLInfo.p_sdlevent = this->p_sdlevent;
        // ����vulkan��Ϣ��ȫ��
        {
            // instance ��չ
            uint32_t extension_count = 0;
            SDL_Vulkan_GetInstanceExtensions(this->p_sdlwindow, &extension_count, 0);
            Core::g_ApplicationVulkanInfo.m_instance_extensions.resize(extension_count);
            SDL_Vulkan_GetInstanceExtensions(this->p_sdlwindow, &extension_count, Core::g_ApplicationVulkanInfo.m_instance_extensions.data());
            // У���
			if (Core::g_ApplicationInfo.m_debug) {
				Core::g_ApplicationVulkanInfo.m_validation_layers.push_back("VK_LAYER_KHRONOS_validation");
                // ���У����Ƿ����
				uint32_t layerCount = 0;
                std::vector<VkLayerProperties>layerProperties;
				vkEnumerateInstanceLayerProperties(&layerCount, nullptr);
                layerProperties.resize(layerCount);
				vkEnumerateInstanceLayerProperties(&layerCount, layerProperties.data());
                bool layerSupported = false;
                for (const char* layerName : Core::g_ApplicationVulkanInfo.m_validation_layers) {
                    for (const auto& layerProperty : layerProperties) {
                        if (strcmp(layerName, layerProperty.layerName) == 0) {
                            layerSupported = true;
                            break;
                        }
                    }
                    if (layerSupported) break;
                }
                // ��֧��У���
                if (!layerSupported) {
                    Core::warn("VK_LAYER_KHRONOS_validation was not supported!");
                    Core::g_ApplicationVulkanInfo.m_validation_layers.clear();
                }
                else {
                    Core::g_ApplicationVulkanInfo.m_instance_extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
                }

			}
        }

    }
    this->m_isInitialized = true;
    Core::success("SDL_Vulkan has been initialized");
    return this;
}

SDLManager* ThirdParty::SDLManager::getManager()
{
    return this;
}

const glm::vec2 SDLManager::getLastMousePos_vec2()
{
    return { m_x_pos,m_y_pos };
}

const glm::vec2 SDLManager::getMousePos_vec2()
{
    return { m_x_pos_last,m_y_pos_last };
}

void SDLManager::onUpdate()
{
	// һ��ʱ������up click double_click�ı�־λ
	auto _now = std::chrono::steady_clock::now();
	std::chrono::duration<double, std::milli> tm = _now - m_time_current;
	if (tm.count() > m_time_offset) {
		memset(m_key_up, false, sizeof(m_key_up));
		memset(m_key_click, false, sizeof(m_key_click));
		memset(m_key_double_click, false, sizeof(m_key_double_click));
	}
	if (tm.count() > m_time_offset_mouse) {
		memset(m_mouse_up, false, sizeof(m_mouse_up));
		memset(m_mouse_click, false, sizeof(m_mouse_click));
		memset(m_mouse_double_click, false, sizeof(m_mouse_double_click));

        // ��ջ���״̬
		m_wheel_pos_last = 0;
		m_wheel_pos = 0;
	}
    // ������ͣ��
    if (!has_event) {
        // �������λ��
        m_x_pos_last = m_x_pos;
        m_y_pos_last = m_y_pos;
    }
    else {
        has_event = false;
    }
}

bool SDLManager::keyDoubleClick(SDL_Scancode _code)
{
    return m_key_double_click[_code];
}

bool SDLManager::keyClick(SDL_Scancode _code)
{
    return m_key_click[_code];
}

bool SDLManager::keyUp(SDL_Scancode _code)
{
    return m_key_up[_code];
}

void SDLManager::onStartListener()
{
    if (!p_sdlevent) return;
    SDL_Event& e = *p_sdlevent;
    bool _event = false;
    // ���================================================
    // ���� 
    if (e.type == SDL_MOUSEBUTTONUP) {
        if ((m_mouse_click[e.button.button] = true)) {
            m_mouse_double_click[e.button.button] = true;
        }
        else if ((m_mouse_down[e.button.button] = true)) {
            m_mouse_click[e.button.button] = true;
        }
        m_mouse_up[e.button.button] = true;
        m_mouse_down[e.button.button] = false;
        _event = true;
    }
    // ��갴��
    if (e.type == SDL_MOUSEBUTTONDOWN) {
        m_mouse_up[e.button.button] = false;
        m_mouse_down[e.button.button] = true;
        _event = true;
    }
    // ����ƶ�
    if (e.type == SDL_MOUSEMOTION) {
        m_x_pos_last = m_x_pos;
        m_y_pos_last = m_y_pos;
        m_x_pos = e.button.x;
        m_y_pos = e.button.y;
        if (m_mouse_down[e.button.button])
            _event = true;
    }
    // ������
    if (e.type == SDL_MOUSEWHEEL) {
        m_wheel_pos_last = m_wheel_pos;
        m_wheel_pos = e.wheel.y;
        _event = true;
    }
    //===================================================================
    // ����
    if (e.type == SDL_KEYUP) {
        // ����Ѿ�click,���¼һ��double click
        if ((m_key_click[e.key.keysym.scancode] = true)) {
            m_key_double_click[e.key.keysym.scancode] = true;
        }
        // ����Ѿ����£����¼һ��click
        else if ((m_key_down[e.key.keysym.scancode] = true)) {
            m_key_click[e.key.keysym.scancode] = true;
        }
        // ���°�������
        m_key_up[e.key.keysym.scancode] = true;
        m_key_down[e.key.keysym.scancode] = false;
        _event = true;
    }
    if (e.type == SDL_KEYDOWN) {
        m_key_up[e.key.keysym.scancode] = false;
        m_key_down[e.key.keysym.scancode] = true;
        _event = true;
    }
    if (_event) {
		m_time_current = std::chrono::steady_clock::now();
        has_event = true;
    }
}


bool SDLManager::keyDown(SDL_Scancode _code)
{
    return m_key_down[_code];
}

void SDLManager::windowResize()
{
    int w, h;
    SDL_GetWindowSize(this->p_sdlwindow, &w, &h);
    Core::windowInfo.m_window_width = w;
    Core::windowInfo.m_window_height = h;
    Core::windowInfo.m_window_aspect = (float)w / (float)h;
    this->m_isInitialized = true;
}

void SDLManager::destroy()
{
	if (this->m_isInitialized) {
        SDL_DestroyWindow(this->p_sdlwindow);
        SDL_Quit();
    }
}

bool SDLManager::mouseDown(Uint8 _code) { 
    return m_mouse_down[_code]; 
}
bool SDLManager::mouseUp(Uint8 _code) { 
    return m_mouse_up[_code]; 
}
bool SDLManager::mouseClick(Uint8 _code) { 
    return m_mouse_click[_code]; 
}
bool SDLManager::mouseDoubleClick(Uint8 _code) { 
    return m_mouse_double_click[_code];
}
bool SDLManager::mouseWheel() { 
    return m_wheel_pos != 0; 
}
const std::vector<int> SDLManager::getMousePos() { 
    return { m_x_pos,m_y_pos }; 
}
const std::vector<int> SDLManager::getMouseMove() { 
    return { m_x_pos - m_x_pos_last,m_y_pos - m_y_pos_last };
}
int SDLManager::getMouseWheelMove() { 
    return m_wheel_pos; 
}
