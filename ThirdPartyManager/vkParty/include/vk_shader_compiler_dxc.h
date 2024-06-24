#pragma once
#include <string>
#include "application_window_info.h"
namespace ThirdParty {
	//===============================
	// ��ʱ����������
	// Ĭ�ϱ�����·��:".\\bin\\dxc.exe"
	// Ĭ�ϲ���·��:"..\\resources\\hlsl"
	// Ĭ�����·��:"..\\resources\\spirv"
	//===============================
	class ENGINE_API_THIRDPARTY VkShaderCompiler {
	public:
		// ���ò���Ŀ¼
		static void setCompileRootPath(const char* rootPath);
		// �������Ŀ¼
		static void setCompileOutPath(const char* outPath);

		//************************************
		// Method:    compile
		// FullName:  ThirdParty::VkShaderCompiler::compile
		// Access:    public static 
		// Returns:   void
		// Qualifier: �����ļ���spirv
		// Ĭ�ϱ�����·��:".\\bin\\dxc.exe"
		// Ĭ�ϲ���·��:"..\\resources\\hlsl"
		// Ĭ�����·��:"..\\resources\\spirv"
		// Parameter: const char * fileName	�ļ���
		// Parameter: const char * entryPoint ��ڵ�
		// Parameter: const char * outName ����ļ���
		// Parameter: const char * type ��ɫ������(ps_6_0,vs_6_0...)
		//************************************
		static void compile(const char* fileName, const char* entryPoint, const char* outName,const char* type);
		
	private:
		static const char* COMPILER_PATH;	// ������·��
		static std::string COMPILE_ROOT;	// Ĭ�ϲ���·��
		static std::string COMPILE_OUT;		// Ĭ�����·��
		static std::string m_final_command;		// ��������
	protected:
	};
}

