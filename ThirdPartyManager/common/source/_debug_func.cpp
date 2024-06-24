#ifndef _WIN32
#include <string.h>
#define strncpy_s strncpy
#endif
#ifdef _DEBUG
#include "_debug_func.h"

ENGINE_API_THIRDPARTY AllocList* allocList;
void AddTrack(void* _ptr, size_t size, const char* file, int line)
{
	ALLOC_INFO* p_info;
	if (!allocList)
		allocList = new (AllocList);
	p_info = new ALLOC_INFO;
	p_info->address = (DWORD)_ptr;
	strncpy_s(p_info->file, file, 256);
	p_info->line = line;
	p_info->size = size;
	(*allocList)[_ptr] = p_info;
}

void RemoveTrack(void* ptr)
{
	AllocList::iterator i;
	if (!allocList)
		return;
	auto _ite = allocList->find(ptr);
	if (_ite != allocList->end()) {
		delete _ite->second;
		allocList->erase(_ite);
	}
}

void* _cdecl operator new(size_t size, const char* file, int line) noexcept{
	void* ptr = (void*)malloc(size);
	AddTrack(ptr, size, file, line);
	return ptr;
}

void _cdecl operator delete(void* p) noexcept{
	RemoveTrack(p);
	free(p);
}
#endif