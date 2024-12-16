#include "osc.h"

#include <gtest/gtest.h>

#include <stdio.h>

// ============================================================================

static int32_t allocCount = 0;

void* osc_malloc (size_t size) {
    allocCount++;
    return malloc(size);
}

void osc_free (void* ptr) {
    allocCount--;
    free(ptr);
}

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
    allocCount = 0;

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

    EXPECT_EQ(allocCount, 0);
}

TEST(testParse, Reference2)
{
    allocCount = 0;

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

    EXPECT_EQ(allocCount, 0);
}

// ============================================================================

TEST(testRoundtrip, Int32)
{
    allocCount = 0;

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
    EXPECT_EQ(size & 3, 0);

    hexdump(data, size);

    // Decode
    OscBundle* bundle = osc_parse(data, size);
    EXPECT_NE(bundle, nullptr);

    EXPECT_NE(bundle->messages, nullptr);
    OscMessage* dec = bundle->messages;

    EXPECT_STREQ(dec->addr, msg->addr);
    EXPECT_STREQ(dec->tags, msg->tags);
    EXPECT_EQ(dec->args[0].i32, msg->args[0].i32);

    osc_free(data);
    osc_message_delete(msg);
    osc_bundle_delete(bundle);

    EXPECT_EQ(allocCount, 0);
}

TEST(testRoundtrip, Int64)
{
    allocCount = 0;

    OscMessage* msg = osc_message_create("h");
    EXPECT_NE(msg, nullptr);

    msg->addr = osc_strdup("/root");
    EXPECT_NE(msg->addr, nullptr);

    msg->args[0].i64 = 0xABCDEF0012345678LL;

    // Encode
    uint8_t* data = NULL;
    size_t   size = 0;
    EXPECT_TRUE(osc_encode_message(msg, &data, &size) == 0);
    EXPECT_NE(data, nullptr);
    EXPECT_NE(size, 0);
    EXPECT_EQ(size & 3, 0);

    hexdump(data, size);

    // Decode
    OscBundle* bundle = osc_parse(data, size);
    EXPECT_NE(bundle, nullptr);

    EXPECT_NE(bundle->messages, nullptr);
    OscMessage* dec = bundle->messages;

    EXPECT_STREQ(dec->addr, msg->addr);
    EXPECT_STREQ(dec->tags, msg->tags);
    EXPECT_EQ(dec->args[0].i64, msg->args[0].i64);

    osc_free(data);
    osc_message_delete(msg);
    osc_bundle_delete(bundle);

    EXPECT_EQ(allocCount, 0);
}

TEST(testRoundtrip, Float32)
{
    allocCount = 0;

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
    EXPECT_EQ(size & 3, 0);

    hexdump(data, size);

    // Decode
    OscBundle* bundle = osc_parse(data, size);
    EXPECT_NE(bundle, nullptr);

    EXPECT_NE(bundle->messages, nullptr);
    OscMessage* dec = bundle->messages;

    EXPECT_STREQ(dec->addr, msg->addr);
    EXPECT_STREQ(dec->tags, msg->tags);
    EXPECT_FLOAT_EQ(dec->args[0].f32, msg->args[0].f32);

    osc_free(data);
    osc_message_delete(msg);
    osc_bundle_delete(bundle);

    EXPECT_EQ(allocCount, 0);
}

TEST(testRoundtrip, Float64)
{
    allocCount = 0;

    OscMessage* msg = osc_message_create("d");
    EXPECT_NE(msg, nullptr);

    msg->addr = osc_strdup("/root");
    EXPECT_NE(msg->addr, nullptr);

    msg->args[0].f64 = 0.123456789f;

    // Encode
    uint8_t* data = NULL;
    size_t   size = 0;
    EXPECT_TRUE(osc_encode_message(msg, &data, &size) == 0);
    EXPECT_NE(data, nullptr);
    EXPECT_NE(size, 0);
    EXPECT_EQ(size & 3, 0);

    hexdump(data, size);

    // Decode
    OscBundle* bundle = osc_parse(data, size);
    EXPECT_NE(bundle, nullptr);

    EXPECT_NE(bundle->messages, nullptr);
    OscMessage* dec = bundle->messages;

    EXPECT_STREQ(dec->addr, msg->addr);
    EXPECT_STREQ(dec->tags, msg->tags);
    EXPECT_FLOAT_EQ(dec->args[0].f64, msg->args[0].f64);

    osc_free(data);
    osc_message_delete(msg);
    osc_bundle_delete(bundle);

    EXPECT_EQ(allocCount, 0);
}

