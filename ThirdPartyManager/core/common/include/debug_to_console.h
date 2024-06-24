#pragma once
#include <string>
#include <memory>
#include <format>
#include <cstring>
#include "core/info/include/application_window_info.h"
#undef new
namespace ThirdParty {
	namespace Core {
		ENGINE_API_THIRDPARTY void output(const char* str, int color);
		// 白
		ENGINE_API_THIRDPARTY void info(const char* str);
		// 绿
		ENGINE_API_THIRDPARTY void success(const char* str);
		// 黄
		ENGINE_API_THIRDPARTY void warn(const char* str);
		// 红
		ENGINE_API_THIRDPARTY void error(const char* str);
		// 获取模块路径
		ENGINE_API_THIRDPARTY std::string getModuleCurrentDirectory();

		// DEBUG
		template<typename ... Args>
		void debug(const std::string& str, Args ... args);

		template<typename ... Args>
		void info(const std::string& str, const Args& ... args);

		template<typename ... Args>
		void warn(const std::string& str, Args ... args);

		template<typename ... Args>
		void error(const std::string& str, Args ... args);
	}
}

template<typename  ... Args>
void ThirdParty::Core::info(const std::string& str,const Args& ... args)
{
	auto buf_size = std::snprintf(nullptr, 0, str.c_str(), args...) + 1;
	std::unique_ptr<char[]> buf(new(std::nothrow) char[buf_size]);
	if (!buf)return;
	std::snprintf(buf.get(), buf_size, str.c_str(), args...);
	const auto& str2 = std::string(buf.get(), buf.get() + buf_size - 1);

	output(str2.c_str(), 8);
}

template<typename ... Args>
void ThirdParty::Core::warn(const std::string& str, Args ... args)
{
	auto buf_size = std::snprintf(nullptr, 0, str.c_str(), args...) + 1;
	std::unique_ptr<char[]> buf(new(std::nothrow) char[buf_size]);
	if (!buf)return;
	std::snprintf(buf.get(), buf_size, str.c_str(), args...);
	const auto& str2 = std::string(buf.get(), buf.get() + buf_size - 1);

	output(str2.c_str(), 6);
}


template<typename ... Args>
void ThirdParty::Core::debug(const std::string& str, Args ... args)
{
	auto buf_size = std::snprintf(nullptr, 0, str.c_str(), args...) + 1;
	std::unique_ptr<char[]> buf(new(std::nothrow) char[buf_size]);
	if (!buf)return;
	std::snprintf(buf.get(), buf_size, str.c_str(), args...);
	auto str2 = std::string(buf.get(), buf.get() + buf_size - 1);

	if (str2.empty()) return;

	std::string rel_str;
	rel_str = rel_str.append(__FILE__).append(":").append(std::to_string(__LINE__)).append(":" + str2);

	Core::output(rel_str.c_str(), 11);
}

template<typename ... Args>
void ThirdParty::Core::error(const std::string& str, Args ... args)
{
	auto buf_size = std::snprintf(nullptr, 0, str.c_str(), args...) + 1;
	std::unique_ptr<char[]> buf(new(std::nothrow) char[buf_size]);
	if (!buf)return;
	std::snprintf(buf.get(), buf_size, str.c_str(), args...);
	const auto& str2 = std::string(buf.get(), buf.get() + buf_size - 1);

	output(str2.c_str(), 4);
}