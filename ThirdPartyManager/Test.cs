using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace ThirdPartyManager
{
    public static class Test
    {
        public static void SayHello()
        {
            var a = VulkanManager.Instance;
            Console.WriteLine("Hello World!");
        }
    }
}