TEST(testRoundtrip, String)
{
    allocCount = 0;

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
    EXPECT_EQ(size & 3, 0);

    hexdump(data, size);

    // Decode
    OscBundle* bundle = osc_parse(data, size);
    EXPECT_NE(bundle, nullptr);

    EXPECT_NE(bundle->messages, nullptr);
    OscMessage* dec = bundle->messages;

    EXPECT_STREQ(dec->addr, msg->addr);
    EXPECT_STREQ(dec->tags, msg->tags);
    EXPECT_STREQ(dec->args[0].str, msg->args[0].str);

    osc_free(data);
    osc_message_delete(msg);
    osc_bundle_delete(bundle);

    EXPECT_EQ(allocCount, 0);
}

TEST(testRoundtrip, Bool)
{
    allocCount = 0;

    OscMessage* msg = osc_message_create("TF");
    EXPECT_NE(msg, nullptr);

    msg->addr = osc_strdup("/root");
    EXPECT_NE(msg->addr, nullptr);

    // Encode
    uint8_t* data = NULL;
    size_t   size = 0;
    EXPECT_TRUE(osc_encode_message(msg, &data, &size) == 0);
    EXPECT_NE(data, nullptr);
    EXPECT_NE(size, 0);
    EXPECT_EQ(size & 3, 0);

    hexdump(data, size);

    // Decode
    OscBundle* bundle = osc_parse(data, size);
    EXPECT_NE(bundle, nullptr);

    EXPECT_NE(bundle->messages, nullptr);
    OscMessage* dec = bundle->messages;

    EXPECT_STREQ(dec->addr, msg->addr);
    EXPECT_STREQ(dec->tags, msg->tags);
    EXPECT_EQ(dec->args[0].i32, 1);
    EXPECT_EQ(dec->args[1].i32, 0);

    osc_free(data);
    osc_message_delete(msg);
    osc_bundle_delete(bundle);

    EXPECT_EQ(allocCount, 0);
}

// ============================================================================

TEST(testRoundtrip, Bundle)
{
    allocCount = 0;

    // Message 1
    OscMessage* msg1 = osc_message_create("si");
    EXPECT_NE(msg1, nullptr);

    msg1->addr = osc_strdup("/root/1");
    EXPECT_NE(msg1->addr, nullptr);

    msg1->args[0].str = osc_strdup("test");
    msg1->args[1].i32 = 1234;

    // Message 2
    OscMessage* msg2 = osc_message_create("ds");
    EXPECT_NE(msg2, nullptr);

    msg2->addr = osc_strdup("/root/2");
    EXPECT_NE(msg2->addr, nullptr);

    msg2->args[0].f64 = 1.2345;
    msg2->args[1].str = osc_strdup("aloha");

    // Make bundle
    OscBundle* bundle = osc_bundle_create(5678);
    bundle->messages = msg1;
    msg1->next = msg2;

    // Encode
    uint8_t* data = NULL;
    size_t   size = 0;
    EXPECT_TRUE(osc_encode_bundle(bundle, &data, &size) == 0);
    EXPECT_NE(data, nullptr);
    EXPECT_NE(size, 0);
    EXPECT_EQ(size & 3, 0);

    hexdump(data, size);

    // Decode
    OscBundle* dec = osc_parse(data, size);
    EXPECT_NE(dec, nullptr);

    EXPECT_NE(dec->messages, nullptr);
    OscMessage* dec1 = dec->messages;

    EXPECT_NE(dec1->next, nullptr);
    OscMessage* dec2 = dec1->next;

    EXPECT_STREQ(dec1->addr, msg2->addr);
    EXPECT_STREQ(dec1->tags, msg2->tags);
    EXPECT_FLOAT_EQ(dec1->args[0].f64, msg2->args[0].f64);
    EXPECT_STREQ(dec1->args[1].str, msg2->args[1].str);
    
    EXPECT_STREQ(dec2->addr, msg1->addr);
    EXPECT_STREQ(dec2->tags, msg1->tags);
    EXPECT_STREQ(dec2->args[0].str, msg1->args[0].str);
    EXPECT_EQ(dec2->args[1].i32, msg1->args[1].i32);

    osc_free(data);
    osc_bundle_delete(bundle);
    osc_bundle_delete(dec);

    EXPECT_EQ(allocCount, 0);
}

