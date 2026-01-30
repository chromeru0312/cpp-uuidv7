cmake_minimum_required(VERSION 3.22)

project(uuidv7lib
    VERSION 1.0.0
    LANGUAGES CXX
)

include(CheckSymbolExists)
include(CMakePackageConfigHelpers)
include(GenerateExportHeader)
include(GNUInstallDirs)

if(NOT DEFINED CMAKE_CXX_STANDARD)
    set(CMAKE_CXX_STANDARD 20)
endif()

set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
add_compile_options($<$<CXX_COMPILER_ID:MSVC>:/utf-8>)

# --- Build Library ---
option(UUIDV7LIB_FORCE_NATIVE "Force native build" OFF)

add_library(uuidv7lib
    "${CMAKE_CURRENT_SOURCE_DIR}/include/uuidv7/uuidv7.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/uuidv7/generator.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/generator.cpp"
)
add_library(uuidv7::uuidv7 ALIAS uuidv7lib)

generate_export_header(uuidv7lib)
target_include_directories(uuidv7lib PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)
target_compile_features(uuidv7lib PUBLIC cxx_std_17)

# --- CS-PRNG Backend ---
set(UUIDV7_USE_OPENSSL OFF)
find_package(OpenSSL QUIET)

if (NOT UUIDV7LIB_FORCE_NATIVE AND OpenSSL_FOUND)
    set(UUIDV7_USE_OPENSSL ON)
    target_sources(uuidv7lib PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src/csprng/openssl.cpp)
    target_link_libraries(uuidv7lib PRIVATE OpenSSL::Crypto)
elseif (WIN32)
    target_sources(uuidv7lib PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src/csprng/windows.cpp)
    target_link_libraries(uuidv7lib PRIVATE bcrypt)
else()
    check_symbol_exists(arc4random "stdlib.h" HAVE_ARC4RANDOM)
    check_symbol_exists(getrandom "sys/random.h" HAVE_GETRANDOM)

    if(HAVE_ARC4RANDOM)
        target_sources(uuidv7lib PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src/csprng/bsd.cpp)
    elseif(HAVE_GETRANDOM)
        target_sources(uuidv7lib PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src/csprng/unix.cpp)
    else()
        message(FATAL_ERROR "Unsupported platform. Please use external CS-PRNG library such as OpenSSL.")
    endif()
endif()

# --- Debugger Visualizer ---
set(NATVIS_FILE "${CMAKE_CURRENT_SOURCE_DIR}/uuidv7.natvis")
target_sources(uuidv7lib PUBLIC "$<BUILD_INTERFACE:${NATVIS_FILE}>")

# --- Source Groups ---
get_target_property(ALL_SOURCE_FILES uuidv7lib SOURCES)
source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}" FILES
    "${ALL_SOURCE_FILES}"
)


# --- Testing ---
option(UUIDV7LIB_BUILD_TEST "Build test" OFF)
if (UUIDV7LIB_BUILD_TEST)
    enable_testing()

    include(FetchContent)
    FetchContent_Declare(
        googletest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG        v1.17.0
    )
    set(BUILD_SHARED_LIBS OFF)
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(googletest)

    include(GoogleTest)
    add_subdirectory(test)
endif()


# --- Documentation ---
option(UUIDV7LIB_BUILD_DOCS "Build documentation" OFF)
if (UUIDV7LIB_BUILD_DOCS)
    find_package(Doxygen REQUIRED)

    include(FetchContent)
    FetchContent_Declare(
        doxygen-awesome-css
        URL https://github.com/jothepro/doxygen-awesome-css/archive/refs/heads/main.zip
        DOWNLOAD_EXTRACT_TIMESTAMP TRUE
    )
    FetchContent_MakeAvailable(doxygen-awesome-css)
    FetchContent_GetProperties(doxygen-awesome-css SOURCE_DIR AWESOME_CSS_DIR)

    set(DOXYGEN_GENERATE_HTML YES)
    set(DOXYGEN_EXTRACT_STATIC YES)
    set(DOXYGEN_PROJECT_NAME "cpp-uuidv7")
    set(DOXYGEN_PROJECT_BRIEF "C++ implementation of UUID Version 7 (RFC 9562)")
    set(DOXYGEN_HTML_EXTRA_STYLESHEET ${AWESOME_CSS_DIR}/doxygen-awesome.css)

    doxygen_add_docs(
        uuidv7lib_docs
        "${CMAKE_CURRENT_SOURCE_DIR}/include/uuidv7"
        COMMENT "Generate documentation"
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    )
endif()


# --- Install ---
set_target_properties(uuidv7lib PROPERTIES EXPORT_NAME uuidv7)
install(FILES
    "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/uuidv7/uuidv7.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/uuidv7/generator.hpp"
    "${CMAKE_CURRENT_BINARY_DIR}/uuidv7lib_export.h"
    DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/uuidv7"
    COMPONENT Devel
)
install(TARGETS uuidv7lib
    EXPORT uuidv7Targets
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR} COMPONENT Runtime
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR} COMPONENT Runtime
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR} COMPONENT Runtime
    INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)
install(EXPORT uuidv7Targets
    FILE uuidv7Targets.cmake
    NAMESPACE uuidv7::
    DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/uuidv7"
    COMPONENT Devel
)

configure_package_config_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake/uuidv7Config.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/uuidv7Config.cmake"
    INSTALL_DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/uuidv7"
)
write_basic_package_version_file(
    "${CMAKE_CURRENT_BINARY_DIR}/uuidv7ConfigVersion.cmake"
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY SameMajorVersion
)

install(FILES
    "${CMAKE_CURRENT_BINARY_DIR}/uuidv7Config.cmake"
    "${CMAKE_CURRENT_BINARY_DIR}/uuidv7ConfigVersion.cmake"
    DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/uuidv7"
    COMPONENT Devel
)
export(EXPORT uuidv7Targets
    FILE "${CMAKE_CURRENT_BINARY_DIR}/uuidv7Targets.cmake"
    NAMESPACE uuidv7::
)
