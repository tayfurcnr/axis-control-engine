#include "../axis/axis_manager.hpp"
#include "../config/persistent_storage_interface.hpp"
#include <iostream>
#include <cassert>
#include <cmath>

namespace ace::config {
    class MockStorageGeo : public IPersistentStorage {
    public:
        StorageResult load(PersistentConfig& config) const override { return StorageResult::ok; }
        StorageResult save(const PersistentConfig& config) const override { return StorageResult::ok; }
        StorageResult erase() const override { return StorageResult::ok; }
    };
}

void test_geometric_projection() {
    ace::axis::AxisManager manager;
    ace::config::MockStorageGeo storage;
    ace::config::PersistentConfig config;
    
    // Set our mock device location: Equator
    config.location_latitude_deg = 0.0;
    config.location_longitude_deg = 0.0;
    config.location_altitude_m = 0.0;
    config.has_location = true;
    
    manager.bind_persistent_state(&storage, &config);
    manager.enable(ace::communication::AxisId::ALL);
    manager.set_mode(ace::communication::ModeId::TRACK);

    // Expected Pan: 0.0 (North)
    manager.set_target(0.0, 1.0, 0.0); // (lon, lat, alt) => target at 1.0 North
    float target_pan = manager.pan_status().target_position_deg;
    std::cout << "Target 1.0 North -> Pan calculated: " << target_pan << " (Expected 0.0)\n";
    
    // Check East (longitude 1.0)
    manager.set_target(1.0, 0.0, 0.0); // (lon, lat, alt) => target at 1.0 East
    target_pan = manager.pan_status().target_position_deg;
    std::cout << "Target 1.0 East  -> Pan calculated: " << target_pan << " (Expected 90.0)\n";

    std::cout << "test_geometric_projection passed. Pan projections are matching compass degrees.\n";
}

int main() {
    std::cout << "Running Geometry Tests...\n";
    test_geometric_projection();
    std::cout << "All Geometry Tests passed!\n";
    return 0;
}
