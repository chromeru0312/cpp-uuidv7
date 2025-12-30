#include <sys/random.h>
#include <errno.h>
#include "uuidv7/uuidv7.hpp"

namespace uuidv7 {

template <typename T>
inline void generate_random_bytes(T& buffer) {
    unsigned char* ptr = reinterpret_cast<unsigned char*>(&buffer);
    size_t bytes_to_read = sizeof(buffer), bytes_read_total = 0;

    while (bytes_read_total < bytes_to_read) {
        ssize_t bytes_read_current = getrandom(ptr + bytes_read_total, bytes_to_read - bytes_read_total, 0);
        if (bytes_read_current < 0) {
            if (errno == EINTR) {
                continue;
            } else {
                throw std::system_error(errno, std::generic_category(), "getrandom failed to generate random bytes");
            }
        }
        if (bytes_read_current == 0 && bytes_to_read > 0) {
            throw std::runtime_error("getrandom returned 0 bytes without error, failed to fill buffer.");
        }
        bytes_read_total += bytes_read_current;
    }
}

void uuidv7_generator::generate_random(std::uint16_t& a, std::uint64_t& b) {
    generate_random_bytes(a);
    generate_random_bytes(b);
};

} // namespace uuidv7