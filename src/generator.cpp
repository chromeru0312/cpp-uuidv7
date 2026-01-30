#include <chrono>
#include <cstring>
#include "uuidv7/generator.hpp"

namespace uuidv7 {

// uuidv7_generator
uuidv7 uuidv7_generator::generate() {
    std::lock_guard<std::mutex> lock(mutex_);

    auto now_duration = std::chrono::system_clock::now().time_since_epoch();
    auto millis = std::chrono::duration_cast<std::chrono::milliseconds>(now_duration).count();
    std::array<std::uint8_t, 6> millis_bytes = {};
    for (int i = 5; i >= 0; i--) {
        millis_bytes[i] = static_cast<std::uint8_t>(millis & 0xFF);
        millis >>= 8;
    }

    if (std::memcmp(last_generated_.data_.data(), millis_bytes.data(), 6) < 0) {
        auto rand = generate_random();
        std::memcpy(last_generated_.data_.data(), millis_bytes.data(), 6);
        std::memcpy(last_generated_.data_.data() + 6, rand.data(), 2);
        std::memcpy(last_generated_.data_.data() + 8, rand.data() + 2, 8);

        last_generated_.data_[6] = ((last_generated_.data_[6] >> 4) & (0xFF >> 4)) | (uuidv7::VERSION << 4);
        last_generated_.data_[8] = ((last_generated_.data_[8] >> 2) & (0xFF >> 2)) | (uuidv7::VARIANT << 6);
    } else {
        std::uint8_t target = 15;
        while (target > 8) {
            if (last_generated_.data_[target] < 0xFF) {
                last_generated_.data_[target]++;
                target = 0;
                break;
            } else {
                last_generated_.data_[target] = 0;
                target--;
            }
        }

        if (target > 0) {
            do {
                if ((last_generated_.data_[8] & 0x3F) < 0x3F) {
                    last_generated_.data_[8]++;
                    break;
                }
                last_generated_.data_[8] = 0x00 | (uuidv7::VARIANT << 6);

                if ((last_generated_.data_[7]) < 0xFF) {
                    last_generated_.data_[7]++;
                    break;
                }
                last_generated_.data_[7] = 0x00;

                if ((last_generated_.data_[6] & 0x0F) < 0x0F) {
                    last_generated_.data_[6]++;
                    break;
                }
                throw sequence_overflow_error("Too many UUIDs generated in the same millisecond; sequence counter overflowed.");
            }
            while(false);
        }
    }
    return last_generated_;
}

} // namespace uuidv7
