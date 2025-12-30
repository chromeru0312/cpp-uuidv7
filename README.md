# cpp-uuidv7: C++17 UUIDv7 Library

**cpp-uuidv7** is a C++17 library for generating and parsing RFC 9562 UUIDv7 identifiers. UUIDv7 is a time-based UUID with millisecond precision, suitable for applications requiring sortable and unique identifiers. It features a simple, thread-safe, and cross-platform design.

## Features

  * Generation and parsing of UUIDv7
  * Thread-safe `uuidv7_generator` for concurrent generation
  * Monotonically increasing ID generation within the same millisecond
  * Cross-platform CSPRNG utilization (OpenSSL, Windows BCrypt, Unix `getrandom`, BSD/macOS `arc4random_buf`)
  * Easy conversion to strings and byte arrays

## Requirements

  * C++17 compatible compiler
  * CMake (3.22 or higher)
  * (Optional) OpenSSL (if using OpenSSL as the random number source)

## Building the Library

Build using CMake.

1.  Clone the repository: `git clone <repository-url> && cd cpp-uuidv7`
2.  Create and navigate to the build directory: `mkdir build && cd build`
3.  Configure CMake: `cmake .. [options]`
      * `-DUUIDV7LIB_TEST=ON`: Build tests (automatically fetches GoogleTest)
      * `-DUUIDV7LIB_DOCS=ON`: Build documentation (requires Doxygen)
      * `-DUUIDV7LIB_FORCE_NATIVE=ON`: Force the use of native CSPRNG
4.  Build: `cmake --build .`
5.  (Optional) Run tests: `ctest` (if tests were built)
6.  (Optional) Install: `cmake --install . --prefix <location>` (defaults if not specified)

## How to Use the Library

### Generating a UUIDv7

```cpp
#include <uuidv7/uuidv7.hpp>
#include <string>

int main() {
    // Generate a UUIDv7
    uuidv7::uuidv7_generator generator;
    uuidv7::uuidv7 id1 = generator.generate();

    // Convert to string
    std::string id1_str = id1.to_string();

    // Convert to byte representation
    std::array<uint8_t, 16> id1_bytes = id1.get_bytes();

    return 0;
}
```

### Parsing a UUIDv7 String

```cpp
#include <uuidv7/uuidv7.hpp>
#include <iostream>
#include <string>
#include <stdexcept>

int main() {
    std::string uuid_str = "018f7a6a-a1f1-72de-8000-000000000001";

    try {
        uuidv7::uuidv7 parsed_id = uuidv7::uuidv7::parse(uuid_str);
    } catch (const std::invalid_argument& e) {
        std::cerr << "Parse Error: " << e.what() << std::endl;
    } catch (const std::runtime_error& e) {
        std::cerr << "Runtime Error: " << e.what() << std::endl;
    }

    return 0;
}
```

## License

Conforms to the MIT License.