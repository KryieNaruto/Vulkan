#ifdef _WIN32
#include "debug_to_console.h"
#include <windows.h>
#include <stdio.h>
#include <time.h>
#include <sys/timeb.h>
#include <memory>
using namespace ThirdParty;
void Core::output(const char* str, int color)
{
	if (!str) return;
	// 获取时间
	time_t time_p;
	struct tm p;
	time(&time_p);
	localtime_s(&p,&time_p);	// 精确到秒
	struct timeb pb;
	ftime(&pb);				// 精确到毫秒
	printf("[%.2d:%.2d:%.2d:%.3d]", p.tm_hour, p.tm_min, p.tm_sec, pb.millitm);
	HANDLE handle = GetStdHandle(STD_OUTPUT_HANDLE);
	SetConsoleTextAttribute(handle, FOREGROUND_INTENSITY | color);
	printf("[ThirdParty::Core]:%s\n", str);
	SetConsoleTextAttribute(handle, FOREGROUND_INTENSITY | 7);
}

void Core::info(const char* str)
{
	output(str, 8);
}

void Core::success(const char* str)
{
	output(str, 2);
}

void Core::warn(const char* str)
{
	output(str, 6);
}

void Core::error(const char* str)
{
	output(str, 4);
}

std::string Core::getModuleCurrentDirectory()
{
	char test[1024];
	GetModuleFileNameA(NULL, test, 1024);
	(strrchr(test, '\\'))[0] = 0;
	return test;
}
#endif
