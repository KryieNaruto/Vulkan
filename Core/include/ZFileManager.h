#pragma once
#include "ZClass.h"
#include "core_struct.h"

namespace Core {
	// �ļ�������
	class ENGINE_API_CORE ZFileManager :public ZClass
	{
	public:
		ZFileManager() = default;
		virtual ~ZFileManager() {};

		// ��ȡroot·���µ��ļ�Ŀ¼,�����ļ��б�
		std::vector<FileInfo*> getFileList(std::string& root_path,bool only_folder = false,const std::string& suffix = "");
		std::vector<FileInfo*> getFileList(std::string& root_path,bool only_folder, const std::vector<std::string>& suffix);
		// �½��ļ���
		FileInfo* createFolder(std::string root_path, const char* folder_name);
		FileInfo* createFolder(std::string folder_path);
		// �½��ļ�
		FileInfo* createFile(std::string file_path);
		// �ж��ļ��Ƿ����
		bool file_is_exists(const char* file_path);
		bool folder_is_exists(const char* folder_path);
		// ��ȡ�ļ�Ŀ¼
		std::string getFilePath(const std::string& _file_path);
	private: 
	protected:
	};
}

