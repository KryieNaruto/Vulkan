namespace ThirdPartyManager
{
    /// <summary>
    /// 单例工厂，第三方库管理基类
    /// </summary>
    public abstract class Manager
    {
        
        public Manager? Instance { get; private set; }
        public Manager() { }
    }
}
