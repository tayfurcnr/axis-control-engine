#include "ace/config/persistent_storage.hpp"

#include <cstddef>

#include "esp_err.h"
#include "nvs_flash.h"

namespace ace::config {

namespace {

constexpr const char* kNamespace = "ace";
constexpr const char* kConfigKey = "persistent_cfg";

void apply_default_config(PersistentConfig& config)
{
    config = PersistentConfig {};
}

}  // namespace

// SUMMARY: DONE: Kalıcı konfigürasyon bloğunu ESP NVS belleğinden okur; versiyonlama uyumsuzluğundan korur.
StorageResult PersistentStorage::load(PersistentConfig& config) const
{
    nvs_handle_t handle {};
    const esp_err_t open_result = nvs_open(kNamespace, NVS_READONLY, &handle);
    if (open_result != 0) {
        apply_default_config(config);
        return StorageResult::not_found;
    }

    std::size_t blob_size = sizeof(PersistentConfig);
    const esp_err_t get_result = nvs_get_blob(handle, kConfigKey, &config, &blob_size);
    const std::uint32_t loaded_version = config.version;
    nvs_close(handle);

    if (get_result != 0 || blob_size != sizeof(PersistentConfig)) {
        apply_default_config(config);
        return StorageResult::read_failed;
    }

    if (loaded_version != PersistentConfig::kVersion) {
        apply_default_config(config);
        return StorageResult::version_mismatch;
    }

    return StorageResult::ok;
}

// SUMMARY: DONE: Kalıcı konfigürayon bloğunu ESP NVS belleğine serialize ederek yazar.
StorageResult PersistentStorage::save(const PersistentConfig& config) const
{
    nvs_handle_t handle {};
    const esp_err_t open_result = nvs_open(kNamespace, NVS_READWRITE, &handle);
    if (open_result != 0) {
        return StorageResult::invalid_state;
    }

    const esp_err_t set_result = nvs_set_blob(handle, kConfigKey, &config, sizeof(PersistentConfig));
    if (set_result != 0) {
        nvs_close(handle);
        return StorageResult::write_failed;
    }

    const esp_err_t commit_result = nvs_commit(handle);
    nvs_close(handle);
    return commit_result == 0 ? StorageResult::ok : StorageResult::write_failed;
}

// SUMMARY: DONE: NVS bloğundaki konfigürasyon anahtarını tamamen siler ve varsayılana dönmesini zorlar.
StorageResult PersistentStorage::erase() const
{
    nvs_handle_t handle {};
    const esp_err_t open_result = nvs_open(kNamespace, NVS_READWRITE, &handle);
    if (open_result != 0) {
        return StorageResult::invalid_state;
    }

    const esp_err_t erase_result = nvs_erase_key(handle, kConfigKey);
    if (erase_result != 0) {
        nvs_close(handle);
        return StorageResult::erase_failed;
    }

    const esp_err_t commit_result = nvs_commit(handle);
    nvs_close(handle);
    return commit_result == 0 ? StorageResult::ok : StorageResult::erase_failed;
}

}  // namespace ace::config
