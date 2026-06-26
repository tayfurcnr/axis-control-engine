#include "ace/diagnostics/diagnostics.hpp"

#include <cstdio>

namespace ace::diagnostics {

// SUMMARY: TODO: Voltaj, sıcaklık, encoder bağlantısı ve sistem bellek sağlığını test eder; HAL entegrasyonu sonra dolacak.
bool Diagnostics::run_self_test() const
{
    std::puts("Diagnostics: run_self_test");
    return true;
}

}  // namespace ace::diagnostics
