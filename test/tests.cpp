#include <gtest/gtest.h>
#include "uuidv7/uuidv7.hpp"

TEST(UUIDv7, Generate)
{
    uuidv7::uuidv7 uuid = uuidv7::uuidv7_generator::generate_default();

    EXPECT_EQ((uuid.get_bytes()[6] >> 4) & 0x0F, 7); // Version
    EXPECT_EQ((uuid.get_bytes()[8] >> 6) & 0x03, 2); // Variant

    std::string uuid_str = uuid.to_string();
    EXPECT_EQ(uuid_str.length(), 36);
}

TEST(UUIDv7, Parse)
{
    std::string uuid_str = "019653b5-c655-7571-a1bb-6120dba3a645";
    uuidv7::uuidv7 uuid = uuidv7::uuidv7::parse(uuid_str);

    EXPECT_EQ(uuid.get_bytes()[6] >> 4, 7); // Version
    EXPECT_EQ(uuid.get_bytes()[8] >> 6, 2); // Variant

    std::cerr << "Parsed UUIDv7: " << uuid.to_string() << std::endl;
}

TEST(UUIDv7, AdditionalMonotonicity)
{
    uuidv7::uuidv7 uuid = uuidv7::uuidv7_generator::generate_default();
    std::this_thread::sleep_for(std::chrono::milliseconds(1));

    uuidv7::uuidv7 uuid2 = uuidv7::uuidv7_generator::generate_default();
    EXPECT_GT(uuid2.get_bytes()[0], uuid.get_bytes()[0]);
}