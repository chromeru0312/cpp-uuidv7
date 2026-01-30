#include <array>
#include <cstdint>
#include <cstring>
#include <optional>
#include <string>
#include <utility>
#include <gtest/gtest.h>
#include "uuidv7/uuidv7.hpp"
#include "uuidv7/generator.hpp"

TEST(UUIDv7, Generate)
{
    // Generate Default
    std::optional<uuidv7::uuidv7> opt_uuid;
    ASSERT_NO_THROW(opt_uuid = uuidv7::uuidv7_generator::generate_default());
    uuidv7::uuidv7 uuid = opt_uuid.value();

    EXPECT_EQ((uuid.get_bytes()[6] >> 4) & 0x0F, 7); // Version
    EXPECT_EQ((uuid.get_bytes()[8] >> 6) & 0x03, 2); // Variant
    EXPECT_EQ(uuid.to_string(true).length(), 36);
    EXPECT_EQ(uuid.to_string(false).length(), 32);

    // Generate with custom generator
    std::array<uint8_t, 16> uuid_bytes = {
        0x04, 0x18, 0x46, 0xe8, 0x1c, 0x98, // 2112-09-03 [future]
        0x70, 0x00,
        0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
    };
    ASSERT_NO_THROW(opt_uuid = uuidv7::uuidv7::from_bytes(uuid_bytes));
    uuidv7::uuidv7 base_uuid = opt_uuid.value();

    uuidv7::uuidv7_generator generator(base_uuid);
    ASSERT_NO_THROW(opt_uuid = generator.generate());
    uuidv7::uuidv7 uuid2 = opt_uuid.value();
    std::array<uint8_t, 16> bytes2 = uuid2.get_bytes();

    EXPECT_EQ((bytes2[6] >> 4) & 0x0F, 7); // Version
    EXPECT_EQ((bytes2[8] >> 6) & 0x03, 2); // Variant
    EXPECT_EQ(std::memcmp(&bytes2[0], &uuid_bytes[0], 15), 0);
    EXPECT_EQ(bytes2[15], uuid_bytes[15] + 1);
}

TEST(UUIDv7, GenerateError)
{
    // sequence overflow
    std::array<uint8_t, 16> uuid_bytes = {
        0x04, 0x18, 0x46, 0xe8, 0x1c, 0x98, // 2112-09-03 [future]
        0x7f, 0xff,
        0xbf, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff
    };
    std::optional<uuidv7::uuidv7> opt_uuid;
    ASSERT_NO_THROW(opt_uuid = uuidv7::uuidv7::from_bytes(uuid_bytes));
    uuidv7::uuidv7 base_uuid = opt_uuid.value();

    uuidv7::uuidv7_generator generator(base_uuid);
    ASSERT_THROW(opt_uuid = generator.generate(), uuidv7::sequence_overflow_error);
}

TEST(UUIDv7, ConvertString)
{
    std::optional<uuidv7::uuidv7> opt_uuid;

    // from string (including hyphens)
    std::string uuid_str1 = "01965347-e56d-7571-a1bb-6120dba3a645";
    ASSERT_NO_THROW(opt_uuid = uuidv7::uuidv7::parse(uuid_str1));
    uuidv7::uuidv7 uuid1 = opt_uuid.value();

    // from string (without hyphens)
    std::string uuid_str2 = "01965347e56d7571a1bb6120dba3a645";
    ASSERT_NO_THROW(opt_uuid = uuidv7::uuidv7::parse(uuid_str2));
    uuidv7::uuidv7 uuid2 = opt_uuid.value();

    ASSERT_EQ(uuid1, uuid2);

    // to string (without hyphens)
    std::string converted_str1 = uuid1.to_string(false);
    EXPECT_EQ(converted_str1, uuid_str2);
    EXPECT_EQ(converted_str1.length(), 32);

    // to string (including hyphens)
    std::string converted_str2 = uuid2.to_string(true);
    EXPECT_EQ(converted_str2, uuid_str1);
    EXPECT_EQ(converted_str2.length(), 36);
}

