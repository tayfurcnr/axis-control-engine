#pragma once

#include <cstdint>

#include "ace/config/config.hpp"

namespace ace::config {

enum class StorageResult : std::uint8_t {
    ok = 0,
    not_found,
    invalid_state,
    read_failed,
    write_failed,
    erase_failed,
    version_mismatch,
};

class IPersistentStorage {
public:
    virtual ~IPersistentStorage() = default;

    [[nodiscard]] virtual StorageResult load(PersistentConfig& config) const = 0;
    [[nodiscard]] virtual StorageResult save(const PersistentConfig& config) const = 0;
    [[nodiscard]] virtual StorageResult erase() const = 0;
};

}  // namespace ace::config
