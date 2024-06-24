using Core;
using static Core.ZsEnum;

namespace ThirdPartyManager
{
    public class SDLException : ZsException
    {
        
        public SDLException(string _module, string _info, MESSAGE_TYPE _type) : base(_module,_info,_type)
        {

        }
    }
}
