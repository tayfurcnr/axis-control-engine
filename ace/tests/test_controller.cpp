#include "../control/controller.hpp"
#include <iostream>
#include <cassert>
#include <cmath>

void test_pid_basic() {
    ace::control::Controller ctrl;
    ctrl.set_gains(1.0f, 0.0f, 0.0f, 0.0f); // Pure P
    ctrl.set_limits(0.0f, 100.0f);
    
    // Setpoint 10.0, current 0.0 -> error = 10.0 -> * Kp = 10.0
    float output = ctrl.update(10.0f, 0.0f, 0.01f);
    assert(std::abs(output - 10.0f) < 0.001f);
    std::cout << "test_pid_basic passed.\n";
}

void test_pid_clamping() {
    ace::control::Controller ctrl;
    ctrl.set_gains(10.0f, 0.0f, 0.0f, 0.0f); // Pure P
    ctrl.set_limits(0.0f, 50.0f); // Max output 50.0
    
    // Setpoint 10.0, current 0.0 -> error = 10.0 -> * Kp = 100.0 -> Clamped to 50.0
    float output = ctrl.update(10.0f, 0.0f, 0.01f);
    assert(std::abs(output - 50.0f) < 0.001f);
    std::cout << "test_pid_clamping passed.\n";
}

void test_pid_integral_windup() {
    ace::control::Controller ctrl;
    ctrl.set_gains(0.0f, 10.0f, 0.0f, 0.0f); // Pure I
    ctrl.set_limits(5.0f, 100.0f); // Max integral 5.0
    
    // error = 1.0, dt = 1.0 -> added 1.0
    ctrl.update(1.0f, 0.0f, 1.0f);
    // error = 10.0, dt = 1.0 -> added 10.0 -> clamped to 5.0
    float output = ctrl.update(10.0f, 0.0f, 1.0f);
    
    // Output = Ki * integral = 10.0 * 5.0 = 50.0
    assert(std::abs(output - 50.0f) < 0.001f);
    std::cout << "test_pid_integral_windup passed.\n";
}

int main() {
    std::cout << "Running Controller Tests...\n";
    test_pid_basic();
    test_pid_clamping();
    test_pid_integral_windup();
    std::cout << "All Controller Tests passed!\n";
    return 0;
}
