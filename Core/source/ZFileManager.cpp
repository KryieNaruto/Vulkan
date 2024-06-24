#include "ZFileManager.h"
#include "core_util_function.h"
#include <Windows.h>
#include <fstream>
#pragma warning(disable:4251)
namespace fs = std::filesystem;
std::vector<Core::FileInfo*> Core::ZFileManager::getFileList(std::string& root_path, bool only_folder /*= false*/, const std::string& suffix)
{
	std::vector<Core::FileInfo*> files;
	// 清空files
	if (!files.empty()) { files.resize(0); files.shrink_to_fit(); }
	// 如果路径为空，则读取盘符
	if (std::string(root_path) == "") {
		files.clear(); files.shrink_to_fit();
		// 空根
		FileInfo* root = new FileInfo();
		files.push_back(root);

		CONST SIZE_T BUFSIZE = 64;
		TCHAR szLogicDriveStrings[BUFSIZE];
		PCHAR szDrive;
		ZeroMemory(szLogicDriveStrings, BUFSIZE);
		GetLogicalDriveStrings(BUFSIZE - 1, szLogicDriveStrings);
		szDrive = (PCHAR)szLogicDriveStrings;
		char driver[4] = { '\0' }; uint8_t name_i = 0;
		while (szDrive != 0) {
			if (*szDrive != '\0') {
				driver[name_i] = *szDrive;
				name_i = (name_i + 1) % 3;
			}
			else {
				if (driver[0] == '\0') break;
				const auto& p = std::filesystem::path(std::string(driver));
				const auto& e = std::filesystem::exists(p);
				Core::FileInfo* f = new Core::FileInfo(std::filesystem::directory_entry(p));
				f->p_parent_folder = root;
				files.push_back(f);
				driver[0] = '\0';
			}
			szDrive++;
		}
		return files;
	};
	const auto& path = std::filesystem::path(root_path);
	// 获取绝对路径
	const auto absolute_folder_path = std::filesystem::absolute(path);
	// 判断是否存在
	const auto is_exist = std::filesystem::exists(path);
	// 判断是否为文件
	const auto is_directory = std::filesystem::is_directory(path);
	if (!(is_exist && (is_directory || !only_folder))) {
		Core::warn("%s does not exist or is not a folder", absolute_folder_path.string().c_str());
		return files;
	}
	// 遍历文件夹下子文件
	const auto& folder_entry = std::filesystem::directory_entry(path);
	FileInfo* root = new FileInfo(folder_entry);
	files.push_back(root);
	for (const auto& ite : std::filesystem::directory_iterator(path)) {
		// 判断是否隐藏
		const auto is_not_hidden = (GetFileAttributes(ite.path().string().c_str()) & FILE_ATTRIBUTE_HIDDEN) != FILE_ATTRIBUTE_HIDDEN;
		// 判断后缀
		bool has_not_suffix = !ite.path().has_extension();
		bool has_suffix_and_equal = !has_not_suffix;
		if (!suffix.empty() || suffix != "") {
			if (has_suffix_and_equal) {
				const auto& str = ite.path().extension().string();
				has_suffix_and_equal &= (strcmp(str.c_str(), suffix.c_str()) == 0);
			}
		}
		if (!is_not_hidden || (!has_suffix_and_equal && !has_not_suffix)) continue;
		FileInfo* file = new FileInfo(ite);
		file->p_parent_folder = root;
		files.push_back(file);
	}

	return files;
}

