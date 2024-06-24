#include "utils.h"
#include <filesystem>
#include <fstream>
std::string ThirdParty::Utils::readFile(const std::string& _file_path)
{
	std::filesystem::path file_path = _file_path;
	if (std::filesystem::exists(file_path) && !std::filesystem::is_directory(file_path)) {
		std::ifstream input(file_path, std::ios::in);
		std::ostringstream buf;
		char c;
		if (input.is_open()) {
			while (buf && input.get(c)) buf.put(c);
			input.close();
		}
		return buf.str();
	}
	else {
		Core::warn(("Failed to read file:" + _file_path).c_str());
		return "";
	}
}
