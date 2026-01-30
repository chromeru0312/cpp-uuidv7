#include <array>
#include <cstdint>
#include <cstdlib>
#include "uuidv7/generator.hpp"

std::array<std::uint8_t, 10> uuidv7::uuidv7_generator::generate_random() {
    std::array<std::uint8_t, 10> buffer;
    arc4random_buf(buffer.data(), buffer.size());
    return buffer;
};
