#include "ace/safety/safety_manager.hpp"

#include <cstdio>

namespace ace::safety {

// SUMMARY: DONE: Tüm eksenleri devre dışı bırakarak sistemi güvenli bekleme durumuna (Safe State) alır.
void SafetyManager::enter_safe_state()
{
    safe_state_active_ = true;
    std::puts("SafetyManager: enter_safe_state");
}

// SUMMARY: DONE: Güvenlik kilidinin aktif olup olmadığını sorgular.
bool SafetyManager::safe_state_active() const
{
    return safe_state_active_;
}

}  // namespace ace::safety
