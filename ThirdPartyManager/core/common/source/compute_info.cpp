#ifdef _WIN32
#include "compute_info.h"
#include <stdexcept>
#include <intrin.h>
#include <array>
#include <time.h>
#include <vector>
#pragma warning(disable: 4996)
using namespace ThirdParty::Core;
ComputeInfo::ComputeInfo()
{
	this->m_pstart = getStart();
	// 加载ntdll.dll
	this->m_hinst = LoadLibrary(TEXT("ntdll.dll"));
	if (NULL == this->m_hinst) {
		throw std::runtime_error("Failed to load ntdll.dll");
	}
	// 获取系统规格
	this->m_psystem = getSystem();
	// 获取CPU
	this->m_pcpu = getCPU();

	FreeLibrary(this->m_hinst);
}

ComputeInfo::~ComputeInfo()
{
	if (this->m_psystem)
		delete[] m_psystem;
	if (this->m_pcpu)
		delete[] m_pcpu;
	if (this->m_pgpu)
		delete[] m_pgpu;
	if (this->m_pstart)
		delete[] m_pstart;
}

void ComputeInfo::printInfo()
{
	printf("=================================================\n");
	printf("Start\t:%s", this->getStart());
	printf("OS\t:%s\n", this->getSystem());
	printf("CPU\t:%s\n", this->getCPU());
	printf("=================================================\n");
}


char* ComputeInfo::getStart()
{
	if (this->m_pstart)
		return m_pstart;
	// 获取当前时间
	time_t time_temp = time(NULL);
	char* s = ctime(&time_temp);
	strcat(s, "\0");
	this->m_pstart = new char[strlen(s) + 1];
	strcpy(this->m_pstart, s);
	return this->m_pstart;
}

char* ComputeInfo::getSystem()
{
	if (this->m_psystem)
		return this->m_psystem;

	typedef void (WINAPI* pfRTLGETNTVERSIONNUMBERS)(DWORD*, DWORD*, DWORD*);
	pfRTLGETNTVERSIONNUMBERS pfRtlGetNtVersionNumbers;
	pfRtlGetNtVersionNumbers = (pfRTLGETNTVERSIONNUMBERS)::GetProcAddress(this->m_hinst, "RtlGetNtVersionNumbers");
	char* result = new char[64];
	char* temp = new char[16];
	DWORD dwMajorVer = 0, dwMinorVer = 0, dwBuildNumber = 0;
	if (pfRtlGetNtVersionNumbers)
	{
		pfRtlGetNtVersionNumbers(&dwMajorVer, &dwMinorVer, &dwBuildNumber);
		dwBuildNumber &= 0x0ffff;
	}

#ifdef _WIN32
	strcpy(result, "Microsoft Windows [Version");
	strcat(result, " ");
	strcat(result, _ultoa(dwMajorVer, temp, 10));
	strcat(result, ".");
	strcat(result, _ultoa(dwMinorVer, temp, 10));
	strcat(result, ".");
	strcat(result, _ultoa(dwBuildNumber, temp, 10));
	strcat(result, "]");
#ifdef _WIN64
	strcat(result, " x64\0");
#else
	strcat(result, " x32\0");
#endif
#else
	strcpy(result, "ERROR:IS NOT SUPPORT!");
#endif
	size_t len = strlen(result) + 1;
	this->m_psystem = new char[len];
	strcpy(this->m_psystem, result);
	delete[] result;
	delete[] temp;
	return this->m_psystem;
}

char* ComputeInfo::getCPU()
{
	if (this->m_pcpu)
		return m_pcpu;
	std::array<int, 4> cpui;
	std::vector<std::array<int, 4>> extdata_;

	__cpuid(cpui.data(), 0x80000000);
	int nExIds_ = cpui[0];

	char brand[0x40];
	memset(brand, 0, sizeof(brand));

	for (int i = 0x80000000; i <= nExIds_; ++i)
	{
		__cpuidex(cpui.data(), i, 0);
		extdata_.push_back(cpui);
	}

	// Interpret CPU brand string if reported
	if (nExIds_ >= 0x80000004)
	{
		memcpy(brand, extdata_[2].data(), sizeof(cpui));
		memcpy(brand + 16, extdata_[3].data(), sizeof(cpui));
		memcpy(brand + 32, extdata_[4].data(), sizeof(cpui));
		strcat(brand, "\0");
	}

	this->m_pcpu = new char[strlen(brand)+1];
	strcpy(this->m_pcpu, brand);

	return this->m_pcpu;
}

char* ComputeInfo::getGPU()
{
	if (this->m_pgpu)
		return this->m_pgpu;
	return nullptr;
}

#endif
