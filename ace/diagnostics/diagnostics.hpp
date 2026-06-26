#pragma once

namespace ace::diagnostics {

class Diagnostics {
public:
    [[nodiscard]] bool run_self_test() const;
};

}  // namespace ace::diagnostics
