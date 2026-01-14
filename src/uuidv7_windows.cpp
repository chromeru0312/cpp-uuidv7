#include <windows.h>
#include <bcrypt.h>
#include "uuidv7/uuidv7.hpp"

#pragma comment(lib, "bcrypt.lib")

namespace uuidv7 {

void uuidv7_generator::generate_random(std::uint16_t& a, std::uint64_t& b) {
    NTSTATUS status;
    status = BCryptGenRandom(NULL, reinterpret_cast<PUCHAR>(&a), sizeof(a), 2);
    if (!BCRYPT_SUCCESS(status)) {
        throw std::system_error(status, std::system_category(), "BCryptGenRandom failed to generate random bytes for rand_a.");
    }
    status = BCryptGenRandom(NULL, reinterpret_cast<PUCHAR>(&b), sizeof(b), 2);
    if (!BCRYPT_SUCCESS(status)) {
        throw std::system_error(status, std::system_category(), "BCryptGenRandom failed to generate random bytes for rand_b.");
    }
};

} // namespace uuidv7