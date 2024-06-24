#pragma once
#include "core/info/include/application_window_info.h"
#include "math_util.h"
#include "_debug_func.h"
#include <vector>
#include <new>

#if ((defined _DEBUG) && (defined _WIN32))
#define DEBUG_NEW new(__FILE__,__LINE__)
#else
#define DEBUG_NEW new
#endif
#define new DEBUG_NEW

namespace ThirdParty {
	//==========================
	// 抽象类
	// 负责第三方库的统一管理
	//==========================
	template<typename T>
	class Manager
	{
	public:
		Manager() = default;
		virtual ~Manager();
		virtual inline T* getManager() { return (T*)this; };
		virtual T* init() = 0;
		virtual void destroy() = 0;
		virtual void windowResize() = 0;
		inline bool isReady() { return this->m_isInitialized; }
	protected:
		bool m_isInitialized = { false };
	private:
	};

	template<typename T>
	ThirdParty::Manager<T>::~Manager()
	{
	}

	inline ENGINE_API_THIRDPARTY std::string string_To_UTF8(const std::string& str) {
#ifdef _WIN32
		int nwLen = ::MultiByteToWideChar(CP_ACP, 0, str.c_str(), -1, NULL, 0);

		wchar_t* pwBuf = new wchar_t[nwLen + 1];//一定要加1，不然会出现尾巴
		ZeroMemory(pwBuf, nwLen * 2 + 2);

		::MultiByteToWideChar(CP_ACP, 0, str.c_str(), static_cast<int>(str.length()), pwBuf, nwLen);

		int nLen = ::WideCharToMultiByte(CP_UTF8, 0, pwBuf, -1, NULL, NULL, NULL, NULL);

		char* pBuf = new char[nLen + 1];
		ZeroMemory(pBuf, nLen + 1);

		::WideCharToMultiByte(CP_UTF8, 0, pwBuf, nwLen, pBuf, nLen, NULL, NULL);

		std::string retStr(pBuf);

		delete[]pwBuf;
		delete[]pBuf;

		pwBuf = NULL;
		pBuf = NULL;
        return retStr;
#else   // LINUX
        return str;
#endif
    }

}

