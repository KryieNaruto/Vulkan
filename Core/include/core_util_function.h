#pragma once
#include <string>
#include <filesystem>
#include <windows.h>
#include <stdio.h>
#include <time.h>
#include <sys/timeb.h>
#include <memory>
#include <chrono>
#include "id_generate.h"
namespace Core {
	// lambda转指针函数
	// 弃用，由std::function代替
	/*using voidFuncType = void(*)();
	template<typename F>
	voidFuncType lambda2func(F lambda) {
		static auto lambda_back = lambda;
		return []() {lambda_back(); };
	}*/
	// 控制台输出
	template<typename ... Args>
	void output(const std::string& str, int color, Args ... args);
	// 白
	template<typename ... Args>
	void info(const char* str, Args ... args);
	// 绿
	template<typename ... Args>
	void success(const char* str, Args ... args);
	// 黄
	template<typename ... Args>
	void warn(const char* str, Args ... args);
	// 红
	template<typename ... Args>
	void error(const char* str, Args ... args);
	// 黄
	template<typename ... Args>
	void debug(const std::string&, Args ... args);

	// std::string 转换为 UTF-8 编码
	ENGINE_API_CORE std::string string_To_UTF8(const std::string& str);

	/*ENGINE_API_CORE void warn(const char* str, const char* args ...);
	ENGINE_API_CORE void error(const char* str, const char* args ...);
	ENGINE_API_CORE void success(const char* str, const char* args ...);
	ENGINE_API_CORE void debug(const char* str, const char* args ...);
	ENGINE_API_CORE void info(const char* str, const char* args ...);*/

	ENGINE_API_CORE void warn(const char* str,		const std::string& args ...);
	ENGINE_API_CORE void error(const char* str,		const std::string& args ...);
	ENGINE_API_CORE void success(const char* str,	const std::string& args ...);
	ENGINE_API_CORE void debug(const char* str,		const std::string& args ...);
	ENGINE_API_CORE void info(const char* str,		const std::string& args ...);
}

template<typename ... Args>
void Core::output(const std::string& str, int color, Args ... args)
{
	auto buf_size = std::snprintf(nullptr, 0, str.c_str(), args...) + 1;
	std::unique_ptr<char[]> buf(new(std::nothrow) char[buf_size]);
	if (!buf)return;
	std::snprintf(buf.get(), buf_size, str.c_str(), args...);
	const auto& str2 = std::string(buf.get(), buf.get() + buf_size - 1);

	if (str2.empty()) return;
	// 获取时间
	time_t time_p;
	struct tm p;
	time(&time_p);
	localtime_s(&p, &time_p);	// 精确到秒
	struct timeb pb;
	ftime(&pb);				// 精确到毫秒
	printf("[%.2d:%.2d:%.2d:%.3d]", p.tm_hour, p.tm_min, p.tm_sec, pb.millitm);
	HANDLE handle = GetStdHandle(STD_OUTPUT_HANDLE);
	SetConsoleTextAttribute(handle, FOREGROUND_INTENSITY | color);
	printf("[Core]:%s\n", str2.c_str());
	SetConsoleTextAttribute(handle, FOREGROUND_INTENSITY | 7);
}

template<typename ... Args>
void Core::info(const char* str, Args ... args)
{
	Core::output(str, 8, args...);
}

template<typename ... Args>
void Core::success(const char* str, Args ... args)
{
	Core::output(str, 2, args...);
}

template<typename ... Args>
void Core::warn(const char* str, Args ... args)
{
	Core::output(str, 6, args...);
}

template<typename ... Args>
void Core::error(const char* str, Args ... args)
{
	Core::output(str, 4, args...);
}

template<typename ... Args>
void Core::debug(const std::string& str, Args ... args)
{
	auto buf_size = std::snprintf(nullptr, 0, str.c_str(), args...) + 1;
	std::unique_ptr<char[]> buf(new(std::nothrow) char[buf_size]);
	if (!buf)return;
	std::snprintf(buf.get(), buf_size, str.c_str(), args...);
	auto str2 = std::string(buf.get(), buf.get() + buf_size - 1);

	if (str2.empty()) return;

	std::string rel_str;
	rel_str = rel_str.append(__FILE__).append(":").append(std::to_string(__LINE__)).append(":" + str2);

	Core::output(rel_str, 11);
}