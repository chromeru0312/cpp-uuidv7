#pragma once

#include <array>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <optional>
#include <ostream>
#include <stdexcept>
#include <string>
#include <string_view>
#include "uuidv7lib_export.h"

#if __cpp_lib_constexpr_algorithms >= 201806L
    #include <algorithm>
#else
    #define NO_CONSTEXPR_ALGO
#endif

#if __cpp_lib_constexpr_string >= 201907L
    #define CONSTEXPR_STRING constexpr
#else
    #define CONSTEXPR_STRING inline
#endif

namespace uuidv7 {

class uuidv7_generator;

/// @brief Error class representing an invalid format error when parsing `uuidv7`
class UUIDV7LIB_EXPORT invalid_format_error : public std::invalid_argument {
public:
    /// @brief Create a new invalid_format_error object
    /// @param message Error message
    invalid_format_error(const std::string& message) : std::invalid_argument(message) {}
};

/// @brief Struct representing the value of a UUID Version 7
///
/// UUID Version 7 is timestamp-based and has temporal ordering.
/// For details, see RFC 9562 Section 5.7.
///
/// @sa uuidv7_generator Class for generating UUIDs
struct UUIDV7LIB_EXPORT uuidv7 {
public:
    /// @brief Maximum value of rand_a
    static constexpr std::uint16_t MAX_RAND_A = 0x0FFF;
    /// @brief Maximum value of rand_b
    static constexpr std::uint64_t MAX_RAND_B = 0x3FFFFFFFFFFFFFFF;

    /// @brief Version
    static constexpr std::uint8_t VERSION = 7; // 0b0111
    /// @brief Variant
    static constexpr std::uint8_t VARIANT = 0b10;

    /// @brief Parse `uuidv7` from string
    /// @param str UUID Version 7 string
    /// @return `uuidv7` object
    /// @throw invalid_format_error if the string does not conform to UUID Version 7 format
    static constexpr uuidv7 parse(std::string_view const& str);

    /// @brief Try to parse `uuidv7` from string
    /// @param str UUID Version 7 string
    /// @return std::optional<uuidv7> (`std::nullopt` on failure)
    static constexpr std::optional<uuidv7> try_parse(std::string_view const& str) noexcept;

    /// @brief Create `uuidv7` from a 16-byte array
    /// @param bytes 16-byte array
    /// @return `uuidv7` object
    /// @throw invalid_format_error if the byte array does not conform to UUID Version 7 format
    static constexpr uuidv7 from_bytes(const std::array<uint8_t, 16>& bytes);

    /// @brief Create `uuidv7` from a 16-byte array pointer
    /// @param bytes Pointer to 16-byte array
    /// @return `uuidv7` object
    /// @throw invalid_format_error if the byte array does not conform to UUID Version 7 format
    static constexpr uuidv7 from_bytes(const uint8_t* bytes);

    /// @brief Get the 16-byte array representation of the `uuidv7`
    /// @return 16-byte array representing the `uuidv7`
    constexpr std::array<uint8_t, 16> get_bytes() const noexcept { return data_; }

    /// @brief Convert `uuidv7` to its standard string representation
    /// @param include_hyphens Whether to include hyphens in the string representation (default: `true`)
    /// @return `uuidv7` string representation
    /// @note In C++17, this function is not constexpr due to `std::string` limitations.
    CONSTEXPR_STRING std::string to_string(bool include_hyphens = true) const;

    /// @brief Get hash value for `uuidv7`
    /// @return Hash value
    constexpr size_t get_hash() const noexcept;

private:
    std::array<uint8_t, 16> data_;

    constexpr uuidv7(std::array<uint8_t, 16> bytes) : data_(bytes) { }
    constexpr uuidv7(std::uint64_t unix_ts_ms, std::uint16_t rand_a, std::uint64_t rand_b);

    enum class ParseResult : std::uint8_t {
        Success = 0,
        InvalidVersion,
        InvalidVariant,
        InvalidFormat,
    };
    static constexpr ParseResult parse_inner(std::string_view const& str, std::array<std::uint8_t, 16>& result);

