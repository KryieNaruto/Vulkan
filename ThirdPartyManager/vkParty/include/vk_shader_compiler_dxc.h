#pragma once
#include <string>
#include "application_window_info.h"
namespace ThirdParty {
	//===============================
	// 临时编译管理对象
	// 默认编译器路径:".\\bin\\dxc.exe"
	// 默认查找路径:"..\\resources\\hlsl"
	// 默认输出路径:"..\\resources\\spirv"
	//===============================
	class ENGINE_API_THIRDPARTY VkShaderCompiler {
	public:
		// 设置查找目录
		static void setCompileRootPath(const char* rootPath);
		// 设置输出目录
		static void setCompileOutPath(const char* outPath);

		//************************************
		// Method:    compile
		// FullName:  ThirdParty::VkShaderCompiler::compile
		// Access:    public static 
		// Returns:   void
		// Qualifier: 编译文件到spirv
		// 默认编译器路径:".\\bin\\dxc.exe"
		// 默认查找路径:"..\\resources\\hlsl"
		// 默认输出路径:"..\\resources\\spirv"
		// Parameter: const char * fileName	文件名
		// Parameter: const char * entryPoint 入口点
		// Parameter: const char * outName 输出文件名
		// Parameter: const char * type 着色器类型(ps_6_0,vs_6_0...)
		//************************************
		static void compile(const char* fileName, const char* entryPoint, const char* outName,const char* type);
		
	private:
		static const char* COMPILER_PATH;	// 编译器路径
		static std::string COMPILE_ROOT;	// 默认查找路径
		static std::string COMPILE_OUT;		// 默认输出路径
		static std::string m_final_command;		// 最终命令
	protected:
	};
}

