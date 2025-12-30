cmake_minimum_required(VERSION 3.25)
include(GNUInstallDirs)

project(uuidv7lib
    VERSION 1.0.0
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_CXX_EXTENSIONS OFF)

# --- Library ---
option(UUIDV7LIB_FORCE_NATIVE "Force native build" OFF)

add_library(uuidv7lib
    ${CMAKE_CURRENT_SOURCE_DIR}/src/uuidv7.cpp
)

find_package(OpenSSL QUIET)
if (NOT UUIDV7LIB_FORCE_NATIVE AND OpenSSL_FOUND)
    target_sources(uuidv7lib PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src/uuidv7_openssl.cpp)
    target_link_libraries(uuidv7lib PRIVATE OpenSSL::Crypto)
elseif (WIN32)
    target_sources(uuidv7lib PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src/uuidv7_windows.cpp)
    target_link_libraries(uuidv7lib PRIVATE bcrypt)
elseif (BSD OR ANDROID OR APPLE)
    target_sources(uuidv7lib PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src/uuidv7_bsd.cpp)
elseif (UNIX)
    target_sources(uuidv7lib PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src/uuidv7_unix.cpp)
else()
    message(FATAL_ERROR "Unsupported platform")
endif()

target_include_directories(uuidv7lib
PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)
target_compile_features(uuidv7lib PUBLIC cxx_std_17)
add_library(uuidv7::uuidv7 ALIAS uuidv7lib)

# --- Shared Library Support ---
if (BUILD_SHARED_LIBS)
    include(GenerateExportHeader)
    set(UUIDV7LIB_EXPORT_HEADER "${CMAKE_CURRENT_BINARY_DIR}/include/uuidv7/uuidv7_export.h")
    generate_export_header(uuidv7lib
        EXPORT_MACRO_NAME UUIDV7_API
        EXPORT_FILE_NAME ${UUIDV7LIB_EXPORT_HEADER}
    )

    target_compile_definitions(uuidv7lib PRIVATE UUIDV7LIB_SHARED)
    target_include_directories(uuidv7lib PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/include>
    )
endif()

# --- Debugger Visualizer ---
set(NATVIS_FILE "${CMAKE_CURRENT_SOURCE_DIR}/uuidv7.natvis")
target_sources(uuidv7lib PRIVATE ${NATVIS_FILE})
source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}" FILES "${CMAKE_CURRENT_SOURCE_DIR}/include/uuidv7.hpp")


# --- Testing ---
option(UUIDV7LIB_TEST "Build test" OFF)
if (UUIDV7LIB_TEST)
    enable_testing()

    include(FetchContent)
    FetchContent_Declare(
        googletest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG        v1.17.0
    )
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(googletest)

    include(GoogleTest)
    add_subdirectory(test)
endif()


# --- Documentation ---
option(UUIDV7LIB_DOCS "Build documentation" OFF)
if (UUIDV7LIB_DOCS)
    find_package(Doxygen REQUIRED)
    set(DOXYGEN_GENERATE_HTML YES)
    set(DOXYGEN_EXTRACT_ALL YES)
    set(DOXYGEN_EXTRACT_PRIVATE YES)
    set(DOXYGEN_EXTRACT_STATIC YES)
    set(DOXYGEN_EXTRACT_LOCAL_CLASSES YES)
    set(DOXYGEN_EXTRACT_LOCAL_METHODS YES)
    set(DOXYGEN_PROJECT_NAME "cpp-uuidv7")
    set(DOXYGEN_PROJECT_BRIEF "C++ implementation of RFC 4122 UUIDv7")

    doxygen_add_docs(
        uuidv7lib_docs
        ${CMAKE_CURRENT_SOURCE_DIR}/include/uuidv7
        ${CMAKE_CURRENT_SOURCE_DIR}/src
        COMMENT "Generate documentation"
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    )
endif()


# --- Install ---
include(CMakePackageConfigHelpers)

install(FILES
    include/uuidv7/uuidv7.hpp
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/uuidv7
    COMPONENT Devel
)
if (BUILD_SHARED_LIBS)
    install(FILES
        ${UUIDV7LIB_EXPORT_HEADER}
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/uuidv7
        COMPONENT Devel
    )
endif()

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
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/uuidv7
    COMPONENT Devel
)

write_basic_package_version_file(
    "${CMAKE_CURRENT_BINARY_DIR}/uuidv7ConfigVersion.cmake"
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY SameMajorVersion
)

install(FILES
    "${CMAKE_CURRENT_BINARY_DIR}/cmake/uuidv7Config.cmake"
    "${CMAKE_CURRENT_BINARY_DIR}/uuidv7ConfigVersion.cmake"
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/uuidv7
    COMPONENT Devel
)

export(EXPORT uuidv7Targets
    FILE "${CMAKE_CURRENT_BINARY_DIR}/uuidv7Config.cmake"
    NAMESPACE uuidv7::
)
