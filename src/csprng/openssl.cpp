#include <array>
#include <cstdint>
#include <stdexcept>
#include <string>
#include <openssl/rand.h>
#include <openssl/err.h>
#include "uuidv7/generator.hpp"

std::array<std::uint8_t, 10> uuidv7::uuidv7_generator::generate_random() {
    std::array<std::uint8_t, 10> buffer;
    if (RAND_bytes(reinterpret_cast<unsigned char*>(buffer.data()), buffer.size()) == 1)
        return buffer;

    unsigned long err_code = ERR_get_error();
    std::string error;
    if (err_code != 0) {
        char err_buf[256];
        ERR_error_string_n(err_code, err_buf, sizeof(err_buf));
        error = err_buf;
    } else {
        error = "RAND_bytes failed, but no OpenSSL error reported.";
    }
    throw std::runtime_error("RAND_bytes failed: " + error);
};
