#pragma once

namespace ace::safety {

class SafetyManager {
public:
    void enter_safe_state();
    [[nodiscard]] bool safe_state_active() const;

private:
    bool safe_state_active_ = false;
};

}  // namespace ace::safety
