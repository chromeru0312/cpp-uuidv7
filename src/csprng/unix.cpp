#include <array>
#include <cerrno>
#include <cstdint>
#include <system_error>
#include <sys/random.h>
#include "uuidv7/generator.hpp"

std::array<std::uint8_t, 10> uuidv7::uuidv7_generator::generate_random() {
    std::array<std::uint8_t, 10> buffer;
    size_t bytes_read_total = 0;

    while (bytes_read_total < 10) {
        ssize_t bytes_read_current = getrandom(buffer.data() + bytes_read_total, 10 - bytes_read_total, 0);
        if (bytes_read_current < 0) {
            if (errno == EINTR) {
                continue;
            } else {
                throw std::system_error(errno, std::generic_category(), "getrandom failed to generate random bytes");
            }
        }
        bytes_read_total += bytes_read_current;
    }
    return buffer;
};
