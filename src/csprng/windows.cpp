#include <array>
#include <cstdint>
#include <system_error>
#include <windows.h>
#include <bcrypt.h>
#include "uuidv7/generator.hpp"

#ifndef BCRYPT_USE_SYSTEM_PREFERRED_RNG
    #define BCRYPT_USE_SYSTEM_PREFERRED_RNG 2
#endif

std::array<std::uint8_t, 10> uuidv7::uuidv7_generator::generate_random() {
    std::array<std::uint8_t, 10> buffer;
    NTSTATUS status = BCryptGenRandom(NULL, reinterpret_cast<PUCHAR>(buffer.data()), buffer.size(), BCRYPT_USE_SYSTEM_PREFERRED_RNG);
    if (!BCRYPT_SUCCESS(status)) {
        throw std::system_error(status, std::system_category(), "BCryptGenRandom failed to generate random bytes.");
    }
    return buffer;
};
