#include "../axis/axis_manager.hpp"
#include "../config/persistent_storage_interface.hpp"
#include <iostream>
#include <cassert>

namespace ace::config {
    class MockStorage : public IPersistentStorage {
    public:
        StorageResult load(PersistentConfig& config) const override { return StorageResult::ok; }
        StorageResult save(const PersistentConfig& config) const override { return StorageResult::ok; }
        StorageResult erase() const override { return StorageResult::ok; }
    };
}

void test_axis_state_machine() {
    ace::axis::AxisManager manager;
    ace::config::MockStorage storage;
    ace::config::PersistentConfig config;
    
    manager.bind_persistent_state(&storage, &config);
    
    // Initial state
    assert(manager.pan_status().state == ace::axis::AxisState::boot);
    
    // Enable
    manager.enable(ace::communication::AxisId::ALL);
    assert(manager.pan_status().state == ace::axis::AxisState::ready);
    
    // Set Target
    manager.set_angle(ace::communication::AxisId::PAN, 45.0f, 0.0f);
    assert(manager.pan_status().state == ace::axis::AxisState::position);
    
    // Stop
    manager.stop(ace::communication::StopTypeId::SOFT);
    assert(manager.pan_status().state == ace::axis::AxisState::ready);
    
    std::cout << "test_axis_state_machine passed.\n";
}

void test_sensor_fusion_boot_homing() {
    ace::axis::AxisManager manager;
    ace::config::MockStorage storage;
    ace::config::PersistentConfig config;
    manager.bind_persistent_state(&storage, &config);

    // Simulate 9-DOF IMU reading at boot: Compass says 90 deg Pan, Accel says 5 deg Tilt
    ace::axis::SensorData boot_sensors;
    boot_sensors.compass_heading_deg = 90.0f;
    boot_sensors.accel_tilt_deg      = 5.0f;
    boot_sensors.pan_rate_dps        = 0.0f;
    boot_sensors.tilt_rate_dps       = 0.0f;

    manager.enable(ace::communication::AxisId::ALL);

    // After first update, system should be seeded from IMU
    manager.update(0.01f, boot_sensors);

    // Current position should now match compass/accel seed (within fusion tolerance)
    assert(manager.pan_status().current_position_deg > 89.0f && manager.pan_status().current_position_deg < 91.0f);
    assert(manager.tilt_status().current_position_deg > 4.0f && manager.tilt_status().current_position_deg < 6.0f);

    std::cout << "test_sensor_fusion_boot_homing passed.\n";
}

int main() {
    std::cout << "Running AxisManager Tests...\n";
    test_axis_state_machine();
    test_sensor_fusion_boot_homing();
    std::cout << "All AxisManager Tests passed!\n";
    return 0;
}
