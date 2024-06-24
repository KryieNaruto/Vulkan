
using Core;
using Vortice.Vulkan;
using static Vortice.Vulkan.Vulkan;
using static SDL2.SDL;

namespace ThirdPartyManager
{
    public unsafe sealed class VulkanManager : Manager
    {
        new public static VulkanManager Instance { get; private set; } = new VulkanManager();

        private readonly VkInstance _Instance;
        private readonly VkSurfaceKHR _SurfaceKHR;
        private readonly VkSwapchainKHR _SwapchainKHR;
        private readonly List<VkPhysicalDevice> _PhysicalDevices;
        private readonly int _PhysicalDeviceIndex;

        public VulkanManager()
        {
            #region 获取VkInstance
            HashSet<string> availableLayers = new HashSet<string>(EnumerateInstanceLayers());
            HashSet<string> availableExtensions = new HashSet<string>(GetInstanceExtensions());

            List<string> instanceExtensions = [.. SDLManager.Instance.Vulkan_GetInstanceExtensions()];
            List<string> instanceLayers = [];

            VkApplicationInfo _applicationInfo = new()
            {
                pApplicationName = new VkString(Configuration.ApplicationName),
                applicationVersion = Configuration.ApplicationVersion,
                pEngineName = new VkString(Configuration.EngineName),
                engineVersion = Configuration.EngineVersion,
                apiVersion = VkVersion.Version_1_3,
                pNext = null,
            };
            VkInstanceCreateInfo _createInfo = new()
            {
                pApplicationInfo = &_applicationInfo,
                enabledExtensionCount = (uint)instanceExtensions.Count,
                ppEnabledExtensionNames = new VkStringArray(instanceExtensions),
                enabledLayerCount = (uint)instanceLayers.Count,
                ppEnabledLayerNames = new VkStringArray(instanceLayers),
                flags = 0,
                pNext = null,
            };
            // 创建vkinstance

            if (!CheckVkResult(vkCreateInstance(&_createInfo, null, out _Instance)))
                throw new VulkanException(ZsUtils.GetMethodFullName(), "Failed to create vkInstance!", ZsEnum.MESSAGE_TYPE.ERROR);
            #endregion
        }
        public bool CheckVkResult(VkResult _result)
        {
            if (_result != VkResult.Success) return false;
            return true;
        }

        // 检查Vk版本兼容性
        private readonly Lazy<bool> s_isSupported = new(CheckIsSupported);
        public bool IsSupported() => s_isSupported.Value;
        private static bool CheckIsSupported()
        {
            try
            {
                VkResult result = vkInitialize();
                if (result != VkResult.Success)
                    return false;

                uint propCount;
                result = vkEnumerateInstanceExtensionProperties(&propCount, null);
                if (result != VkResult.Success)
                {
                    return false;
                }

                // We require Vulkan 1.1 or higher
                VkVersion version = vkEnumerateInstanceVersion();
                if (version < VkVersion.Version_1_1)
                    return false;

                // TODO: Enumerate physical devices and try to create instance.

                return true;
            }
            catch
            {
                return false;
            }
        }
        // 获取所有Instance Layers
        private string[] EnumerateInstanceLayers()
        {
            if (!IsSupported())
            {
                return [];
            }

            uint count = 0;
            VkResult result = vkEnumerateInstanceLayerProperties(&count, null);
            if (result != VkResult.Success || count == 0)
            {
                return [];
            }

            VkLayerProperties* properties = stackalloc VkLayerProperties[(int)count];
            vkEnumerateInstanceLayerProperties(&count, properties).CheckResult();

            string[] resultExt = new string[count];
            for (int i = 0; i < count; i++)
            {
                resultExt[i] = properties[i].GetLayerName();
            }

            return resultExt;
        }
        // 获取所有Instance Extensions
        private static string[] GetInstanceExtensions()
        {
            uint count = 0;
            VkResult result = vkEnumerateInstanceExtensionProperties(&count, null);
            if (result != VkResult.Success)
            {
                return Array.Empty<string>();
            }

            if (count == 0)
            {
                return Array.Empty<string>();
            }

            VkExtensionProperties* props = stackalloc VkExtensionProperties[(int)count];
            vkEnumerateInstanceExtensionProperties(&count, props);

            string[] extensions = new string[count];
            for (int i = 0; i < count; i++)
            {
                extensions[i] = props[i].GetExtensionName();
            }

            return extensions;
        }
    }

}
