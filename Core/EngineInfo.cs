using static Core.ZsEnum;

namespace Core
{
    public static class EngineInfo
    {
        // 消息
        internal static List<string> MessageInfo { set; get; } = new List<string>();
        // 消息类型
        internal static List<MESSAGE_TYPE> MessageType { set; get; } = new List<MESSAGE_TYPE>();
        // 消息模块位置
        internal static List<string> MessageModule { set; get; } = new List<string>();
        // 消息时间
        internal static List<string> MessageTime { set; get; } = new List<string>();
        public static void AddMessageInfo(string _module,string _info, MESSAGE_TYPE _type,string _time)
        {
            if (_info != "")
            {
                MessageType.Add(_type);
                MessageInfo.Add(_info);
                MessageModule.Add(_module);
                MessageTime.Add(_time);
            }
        }
        
    }
}
