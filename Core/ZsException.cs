using static Core.ZsEnum;

namespace Core
{
    public class ZsException : Exception
    {
        public ZsException(string _info) :base(_info) {}

        public ZsException(string _module, string _info, MESSAGE_TYPE _type) :base(_info)
        {
            // 获取时间
            var time = DateTime.Now.ToString("HH:mm:ss:ff");
            EngineInfo.AddMessageInfo(_module, _info, _type, time);
        }

    }
}
