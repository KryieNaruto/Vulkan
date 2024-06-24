using System.Numerics;
using Vortice.Vulkan;

namespace ThirdPartyManager
{
    public static class Configuration
    {
        internal static uint MakeVersion(int major,int minor,int patch)
        {
            return (uint)((major << 22) | (minor << 12) | patch);
        }

        public static string ApplicationName = "Application";
        public static VkVersion ApplicationVersion = new VkVersion(1, 0, 0);
        public static string EngineName = "ZsEngine";
        public static VkVersion EngineVersion = new VkVersion(1, 0, 0);
        public static VkVersion ApiVersion = VkVersion.Version_1_3;
        public static Vector2 WindowSize = new Vector2(0, 0);
    }
}
