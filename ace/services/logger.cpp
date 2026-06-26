#include "ace/services/logger.hpp"

#include <cstdio>

namespace ace::services {

void Logger::set_debug_enabled(bool enabled)
{
    debug_enabled_ = enabled;
}

void Logger::info(const char* message)
{
    std::printf("[INFO] %s\n", message);
}

void Logger::debug(const char* area, const char* message)
{
    if (!debug_enabled_) {
        return;
    }

    std::printf("[DEBUG][%s] %s\n", area, message);
}

}  // namespace ace::services
