#include "core_util_function.h"
#include <ThirdPartyManager/third_party_manager_global.h>

std::string Core::string_To_UTF8(const std::string& str) {
	return ThirdParty::string_To_UTF8(str);
}

ENGINE_API_CORE void Core::warn(const char* str, const std::string& args ...) {
	Core::warn< const std::string& >(str, args);
}
ENGINE_API_CORE void Core::error(const char* str, const std::string& args ...) {
	Core::error< const std::string& >(str, args);
}
ENGINE_API_CORE void Core::success(const char* str, const std::string& args ...) {
	Core::success< const std::string& >(str, args);
}
ENGINE_API_CORE void Core::debug(const char* str, const std::string& args ...) {
	Core::debug< const std::string& >(str, args);
}
ENGINE_API_CORE void Core::info(const char* str, const std::string& args ...) {
	Core::info< const std::string& >(str, args);
}

