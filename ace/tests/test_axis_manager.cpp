#include "../axis/axis_manager.hpp"
#include "../config/persistent_storage_interface.hpp"
#include <iostream>
#include <cassert>

namespace ace::config {
    // Mock storage for testing
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

int main() {
    std::cout << "Running AxisManager Tests...\n";
    test_axis_state_machine();
    std::cout << "All AxisManager Tests passed!\n";
    return 0;
}
