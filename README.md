# cpp-uuidv7: C++17 UUIDv7 Library

cpp-uuidv7 is a header-only C++ library for generating and parsing UUID Version 7.

## Features

  * Generation and parsing of UUID Version 7 (Fully [RFC 9562](https://www.rfc-editor.org/info/rfc9562) compliant)
  * Easy conversion to strings and byte arrays
  * `constexpr` implementation for almost all functions in struct `uuidv7`
  * Thread-safe `uuidv7_generator` for concurrent UUID generation
  * Cross-platform CSPRNG support (OpenSSL, Windows BCrypt, Unix `getrandom`, BSD/macOS `arc4random_buf`)

## Requirements

  * C++17/C++20 compatible compiler
    * C++17 minimum (Enables core functionality)
    * C++20 recommended (Enables `constexpr` optimizations and `std::ranges` implementations)
  * CMake (3.22 or higher)
  * [Optional] OpenSSL
  * [Optional] Doxygen (for building documentation)

> [!NOTE]
> OpenSSL is only required if the native CSPRNG is not available on the target platform.

## Build

Build using CMake.

1. Clone the repository:
    ```bash
    git clone https://github.com/chromeru0312/cpp-uuidv7
    cd cpp-uuidv7
    ```

1. Run the following commands to build the library:
    ```bash
    mkdir build
    cd build
    cmake ..
    cmake --build .
   ```

1. (Optional) Install the library to a specified location:
    ```bash
    cmake --install . --prefix /desired/installation/path
    ```

### Build Options

The following CMake options can be set to customize the build:

| Option | Default | Description |
|--------|---------|-------------|
| `UUIDV7LIB_FORCE_NATIVE` | `OFF` | Force the use of native CSPRNG instead of OpenSSL. |
| `UUIDV7LIB_BUILD_TEST` | `OFF` | Build unit tests. |
| `UUIDV7LIB_BUILD_DOCS` | `OFF` | Build documentation. |

> [!TIP]
> To build shared libraries, add `-DBUILD_SHARED_LIBS=ON` to the CMake command line.

## Library Usage

### Generating a UUIDv7

```cpp
#include <uuidv7/uuidv7.hpp>
#include <uuidv7/generator.hpp>
#include <array>
#include <cstdint>
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
    } catch (const uuidv7::invalid_format_error& e) {
        std::cerr << "Parse Error: " << e.what() << std::endl;
    } catch (const std::runtime_error& e) {
        std::cerr << "Runtime Error: " << e.what() << std::endl;
    }

    return 0;
}
```

## License

[MIT License](LICENSE)
