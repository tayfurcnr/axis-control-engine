#pragma once

#include <string_view>

namespace ace::communication {

struct AlpFrame {
    std::string_view raw;
};

class AlpParser {
public:
    [[nodiscard]] AlpFrame parse(std::string_view message) const;
};

}  // namespace ace::communication
