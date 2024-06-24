using Core;
using System.Numerics;
using System.Runtime.InteropServices;
using Vortice.Vulkan;
using static SDL2.SDL;
using static SDL2.SDL.SDL_WindowFlags;

namespace ThirdPartyManager
{
    public sealed class SDLManager : Manager
    {
        new public static SDLManager Instance { set; get; } = new SDLManager();

        public nint WindowID { private set; get; } = -1;
        SDLManager()
        {
            if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_GAMECONTROLLER | SDL_INIT_EVENTS) != 0)
            {   // 初始化失败
                var msg = SDL_GetError();
                throw new SDLException(ZsUtils.GetMethodFullName(), "Failed to init SDL:{ msg }", ZsEnum.MESSAGE_TYPE.ERROR);
            }
            // 初始化窗口
            WindowID = SDL_CreateWindow(
                Configuration.ApplicationName,
                SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 0, 0,
                SDL_WINDOW_VULKAN | SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI | SDL_WINDOW_MAXIMIZED);
            // 更新窗口大小
            UpdateWindowSize();

        }

        ~SDLManager()
        {
            if (WindowID != -1)
                SDL_Quit();
        }

        public string[] Vulkan_GetInstanceExtensions()
        {
            uint instanceCount = 0;
            var result = SDL_Vulkan_GetInstanceExtensions(Instance.WindowID, out instanceCount, null);
            if (result != SDL_bool.SDL_TRUE || instanceCount == 0)
            {
                var msg = SDL_GetError();
                throw new SDLException(ZsUtils.GetMethodFullName(), "Failed to init SDL:{ msg }", ZsEnum.MESSAGE_TYPE.ERROR);
            }
            var instances = new nint[instanceCount];
            result = SDL_Vulkan_GetInstanceExtensions(Instance.WindowID, out instanceCount, instances);

            if (result == SDL_bool.SDL_TRUE)
            {
                HashSet<string> instances_str = new HashSet<string>();
                foreach (var instance in instances)
                {
                    string? str = Marshal.PtrToStringAnsi(instance);
                    if (str != null)
                        instances_str.Add(str);
                }
                return instances_str.ToArray();
            }
            else
            {
                var msg = SDL_GetError();
                throw new SDLException(ZsUtils.GetMethodFullName(), "Failed to init SDL:{ msg }", ZsEnum.MESSAGE_TYPE.ERROR);
            }
        }

        public void UpdateWindowSize()
        {
            int w, h;
            SDL_GetWindowSize(WindowID, out w, out h);
            Configuration.WindowSize.X = w;
            Configuration.WindowSize.Y = h;
        }
    }
}