TEST(UUIDv7, ConvertBytes)
{
    std::optional<uuidv7::uuidv7> opt_uuid;

    // from bytes (std::array)
    std::array<uint8_t, 16> uuid_bytes = {
        0x01, 0x96, 0x53, 0x47,
        0xe5, 0x6d,
        0x75, 0x71,
        0xa1, 0xbb,
        0x61, 0x20, 0xdb, 0xa3, 0xa6, 0x45
    };
    ASSERT_NO_THROW(opt_uuid = uuidv7::uuidv7::from_bytes(uuid_bytes));
    uuidv7::uuidv7 uuid1 = opt_uuid.value();

    // from bytes (pointer)
    uint8_t uuid_bytes_array[16];
    std::memcpy(uuid_bytes_array, uuid_bytes.data(), 16);
    ASSERT_NO_THROW(opt_uuid = uuidv7::uuidv7::from_bytes(uuid_bytes_array));
    uuidv7::uuidv7 uuid2 = opt_uuid.value();

    ASSERT_EQ(uuid1, uuid2);

    // to bytes
    std::array<uint8_t, 16> converted_bytes1 = uuid1.get_bytes();
    std::array<uint8_t, 16> converted_bytes2 = uuid2.get_bytes();
    EXPECT_EQ(converted_bytes1, uuid_bytes);
    EXPECT_EQ(converted_bytes2, uuid_bytes);
}

TEST(UUIDv7, ConvertError)
{
    std::string uuid_str;

    // from string (invalid format - length)
    uuid_str = "01965347-e56d-7571-a1bb-";
    ASSERT_THROW(uuidv7::uuidv7::parse(uuid_str), uuidv7::invalid_format_error);

    // from string (invalid format - non-hyphen length)
    uuid_str = "01965347-e56d-7571-a1bb-6120dba3";
    ASSERT_THROW(uuidv7::uuidv7::parse(uuid_str), uuidv7::invalid_format_error);

    // from string (invalid format - hyphen position)
    uuid_str = "0196534-7e56-d757-1a1b-b6120dba3a645";
    ASSERT_THROW(uuidv7::uuidv7::parse(uuid_str), uuidv7::invalid_format_error);

    // from string (invalid format - characters)
    uuid_str = "01965347-e56d-7571-a1bb-6120dba3a64Z"; // 'Z' is invalid
    ASSERT_THROW(uuidv7::uuidv7::parse(uuid_str), uuidv7::invalid_format_error);

    // from string (invalid version)
    uuid_str = "75f50a24-995d-4606-bf3e-7f6c5b76c5c1"; // version 4
    ASSERT_THROW(uuidv7::uuidv7::parse(uuid_str), uuidv7::invalid_format_error);

    // from string (invalid variant)
    uuid_str = "01965347-e56d-7571-01bb-6120dba3a645"; // invalid variant
    ASSERT_THROW(uuidv7::uuidv7::parse(uuid_str), uuidv7::invalid_format_error);


    std::array<uint8_t, 16> valid_uuid_bytes = {
        0x01, 0x96, 0x53, 0x47,
        0xe5, 0x6d,
        0x75, 0x71,
        0xa1, 0xbb,
        0x61, 0x20, 0xdb, 0xa3, 0xa6, 0x45
    };
    std::array<uint8_t, 16> uuid_bytes;

    // from bytes (invalid version)
    uuid_bytes = valid_uuid_bytes;
    uuid_bytes[6] = (uuid_bytes[6] & 0x0F) | 0x40; // set version to 4
    ASSERT_THROW(uuidv7::uuidv7::from_bytes(uuid_bytes), uuidv7::invalid_format_error);

    // from bytes (invalid variant)
    uuid_bytes = valid_uuid_bytes;
    uuid_bytes[8] = (uuid_bytes[8] & 0x3F) | 0x00; // set invalid variant
    ASSERT_THROW(uuidv7::uuidv7::from_bytes(uuid_bytes), uuidv7::invalid_format_error);

    // from bytes (null pointer)
    ASSERT_THROW(uuidv7::uuidv7::from_bytes(nullptr), uuidv7::invalid_format_error);
}

TEST(UUIDv7, AdditionalMonotonicity)
{
    uuidv7::uuidv7 uuid = uuidv7::uuidv7_generator::generate_default();
    for (int i = 0; i < 1000; i++) {
        uuidv7::uuidv7 next_uuid = uuidv7::uuidv7_generator::generate_default();
        EXPECT_GT(next_uuid.get_bytes(), uuid.get_bytes());
        uuid = std::move(next_uuid);
    }
}
