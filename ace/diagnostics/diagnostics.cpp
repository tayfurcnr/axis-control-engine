#include "ace/diagnostics/diagnostics.hpp"

#include <cstddef>
#include <cstdio>

#include "esp_heap_caps.h"

namespace ace::diagnostics {

namespace {

constexpr std::size_t kMinFreeHeapBytes = 16u * 1024u;
constexpr std::size_t kMinLargestBlockBytes = 8u * 1024u;

}  // namespace

// SUMMARY: DONE: HAL bağımsız öz-test olarak bellek sağlığını kontrol eder; voltaj/sıcaklık/encoder testleri HAL entegrasyonuna kalır.
bool Diagnostics::run_self_test() const
{
    const std::size_t free_heap = heap_caps_get_free_size(MALLOC_CAP_8BIT);
    const std::size_t largest_block = heap_caps_get_largest_free_block(MALLOC_CAP_8BIT);

    std::printf("Diagnostics: free_heap=%u largest_block=%u\n",
                static_cast<unsigned>(free_heap),
                static_cast<unsigned>(largest_block));

    return free_heap >= kMinFreeHeapBytes && largest_block >= kMinLargestBlockBytes;
}

}  // namespace ace::diagnostics
