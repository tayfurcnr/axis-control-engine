#include "../geometry/target_geometry.hpp"

#include <cassert>
#include <cmath>
#include <iostream>

void test_geometric_projection()
{
    float pan_deg = 0.0f;
    float tilt_deg = 0.0f;

    ace::geometry::calculate_target_angles(0.0, 0.0, 0.0,
                                           1.0, 0.0, 0.0,
                                           pan_deg, tilt_deg);
    std::cout << "Target 1.0 North -> Pan calculated: " << pan_deg << "\n";
    assert(std::fabs(pan_deg - 0.0f) < 0.5f);

    ace::geometry::calculate_target_angles(0.0, 0.0, 0.0,
                                           0.0, 1.0, 0.0,
                                           pan_deg, tilt_deg);
    std::cout << "Target 1.0 East  -> Pan calculated: " << pan_deg << "\n";
    assert(std::fabs(pan_deg - 90.0f) < 0.5f);

    ace::geometry::calculate_target_angles(0.0, 0.0, 0.0,
                                           0.0, 0.0, 100.0,
                                           pan_deg, tilt_deg);
    std::cout << "Target above -> Tilt calculated: " << tilt_deg << "\n";
    assert(tilt_deg > 0.0f);

    std::cout << "test_geometric_projection passed.\n";
}

int main()
{
    std::cout << "Running Geometry Tests...\n";
    test_geometric_projection();
    std::cout << "All Geometry Tests passed!\n";
    return 0;
}
