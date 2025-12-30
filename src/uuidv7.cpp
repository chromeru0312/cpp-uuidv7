#include "uuidv7/uuidv7.hpp"
#include <charconv>
#include <chrono>
#include <cstring>
#include <stdexcept>
#include <iomanip>
#include <sstream>

namespace uuidv7 {

// uuid7
uuidv7::uuidv7(std::uint64_t ts_ms, std::uint16_t r_a, std::uint64_t r_b)
    : unix_ts_ms(ts_ms), rand_a(r_a), rand_b(r_b)
{
    if (rand_a > MAX_RAND_A) {
        throw std::invalid_argument("rand_a exceeds maximum value");
    }
    if (rand_b > MAX_RAND_B) {
         throw std::invalid_argument("rand_b exceeds maximum value");
    }

#ifndef NDEBUG
    update_debug_variables();
#endif
}

uuidv7 uuidv7::parse(std::string const str) {
    if (str.length() != 36)
        throw parse_error("Invalid UUIDv7 string length");
    if (str[8] != '-' || str[13] != '-' || str[18] != '-' || str[23] != '-')
        throw parse_error("Invalid UUIDv7 string format");
    if (str[14] != '7')
        throw parse_error("Invalid UUIDv7 version");

    auto get_hex = [str](int start, int count) {
        std::uint64_t buf;
        auto res = std::from_chars(str.data() + start, str.data() + start + count, buf, 16);
        if (res.ec != std::errc()) throw parse_error("Invalid UUIDv7 format");
        return buf;
    };
    const std::uint64_t hex19 = get_hex(19, 1);
    if ((hex19 & 0b1100) != 0b1000)
        throw parse_error("Invalid UUIDv7 variant");

    std::uint64_t unix_ts_ms = (get_hex(0, 8) << 16) | get_hex(9, 4);
    std::uint16_t rand_a = static_cast<std::uint16_t>(get_hex(15, 3));
    std::uint64_t rand_b = ((hex19 & 0b0011) << 60) | (get_hex(20, 3) << 48) | (get_hex(24, 12));
    return uuidv7(unix_ts_ms, rand_a, rand_b);
}

uuidv7 uuidv7::from_bytes(const std::array<uint8_t, 16>& bytes) {
    if (((bytes[6] & 0xF0) >> 4) != VERSION)
        throw parse_error("Invalid UUIDv7 version");
    if (((bytes[8] & 0xC0) >> 6) != VARIANT)
        throw parse_error("Invalid UUIDv7 variant");

    std::uint64_t unix_ts_ms = 0;
    for (int i = 0; i < 6; ++i) {
        unix_ts_ms = (unix_ts_ms << 8) | bytes[i];
    }
    std::uint16_t rand_a = ((bytes[6] & 0x0F) << 8) | bytes[7];
    std::uint64_t rand_b = 0;
    for (int i = 8; i < 16; ++i) {
        rand_b = (rand_b << 8) | bytes[i];
    }
    rand_b &= MAX_RAND_B;
    return uuidv7(unix_ts_ms, rand_a, rand_b);
}

uuidv7 uuidv7::from_bytes(const uint8_t* bytes) {
    if (!bytes)
        throw parse_error("Input pointer is null");

    std::array<uint8_t, 16> array_bytes;
    std::memcpy(array_bytes.data(), bytes, 16);

    return from_bytes(array_bytes);
}

std::array<uint8_t, 16> uuidv7::get_bytes() const noexcept {
    std::array<uint8_t, 16> bytes;

    // unix_ts_ms (48bit)
    bytes[0] = (unix_ts_ms >> 40) & 0xFF;
    bytes[1] = (unix_ts_ms >> 32) & 0xFF;
    bytes[2] = (unix_ts_ms >> 24) & 0xFF;
    bytes[3] = (unix_ts_ms >> 16) & 0xFF;
    bytes[4] = (unix_ts_ms >> 8) & 0xFF;
    bytes[5] = unix_ts_ms & 0xFF;

    // ver (4bit) + rand_a (12bit)
    bytes[6] = ((rand_a >> 8) & (0xFF >> 4)) | (VERSION << 4);
    bytes[7] = rand_a & 0xFF;

    // var (2bit) + rand_b (62bit)
    bytes[8] = ((rand_b >> 56) & (0xFF >> 2)) | (VARIANT << 6);
    bytes[9] = (rand_b >> 48) & 0xFF;
    bytes[10] = (rand_b >> 40) & 0xFF;
    bytes[11] = (rand_b >> 32) & 0xFF;
    bytes[12] = (rand_b >> 24) & 0xFF;
    bytes[13] = (rand_b >> 16) & 0xFF;
    bytes[14] = (rand_b >> 8) & 0xFF;
    bytes[15] = rand_b & 0xFF;

    return bytes;
}

std::string uuidv7::to_string() const {
    std::string result;
    result.reserve(36);

    constexpr char hex_chars[] = "0123456789abcdef";
    std::array<uint8_t, 16> bytes = get_bytes();

    for (int i = 0; i < 16; ++i) {
        const uint8_t byte = bytes[i];
        result += hex_chars[(byte >> 4) & 0x0F];
        result += hex_chars[byte & 0x0F];

        if (i == 3 || i == 5 || i == 7 || i == 9) {
            result += '-';
        }
    }
    return result;
}

#ifndef NDEBUG
void uuidv7::update_debug_variables() const noexcept {
    // uuid_string
    constexpr char hex_chars[] = "0123456789abcdef";
    const std::array<uint8_t, 16> bytes = get_bytes();
    char* p = uuid_string;

    for (int i = 0; i < 16; ++i) {
        const uint8_t byte = bytes[i];
        *p++ = hex_chars[(byte >> 4) & 0x0F];
        *p++ = hex_chars[byte & 0x0F];

        if (i == 3 || i == 5 || i == 7 || i == 9) {
            *p++ = '-';
        }
    }
    *p = '\0';

    // timestamp_string
    auto time_point = std::chrono::system_clock::time_point(std::chrono::milliseconds(unix_ts_ms));
    auto time_t_val = std::chrono::system_clock::to_time_t(time_point);
    std::tm time_info{};

#if defined(_WIN32)
    gmtime_s(&time_info, &time_t_val);
#else
    gmtime_r(&time_t_val, &time_info);
#endif

    size_t written = strftime(timestamp_string, 25, "%Y-%m-%dT%H:%M:%S", &time_info);
    if (written > 0 && (25 - written) > 5) {
        snprintf(timestamp_string + written, 25 - written, ".%03dZ", static_cast<int>(unix_ts_ms % 1000));
    } else if (written == 0) {
        snprintf(timestamp_string, 25, "[strftime Error]");
    }
}
#endif


// uuidv7_generator
uuidv7 uuidv7_generator::generate() {
    std::lock_guard<std::mutex> lock(mutex_);

    auto now_duration = std::chrono::system_clock::now().time_since_epoch();
    uint64_t millis = static_cast<uint64_t>(std::chrono::duration_cast<std::chrono::milliseconds>(now_duration).count());

    if (millis > last_generated_.unix_ts_ms) {
        std::uint16_t a;
        std::uint64_t b;
        generate_random(a, b);

        last_generated_.unix_ts_ms = millis;
        last_generated_.rand_a = a & uuidv7::MAX_RAND_A;
        last_generated_.rand_b = b & uuidv7::MAX_RAND_B;
    } else {
        if (millis < last_generated_.unix_ts_ms) {
            millis = last_generated_.unix_ts_ms;
        }

        if (last_generated_.rand_b < uuidv7::MAX_RAND_B) {
            last_generated_.rand_b++;
        } else if (last_generated_.rand_a < uuidv7::MAX_RAND_A) {
            last_generated_.rand_b = 0;
            last_generated_.rand_a++;
        } else {
            throw sequence_overflow_error("Too many UUIDs generated in the same millisecond; sequence counter overflowed.");
        }
    }

#ifndef NDEBUG
    last_generated_.update_debug_variables();
#endif
    return last_generated_;
}

} // namespace uuidv7