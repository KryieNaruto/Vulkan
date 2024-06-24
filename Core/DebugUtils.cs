using System.Diagnostics;

namespace Core
{
    public static partial class ZsUtils
    {
        /// <summary>
        /// 返回当前方法的具体名称
        /// </summary>
        public static string GetMethodFullName()
        {
            var stack = new StackTrace(true);
            var method_parent = stack.GetFrame(1)?.GetMethod();
            if (method_parent == null) return "ERROR::Failed to get method info!";
            string systemModule = Environment.NewLine;
            systemModule += "模块名:" + method_parent.Module.ToString() + Environment.NewLine;
            systemModule += "命名空间名:" + method_parent.DeclaringType.Namespace + Environment.NewLine;
            //仅有类名
            systemModule += "类名:" + method_parent.DeclaringType.Name + Environment.NewLine;
            systemModule += "方法名:" + method_parent.Name;
            return systemModule;
        }
    }
}
