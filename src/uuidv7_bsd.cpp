#include <stdlib.h>
#include "uuidv7/uuidv7.hpp"

namespace uuidv7 {

void uuidv7_generator::generate_random(std::uint16_t& a, std::uint64_t& b) {
    arc4random_buf(a, sizeof(a));
    arc4random_buf(b, sizeof(b));
};

} // namespace uuidv7