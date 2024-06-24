#pragma once
#include "ZClass.h"
#include "core_struct.h"

namespace Core {
	// 文件管理器
	class ENGINE_API_CORE ZFileManager :public ZClass
	{
	public:
		ZFileManager() = default;
		virtual ~ZFileManager() {};

		// 获取root路径下的文件目录,返回文件列表
		std::vector<FileInfo*> getFileList(std::string& root_path,bool only_folder = false,const std::string& suffix = "");
		std::vector<FileInfo*> getFileList(std::string& root_path,bool only_folder, const std::vector<std::string>& suffix);
		// 新建文件夹
		FileInfo* createFolder(std::string root_path, const char* folder_name);
		FileInfo* createFolder(std::string folder_path);
		// 新建文件
		FileInfo* createFile(std::string file_path);
		// 判断文件是否存在
		bool file_is_exists(const char* file_path);
		bool folder_is_exists(const char* folder_path);
		// 获取文件目录
		std::string getFilePath(const std::string& _file_path);
	private: 
	protected:
	};
}

