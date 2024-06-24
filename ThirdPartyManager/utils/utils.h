#pragma once
#include <string>
#include "core/common/include/debug_to_console.h"

#define VECTOR_SIZEOF(_vec) _vec.size() * sizeof(_vec[0])


namespace ThirdParty::Utils {
	std::string readFile(const std::string& _file_path);
}
