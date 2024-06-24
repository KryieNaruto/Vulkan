#include "include/application.h"
#include <Core/include/compute_info.h>

int main(int argc, char** args) {
	Application* app = new Application();
	delete app;

	Core::ComputeInfo* cmp_info = Core::createComputeInfo();
	cmp_info->printInfo();
	Core::destroyComputeInfo(cmp_info);

	return 0;
}
