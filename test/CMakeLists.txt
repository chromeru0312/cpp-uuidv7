mark_as_advanced(
    BUILD_GMOCK BUILD_GTEST
    gmock_build_tests gtest_build_samples gtest_build_tests
    gtest_disable_pthreads gtest_force_shared_crt gtest_hide_internal_symbols
)

add_executable(uuidv7lib_test
    tests.cpp
)
target_link_libraries(uuidv7lib_test PRIVATE gtest gmock gtest_main uuidv7::uuidv7)

gtest_discover_tests(uuidv7lib_test)