// ============================================================================

TEST(testRandomRoundtrip, Random)
{
    const char      oscTypes[] = {'i', 'h', 'f', 'd', 's', 'T', 'F', 'N'};
    const size_t    numTests   = 50;

    // Do tests
    for (size_t i=0; i<numTests; ++i) {

        allocCount = 0;

        // Randomize message count
        size_t numMsgs = 1 + (rand() % 3);
        fprintf(stderr, "%2zu. (%zu)\n", i + 1, numMsgs);

        // Create the bundle
        OscBundle*  bundle = osc_bundle_create(5678);
        EXPECT_NE(bundle, nullptr);

        OscMessage* msgs[8];
        for (size_t j=0; j<numMsgs; ++j) {

            // Randomize argument count
            size_t numArgs = 1 + (rand() % 7);

            // Randomize tags
            char tags[8] = {0};
            for (size_t k=0; k<numArgs; ++k) {
                size_t n = rand() % sizeof(oscTypes);
                tags[k] = oscTypes[n];
            }

            fprintf(stderr, " %2zu. '%s'\n", j + 1, tags);

            // Create the message
            OscMessage* msg = osc_message_create(tags);
            EXPECT_NE(msg, nullptr);

            msg->addr = osc_strdup("/root");
            EXPECT_NE(msg->addr, nullptr);

            // Randomize values
            size_t len = 0;
            for (size_t k=0; k<numArgs; ++k) {
                switch (msg->tags[k]) {

                    case 'i':
                    case 'f':
                        for (size_t n=0; n<4; ++n) {
                            msg->args[k].b[n] = rand() % 255;
                        }
                        break;

                    case 'h':
                    case 'd':
                        for (size_t n=0; n<8; ++n) {
                            msg->args[k].b[n] = rand() % 255;
                        }
                        break;

                    case 's':
                        len = 4 + random() % 16;
                        msg->args[k].str = (char*)osc_malloc(len + 1);
                        for (size_t n=0; n<len; ++n) {
                            msg->args[k].str[n] = 'A' + rand() * ('z' - 'A');
                        }
                        msg->args[k].str[len] = 0;
                        break;
                    }
                }

            // Add the message
            osc_bundle_add_message(bundle, msg);
            msgs[j] = msg;
        }

        // Encode
        uint8_t* data = NULL;
        size_t   size = 0;
        EXPECT_TRUE(osc_encode_bundle(bundle, &data, &size) == 0);
        EXPECT_NE(data, nullptr);
        EXPECT_NE(size, 0);
        EXPECT_EQ(size & 3, 0);

        fprintf(stderr, " ");
        hexdump(data, size);

        // Decode
        OscBundle* dec = osc_parse(data, size);
        EXPECT_NE(dec, nullptr);

        // Compare messages
        size_t idx = 0;
        for (OscMessage* mss = dec->messages; mss; mss = mss->next) {
            OscMessage* ref = msgs[idx++];

            // Compare
            EXPECT_STREQ(mss->addr, ref->addr);
            EXPECT_STREQ(mss->tags, ref->tags);

            for (size_t j=0; j<strlen(mss->tags); ++j) {
                switch (mss->tags[j]) {

                    case 'i':
                    case 'f':
                        EXPECT_EQ(memcmp(mss->args[j].b, ref->args[j].b, 4), 0);
                        break;

                    case 'h':
                    case 'd':
                        EXPECT_EQ(memcmp(mss->args[j].b, ref->args[j].b, 8), 0);
                        break;

                    case 'T':
                        EXPECT_EQ(mss->args[j].i32, 1);
                        break;

                    case 'F':
                    case 'N':
                        EXPECT_EQ(mss->args[j].i32, 0);
                        break;

                    case 's':
                        EXPECT_STREQ(mss->args[j].str, ref->args[j].str);
                        break;
                }
            }
        }

        EXPECT_EQ(idx, numMsgs);

        // Cleanup
        osc_free(data);
        osc_bundle_delete(bundle);
        osc_bundle_delete(dec);

        EXPECT_EQ(allocCount, 0);
    }
}

