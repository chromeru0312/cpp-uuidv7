#pragma once

#include <array>
#include <cstdint>
#include <mutex>
#include "uuidv7.hpp"

namespace uuidv7 {

/// @brief Error class representing a sequence overflow error within the same millisecond for `uuidv7`
class UUIDV7LIB_EXPORT sequence_overflow_error : public std::runtime_error {
public:
    /// @brief Create a new sequence_overflow_error object
    /// @param message Error message
    sequence_overflow_error(const std::string& message) : std::runtime_error(message) {}
};

/// @brief `uuidv7` generator class
///
/// This class provides thread-safe generation of `uuidv7`.
/// If multiple UUIDs are generated within the same millisecond,
/// the random part is incremented to maintain monotonicity.
///
/// @note
/// This class maintains state (the last generated UUID).
/// Typically, a single instance is shared within an application thread or process.
class UUIDV7LIB_EXPORT uuidv7_generator {
public:
    /// @brief Default constructor
    uuidv7_generator() : last_generated_(0, 0, 0) {}

    /// @brief Constructor with initial last generated UUID
    /// @param last_uuid Initial last generated `uuidv7`
    uuidv7_generator(uuidv7 last_uuid) : last_generated_(last_uuid) {}

    /// @cond Doxygen_suppress
    // Move constructor and move assignment operator
    uuidv7_generator(uuidv7_generator&&) = default;
    uuidv7_generator& operator=(uuidv7_generator&&) = default;

    // Delete copy constructor and copy assignment operator
    uuidv7_generator(const uuidv7_generator&) = delete;
    uuidv7_generator& operator=(const uuidv7_generator&) = delete;
    /// @endcond

    /// @brief Get the default `uuidv7_generator` instance
    /// @return Reference to the default `uuidv7_generator` instance
    static uuidv7_generator& default_instance() {
        static uuidv7_generator instance;
        return instance;
    }

    /// @brief Generate a new `uuidv7` object with the current time using the default `uuidv7_generator` instance
    /// @return `uuidv7` object
    /// @throw std::system_error if random number generation fails (OS-specific CSPRNG call fails)
    /// @throw std::runtime_error if random number generation fails (CSPRNG library call fails)
    /// @throw sequence_overflow_error if the maximum number of UUIDs that can be generated in the same millisecond is exceeded
    static uuidv7 generate_default() { return default_instance().generate(); }

    /// @brief Generate a new `uuidv7` object with the current time
    /// @return `uuidv7` object
    /// @throw std::system_error if random number generation fails (OS-specific CSPRNG call fails)
    /// @throw std::runtime_error if random number generation fails (CSPRNG library call fails)
    /// @throw sequence_overflow_error if the maximum number of UUIDs that can be generated in the same millisecond is exceeded
    uuidv7 generate();

private:
    std::mutex mutex_;
    uuidv7 last_generated_{0, 0, 0};

    /// @brief Generate 10 random bytes with CSPRNG
    /// @return 10-byte array of random bytes
    std::array<std::uint8_t, 10> generate_random();
};

} // namespace uuidv7
