#include <cassert>
#include <iostream>
#include "ace/safety/safety_manager.hpp"

using namespace ace::safety;

void test_no_fault_initial()
{
    SafetyManager sm;
    assert(!sm.has_fault());
    assert(!sm.is_safe_state_active());
    assert(sm.active_fault_bitmap() == 0);
    std::cout << "test_no_fault_initial passed.\n";
}

void test_warning_does_not_enter_safe_state()
{
    SafetyManager sm;
    sm.report_fault(FaultCode::COMMAND_TIMEOUT);
    assert(sm.has_fault());
    assert(sm.has_fault(FaultCode::COMMAND_TIMEOUT));
    // WARNING seviyesi safe state'e girmemeli
    assert(!sm.is_safe_state_active());
    std::cout << "test_warning_does_not_enter_safe_state passed.\n";
}

void test_critical_enters_safe_state()
{
    SafetyManager sm;
    sm.report_fault(FaultCode::AXIS_PAN_OVERRUN);
    assert(sm.has_fault(FaultCode::AXIS_PAN_OVERRUN));
    // CRITICAL safe state tetiklemeli
    assert(sm.is_safe_state_active());
    std::cout << "test_critical_enters_safe_state passed.\n";
}

void test_emergency_stop_cannot_be_cleared()
{
    SafetyManager sm;
    sm.report_fault(FaultCode::EMERGENCY_STOP);
    assert(sm.is_safe_state_active());
    // EMERGENCY temizlenemez
    bool cleared = sm.clear_fault(FaultCode::EMERGENCY_STOP);
    assert(!cleared);
    assert(sm.has_fault(FaultCode::EMERGENCY_STOP));
    std::cout << "test_emergency_stop_cannot_be_cleared passed.\n";
}

void test_clear_fault()
{
    SafetyManager sm;
    sm.report_fault(FaultCode::COMMAND_TIMEOUT);
    assert(sm.has_fault(FaultCode::COMMAND_TIMEOUT));
    bool cleared = sm.clear_fault(FaultCode::COMMAND_TIMEOUT);
    assert(cleared);
    assert(!sm.has_fault(FaultCode::COMMAND_TIMEOUT));
    std::cout << "test_clear_fault passed.\n";
}

void test_fault_bitmap()
{
    SafetyManager sm;
    sm.report_fault(FaultCode::AXIS_PAN_OVERRUN);
    sm.report_fault(FaultCode::COMMAND_TIMEOUT);
    std::uint32_t bm = sm.active_fault_bitmap();
    assert(bm & static_cast<std::uint32_t>(FaultCode::AXIS_PAN_OVERRUN));
    assert(bm & static_cast<std::uint32_t>(FaultCode::COMMAND_TIMEOUT));
    std::cout << "test_fault_bitmap passed.\n";
}

void test_highest_fault_priority()
{
    SafetyManager sm;
    sm.report_fault(FaultCode::COMMAND_TIMEOUT);   // WARNING
    sm.report_fault(FaultCode::AXIS_PAN_OVERRUN);  // CRITICAL
    auto top = sm.highest_active_fault();
    assert(top.has_value());
    assert(top->severity == FaultSeverity::CRITICAL);
    std::cout << "test_highest_fault_priority passed.\n";
}

void test_can_exit_safe_state_only_after_critical_cleared()
{
    SafetyManager sm;
    sm.report_fault(FaultCode::AXIS_PAN_OVERRUN);  // CRITICAL → safe state
    assert(!sm.can_exit_safe_state());
    sm.clear_fault(FaultCode::AXIS_PAN_OVERRUN);
    assert(sm.can_exit_safe_state());
    std::cout << "test_can_exit_safe_state_only_after_critical_cleared passed.\n";
}

int main()
{
    std::cout << "Running SafetyManager Tests...\n";
    test_no_fault_initial();
    test_warning_does_not_enter_safe_state();
    test_critical_enters_safe_state();
    test_emergency_stop_cannot_be_cleared();
    test_clear_fault();
    test_fault_bitmap();
    test_highest_fault_priority();
    test_can_exit_safe_state_only_after_critical_cleared();
    std::cout << "All SafetyManager Tests passed!\n";
    return 0;
}
