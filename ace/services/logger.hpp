#pragma once

namespace ace::services {

class Logger {
public:
    void set_debug_enabled(bool enabled);
    void info(const char* message);
    void debug(const char* area, const char* message);

private:
    bool debug_enabled_ = false;
};

}  // namespace ace::services
