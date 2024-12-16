#include "osc.h"

#include <gtest/gtest.h>

#include <stdio.h>

// ============================================================================

int load_file (const char* name, uint8_t** pdata, size_t* psize) {

    fprintf(stderr, "Loading '%s' ...\n", name);

    FILE* fp = fopen(name, "rb");
    if (!fp) {
        fprintf(stderr, " error opening file!\n");
        return -1;
    }

    fseek(fp, 0, SEEK_END);
    size_t size = ftell(fp);
    fseek(fp, 0, SEEK_SET);

    uint8_t* data = (uint8_t*)malloc(size);
    fread(data, size, 1, fp);
    fclose(fp);

    *pdata = data;
    *psize = size;

    fprintf(stderr, " %zu B\n", size);
    return 0;
}

void hexdump (const uint8_t* data, size_t size) {
    for (size_t i=0; i<size; ++i) {
        fprintf(stderr, "%02X", data[i]);
    }
    fprintf(stderr, "\n");
}

// ============================================================================

TEST(testParse, Reference1)
{
    uint8_t* data = NULL;
    size_t   size = 0;
    EXPECT_TRUE(load_file("tests/assets/ref1.bin", &data, &size) == 0);

    OscBundle* bundle = osc_parse(data, size);
    EXPECT_NE(bundle, nullptr);

    EXPECT_NE(bundle->messages, nullptr);
    OscMessage* msg = bundle->messages;

    EXPECT_STREQ(msg->addr, "/oscillator/4/frequency");
    EXPECT_STREQ(msg->tags, "f");
    EXPECT_FLOAT_EQ(msg->args[0].f32, 440.0f);

    osc_bundle_delete(bundle);
}

TEST(testParse, Reference2)
{
    uint8_t* data = NULL;
    size_t   size = 0;
    EXPECT_TRUE(load_file("tests/assets/ref2.bin", &data, &size) == 0);

    OscBundle* bundle = osc_parse(data, size);
    EXPECT_NE(bundle, nullptr);

    EXPECT_NE(bundle->messages, nullptr);
    OscMessage* msg = bundle->messages;

    EXPECT_STREQ(msg->addr, "/foo");
    EXPECT_STREQ(msg->tags, "iisff");
    EXPECT_EQ(msg->args[0].i32, 1000);
    EXPECT_EQ(msg->args[1].i32, -1);
    EXPECT_STREQ(msg->args[2].str, "hello");
    EXPECT_FLOAT_EQ(msg->args[3].f32, 1.234f);
    EXPECT_FLOAT_EQ(msg->args[4].f32, 5.678f);

    osc_bundle_delete(bundle);
}

// ============================================================================

TEST(testRoundtrip, Int32)
{
    OscMessage* msg = osc_message_create("i");
    EXPECT_NE(msg, nullptr);

    msg->addr = osc_strdup("/root");
    EXPECT_NE(msg->addr, nullptr);

    msg->args[0].i32 = 1234;

    // Encode
    uint8_t* data = NULL;
    size_t   size = 0;
    EXPECT_TRUE(osc_encode_message(msg, &data, &size) == 0);
    EXPECT_NE(data, nullptr);
    EXPECT_NE(size, 0);

    hexdump(data, size);

    // Decode
    OscBundle* bundle = osc_parse(data, size);
    EXPECT_NE(bundle, nullptr);

    EXPECT_NE(bundle->messages, nullptr);
    OscMessage* dec = bundle->messages;

    EXPECT_STREQ(dec->addr, msg->addr);
    EXPECT_STREQ(dec->tags, msg->tags);
    EXPECT_EQ(dec->args[0].i32, msg->args[0].i32);

    osc_message_delete(msg);
    osc_bundle_delete(bundle);
}

TEST(testRoundtrip, Float32)
{
    OscMessage* msg = osc_message_create("f");
    EXPECT_NE(msg, nullptr);

    msg->addr = osc_strdup("/root");
    EXPECT_NE(msg->addr, nullptr);

    msg->args[0].f32 = 123.4f;

    // Encode
    uint8_t* data = NULL;
    size_t   size = 0;
    EXPECT_TRUE(osc_encode_message(msg, &data, &size) == 0);
    EXPECT_NE(data, nullptr);
    EXPECT_NE(size, 0);

    hexdump(data, size);

    // Decode
    OscBundle* bundle = osc_parse(data, size);
    EXPECT_NE(bundle, nullptr);

    EXPECT_NE(bundle->messages, nullptr);
    OscMessage* dec = bundle->messages;

    EXPECT_STREQ(dec->addr, msg->addr);
    EXPECT_STREQ(dec->tags, msg->tags);
    EXPECT_FLOAT_EQ(dec->args[0].f32, msg->args[0].f32);

    osc_message_delete(msg);
    osc_bundle_delete(bundle);
}

TEST(testRoundtrip, String)
{
    OscMessage* msg = osc_message_create("s");
    EXPECT_NE(msg, nullptr);

    msg->addr = osc_strdup("/root");
    EXPECT_NE(msg->addr, nullptr);

    msg->args[0].str = osc_strdup("Hello!");

    // Encode
    uint8_t* data = NULL;
    size_t   size = 0;
    EXPECT_TRUE(osc_encode_message(msg, &data, &size) == 0);
    EXPECT_NE(data, nullptr);
    EXPECT_NE(size, 0);

    hexdump(data, size);

    // Decode
    OscBundle* bundle = osc_parse(data, size);
    EXPECT_NE(bundle, nullptr);

    EXPECT_NE(bundle->messages, nullptr);
    OscMessage* dec = bundle->messages;

    EXPECT_STREQ(dec->addr, msg->addr);
    EXPECT_STREQ(dec->tags, msg->tags);
    EXPECT_STREQ(dec->args[0].str, msg->args[0].str);

    osc_message_delete(msg);
    osc_bundle_delete(bundle);
}