    friend bool operator==(const uuidv7& lhs, const uuidv7& rhs);
    friend bool operator<(const uuidv7& lhs, const uuidv7& rhs);

    /// @brief Generator class
    friend class uuidv7_generator;
};


// --- Operators ---
/// @brief Equality operator for `uuidv7`
inline bool operator==(const uuidv7& lhs, const uuidv7& rhs) {
    return lhs.data_ == rhs.data_;
}
/// @brief Inequality operator for `uuidv7`
inline bool operator!=(const uuidv7& lhs, const uuidv7& rhs) { return !(lhs == rhs); }

/// @brief Less-than operator for `uuidv7`
inline bool operator<(const uuidv7& lhs, const uuidv7& rhs) {
    return lhs.data_ < rhs.data_;
}
/// @brief Greater-than operator for `uuidv7`
inline bool operator>(const uuidv7& lhs, const uuidv7& rhs) { return rhs < lhs; }
/// @brief Less-than-or-equal-to operator for `uuidv7`
inline bool operator<=(const uuidv7& lhs, const uuidv7& rhs) { return !(rhs < lhs); }
/// @brief Greater-than-or-equal-to operator for `uuidv7`
inline bool operator>=(const uuidv7& lhs, const uuidv7& rhs) { return !(lhs < rhs); }

/// @brief Output stream operator for `uuidv7`
inline std::ostream& operator<<(std::ostream& os, const uuidv7& uuid) {
    return os << uuid.to_string();
}


/// @cond Doxygen_suppress
// --- uuidv7 Implementation ---
constexpr uuidv7::uuidv7(std::uint64_t unix_ts_ms, std::uint16_t rand_a, std::uint64_t rand_b) : data_()
{
    assert(rand_a <= MAX_RAND_A);
    assert(rand_b <= MAX_RAND_B);

    // unix_ts_ms (48bit)
    data_[0] = (unix_ts_ms >> 40) & 0xFF;
    data_[1] = (unix_ts_ms >> 32) & 0xFF;
    data_[2] = (unix_ts_ms >> 24) & 0xFF;
    data_[3] = (unix_ts_ms >> 16) & 0xFF;
    data_[4] = (unix_ts_ms >> 8) & 0xFF;
    data_[5] = unix_ts_ms & 0xFF;

    // ver (4bit) + rand_a (12bit)
    data_[6] = ((rand_a >> 8) & (0xFF >> 4)) | (VERSION << 4);
    data_[7] = rand_a & 0xFF;

    // var (2bit) + rand_b (62bit)
    data_[8] = ((rand_b >> 56) & (0xFF >> 2)) | (VARIANT << 6);
    data_[9] = (rand_b >> 48) & 0xFF;
    data_[10] = (rand_b >> 40) & 0xFF;
    data_[11] = (rand_b >> 32) & 0xFF;
    data_[12] = (rand_b >> 24) & 0xFF;
    data_[13] = (rand_b >> 16) & 0xFF;
    data_[14] = (rand_b >> 8) & 0xFF;
    data_[15] = rand_b & 0xFF;
}

constexpr uuidv7::ParseResult uuidv7::parse_inner(std::string_view const& str, std::array<std::uint8_t, 16>& result)
{
    bool include_hyphens = false;
    int version_index = -1, variant_index = -1;
    if (str.length() == 36) {
        if (str[8] != '-' || str[13] != '-' || str[18] != '-' || str[23] != '-')
            return ParseResult::InvalidFormat;
        include_hyphens = true;
        version_index = 14;
        variant_index = 19;
    } else if (str.length() == 32) {
        version_index = 12;
        variant_index = 16;
    } else return ParseResult::InvalidFormat;

    if (str[version_index] != '7')
        return ParseResult::InvalidVersion;

    auto hex_to_int = [](char c) -> std::uint8_t {
        if (c >= '0' && c <= '9') return static_cast<std::uint8_t>(c - '0');
        if (c >= 'a' && c <= 'f') return static_cast<std::uint8_t>(c - 'a' + 10);
        if (c >= 'A' && c <= 'F') return static_cast<std::uint8_t>(c - 'A' + 10);
        return 0xFF;
    };

    auto var_char = hex_to_int(str[variant_index]);
    if (var_char == 0xFF) return ParseResult::InvalidFormat;
    if (((var_char & 0b1100) >> 2) != VARIANT)
        return ParseResult::InvalidVariant;

    int c = 0;
    for (int i = 0; i < 16; i++) {
        if (include_hyphens && (c == 8 || c == 13 || c == 18 || c == 23)) c++;

        std::uint8_t h1 = hex_to_int(str[c]), h2 = hex_to_int(str[c + 1]);
        if (h1 == 0xFF || h2 == 0xFF) return ParseResult::InvalidFormat;

        result[i] = h1 << 4 | h2;
        c += 2;
    }
    return ParseResult::Success;
}
constexpr uuidv7 uuidv7::parse(std::string_view const& str) {
    uuidv7 result {0, 0, 0};
    switch (parse_inner(str, result.data_)) {
        case ParseResult::Success:
            return result;
        case ParseResult::InvalidVersion:
            throw invalid_format_error("Invalid UUID Version 7 version");
        case ParseResult::InvalidVariant:
            throw invalid_format_error("Invalid UUID Version 7 variant");
        case ParseResult::InvalidFormat:
            throw invalid_format_error("Invalid UUID Version 7 string format");
        default:
            throw invalid_format_error("Unknown UUID Version 7 parse error");
    }
}
constexpr std::optional<uuidv7> uuidv7::try_parse(std::string_view const& str) noexcept {
    uuidv7 result {0, 0, 0};
    if (parse_inner(str, result.data_) == ParseResult::Success)
        return result;
    return std::nullopt;
}

constexpr uuidv7 uuidv7::from_bytes(std::array<uint8_t, 16> const& bytes) {
    if (((bytes[6] >> 4) & 0b1111) != VERSION)
        throw invalid_format_error("Invalid UUID Version 7 version");
    if (((bytes[8] >> 6) & 0b11) != VARIANT)
        throw invalid_format_error("Invalid UUID Version 7 variant");

    return uuidv7(bytes);
}

constexpr uuidv7 uuidv7::from_bytes(const uint8_t* bytes) {
    if (!bytes) throw invalid_format_error("Input pointer is null");
    std::array<uint8_t, 16> ary = {};

#ifdef NO_CONSTEXPR_ALGO
    for (int i = 0; i < 16; i++) { ary[i] = bytes[i]; }
#else
    std::ranges::copy(bytes, bytes + 16, ary.begin());
#endif

    return from_bytes(ary);
}

CONSTEXPR_STRING std::string uuidv7::to_string(bool include_hyphens) const {
    constexpr char HEX_CHARS[] = "0123456789abcdef";
    std::string result(include_hyphens ? 36 : 32, '\0');
    std::array<uint8_t, 16> bytes = get_bytes();

    int c = 0;
    for (int i = 0; i < 16; i++) {
        if (include_hyphens && (i == 4 || i == 6 || i == 8 || i == 10))
            result[c++] = '-';

        const uint8_t byte = bytes[i];
        result[c] = HEX_CHARS[(byte >> 4) & 0x0F];
        result[c + 1] = HEX_CHARS[byte & 0x0F];
        c += 2;
    }
    return result;
}

constexpr size_t uuidv7::get_hash() const noexcept {
    constexpr int Size = sizeof(size_t);
    size_t hash = 0;
    for (int i = 0; i < 16 / Size; i++) {
        size_t value = 0;
        for (int j = 0; j < Size; j++) {
            value = (value << 8) | data_[i * Size + j];
        }
        hash ^= value;
    }
    return hash;
}
/// @endcond

} // namespace uuidv7

/// @cond Doxygen_suppress
namespace std {
    template <>
    struct hash<uuidv7::uuidv7> {
        hash() = default;
        hash(const hash&) = default;

        size_t operator()(const uuidv7::uuidv7& u) const noexcept {
            return u.get_hash();
        }
    };
} // namespace std
/// @endcond
