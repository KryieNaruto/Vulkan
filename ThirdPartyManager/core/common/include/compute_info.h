#pragma
#ifdef _WIN32
#include <windows.h>

namespace ThirdParty {
	namespace Core {
		class ComputeInfo {
		public:
			ComputeInfo();
			virtual ~ComputeInfo();

			void printInfo();

			char* getStart();
			char* getSystem();
			char* getCPU();
			char* getGPU();
		private:
			HINSTANCE m_hinst = NULL;
			char* m_psystem = nullptr;
			char* m_pcpu = nullptr;
			char* m_pgpu = nullptr;
			char* m_pstart = nullptr;
		};

		_declspec(dllexport) inline ComputeInfo* createComputeInfo() { return new ComputeInfo(); }
		_declspec(dllexport) inline void destroyComputeInfo(ComputeInfo* pInfo) { delete pInfo; }
	}
}
#endif