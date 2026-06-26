#include "../motion/motion_planner.hpp"
#include <iostream>
#include <iomanip>

int main() {
    ace::motion::MotionPlanner planner;
    planner.set_limits(60.0f, 30.0f); // 60 dps max vel, 30 dps2 max accel
    planner.reset(0.0f);
    planner.set_target(90.0f);

    std::cout << "Starting trapezoidal motion profile simulation: 0.0 -> 90.0 degrees\n";
    std::cout << "Max Vel: 60.0, Max Accel: 30.0\n";
    std::cout << "------------------------------------\n";

    float time = 0.0f;
    const float dt = 0.1f; // 10 Hz simulation loop for viewing

    while (time <= 4.0f) {
        auto state = planner.update(dt);
        
        std::cout << "T: " << std::fixed << std::setprecision(1) << time 
                  << "s | Pos: " << std::setw(6) << std::setprecision(2) << state.position_deg 
                  << " deg | Vel: " << std::setw(6) << std::setprecision(2) << state.velocity_dps << " dps\n";
                  
        if (state.position_deg == 90.0f && state.velocity_dps == 0.0f) {
            std::cout << "Target reached successfully at T=" << time << "s\n";
            break;
        }

        time += dt;
    }

    return 0;
}