std::vector<Core::FileInfo*> Core::ZFileManager::getFileList(std::string& root_path, bool only_folder /*= false*/, const std::vector<std::string>& suffix /*= {}*/)
{
	std::vector<Core::FileInfo*> files;
	// 清空files
	if (!files.empty()) { files.resize(0); files.shrink_to_fit(); }
	// 如果路径为空，则读取盘符
	if (std::string(root_path) == "") {
		files.clear(); files.shrink_to_fit();
		// 空根
		FileInfo* root = new FileInfo();
		files.push_back(root);

		CONST SIZE_T BUFSIZE = 64;
		TCHAR szLogicDriveStrings[BUFSIZE];
		PCHAR szDrive;
		ZeroMemory(szLogicDriveStrings, BUFSIZE);
		GetLogicalDriveStrings(BUFSIZE - 1, szLogicDriveStrings);
		szDrive = (PCHAR)szLogicDriveStrings;
		char driver[4] = { '\0' }; uint8_t name_i = 0;
		while (szDrive != 0) {
			if (*szDrive != '\0') {
				driver[name_i] = *szDrive;
				name_i = (name_i + 1) % 3;
			}
			else {
				if (driver[0] == '\0') break;
				const auto& p = std::filesystem::path(std::string(driver));
				const auto& e = std::filesystem::exists(p);
				Core::FileInfo* f = new Core::FileInfo(std::filesystem::directory_entry(p));
				f->p_parent_folder = root;
				files.push_back(f);
				driver[0] = '\0';
			}
			szDrive++;
		}
		return files;
	};
	const auto& path = std::filesystem::path(root_path);
	// 获取绝对路径
	const auto absolute_folder_path = std::filesystem::absolute(path);
	// 判断是否存在
	const auto is_exist = std::filesystem::exists(path);
	// 判断是否为文件
	const auto is_directory = std::filesystem::is_directory(path);
	if (!(is_exist && (is_directory || !only_folder))) {
		Core::warn("%s does not exist or is not a folder", absolute_folder_path.string().c_str());
		return files;
	}
	// 遍历文件夹下子文件
	const auto& folder_entry = std::filesystem::directory_entry(path);
	FileInfo* root = new FileInfo(folder_entry);
	files.push_back(root);
	for (const auto& ite : std::filesystem::directory_iterator(path)) {
		// 判断是否隐藏
		const auto is_not_hidden = (GetFileAttributes(ite.path().string().c_str()) & FILE_ATTRIBUTE_HIDDEN) != FILE_ATTRIBUTE_HIDDEN;
		// 判断后缀
		bool has_not_suffix = !ite.path().has_extension();
		bool has_suffix_and_equal = false;
		if (!has_not_suffix) {
			for (const auto& _suffix : suffix) {
				if (!_suffix.empty() || _suffix != "") {
					const auto& str = ite.path().extension().string();
					bool _equal = (strcmp(str.c_str(), _suffix.c_str()) == 0);
					if (_equal) {
						has_suffix_and_equal = true;
						break;
					}
				}
			}
		}
		if (!is_not_hidden || (!has_suffix_and_equal && !has_not_suffix)) continue;
		FileInfo* file = new FileInfo(ite);
		file->p_parent_folder = root;
		files.push_back(file);
	}

	return files;
}

Core::FileInfo* Core::ZFileManager::createFolder(std::string root_path, const char* folder_name)
{
	return createFolder(root_path + "\\" + folder_name);
}

Core::FileInfo* Core::ZFileManager::createFile(std::string file_path)
{
	if (file_is_exists(file_path.c_str())) return new Core::FileInfo(fs::directory_entry(file_path));
	else {
		std::ofstream newFile(file_path, std::ios::ate);
		if (!newFile) {
			Core::error("Failed to create file:%s",file_path.c_str());
			return nullptr;
		}
		newFile.close();
		return new Core::FileInfo(fs::directory_entry(file_path));
	}
}

bool Core::ZFileManager::file_is_exists(const char* file_path)
{
	return fs::exists(fs::path(file_path));
}

Core::FileInfo* Core::ZFileManager::createFolder(std::string folder_path)
{
	auto path = folder_path;
	int i = 1;
	while (folder_is_exists(path.c_str())) {
		path = folder_path + "(" + std::to_string(i++) + ")";
	}
	const auto& success = fs::create_directories(path);
	if (success) {
		Core::FileInfo* result = new Core::FileInfo(fs::directory_entry(path));
		return result;
	}
	else {
		Core::warn("Failed to create a new folder:%s", path.c_str());
		return nullptr;
	}
}

bool Core::ZFileManager::folder_is_exists(const char* folder_path)
{
	return fs::exists(fs::path(folder_path));
}

std::string Core::ZFileManager::getFilePath(const std::string& _file_path)
{
	if (file_is_exists(_file_path.c_str())) {
		fs::path _p(_file_path);
		if (_p.has_extension()) {
			const size_t found = _file_path.find_last_of("/\\");
			return (found == std::string::npos) ? _file_path : _file_path.substr(0, found);
		}
	}
}
