#include <openssl/rand.h>
#include <openssl/err.h>
#include "uuidv7/uuidv7.hpp"

namespace uuidv7 {

std::string get_openssl_error() {
    unsigned long err_code = ERR_get_error();
    if (err_code == 0) {
        return "No OpenSSL error reported, but RAND_bytes indicated failure.";
    }
    char err_buf[256];
    ERR_error_string_n(err_code, err_buf, sizeof(err_buf));
    return std::string(err_buf);
}

void uuidv7_generator::generate_random(std::uint16_t& a, std::uint64_t& b) {
    if (RAND_bytes(reinterpret_cast<unsigned char*>(&a), sizeof(a)) != 1) {
        throw std::runtime_error("RAND_bytes failed for rand_a: " + get_openssl_error());
    }
    if (RAND_bytes(reinterpret_cast<unsigned char*>(&b), sizeof(b)) != 1) {
        throw std::runtime_error("RAND_bytes failed for rand_b: " + get_openssl_error());
    }
};

} // namespace uuidv7