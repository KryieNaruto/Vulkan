#pragma once
#include <malloc.h>
#include <unordered_map>

#ifdef _WIN32
    #include <Windows.h>
#else
    #define _cdecl __attribute__((__cdecl__))
    #ifndef DWORD
        #define WINAPI
        typedef unsigned long DWORD;
    #endif
#endif
// ÄÚ´æÐ¹Â©×·×Ù
#ifdef _DEBUG
#include "core/info/include/application_window_info.h"
typedef struct {
	DWORD address;
	DWORD size;
	char file[256];
	DWORD line;
} ALLOC_INFO;

typedef std::unordered_map<void*, ALLOC_INFO*> AllocList;

ENGINE_API_THIRDPARTY extern AllocList* allocList;

void AddTrack(void* _ptr, size_t size, const char* file, int line);

void RemoveTrack(void* ptr);

void* _cdecl operator new(size_t size, const char* file, int line) noexcept;

void _cdecl operator delete(void* p) noexcept;

#endif