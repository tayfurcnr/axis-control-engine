#pragma once

#include "ace/config/persistent_storage_interface.hpp"

namespace ace::config {

class PersistentStorage final : public IPersistentStorage {
public:
    PersistentStorage() = default;

    [[nodiscard]] StorageResult load(PersistentConfig& config) const override;
    [[nodiscard]] StorageResult save(const PersistentConfig& config) const override;
    [[nodiscard]] StorageResult erase() const override;
};

}  // namespace ace::config
