#include "osc.h"

#include <string.h>

// ============================================================================

static OscMessage* osc_parse_message (const uint8_t* data, size_t size) {

    // Sanity check
    if (size == 0 || data[0] != '/') {
        return NULL;
    }

    OscMessage* msg = NULL;
    size_t      ptr = 0;

    // Find the address/tag string boundary
    for (; data[ptr] != 0; ++ptr) {
        if (ptr >= size) {
            return NULL;
        }
    }
    ptr++;

    // Align pointer to 4
    if (ptr & 3) ptr = (ptr & ~3) + 4;

    // The tag string should begin with ','
    if (ptr >= size || data[ptr] != ',') {
        return NULL;
    }

    ptr++;
    size_t tags_ptr = ptr;

    // Find the arguments pointer
    for (; data[ptr] != 0; ++ptr) {
        if (ptr >= size) {
            return NULL;
        }
    }

    ptr++;

    // Create the message
    const char* addr_str = (const char*)&data[0];
    const char* tags_str = (const char*)&data[tags_ptr];

    msg = osc_message_create(tags_str);
    msg->addr = osc_strdup(addr_str);

    // Parse arguments
    for (size_t i=0; i<strlen(msg->tags); ++i) {

        // Align pointer to 4
        if (ptr & 3) ptr = (ptr & ~3) + 4;

        // Argument size
        size_t arg_size = 0;
        switch (msg->tags[i]) {

            case 'i':
            case 'f':
                arg_size = 4;
                break;

            case 'h':
            case 'd':
                arg_size = 8;
                break;

            case 'c':
            case 'r':
            case 'm':
                arg_size = 4;
                break;

            case 't':
                arg_size = 8;
                break;

            case 'T':
            case 'F':
            case 'N':
            case 'I':
                break;

            case 's':
            case 'S':
                for (size_t p=ptr; data[p] != 0 && p < size; ++p) {
                    arg_size++;
                }
                arg_size += 1;
                break;

            // Unknown, error
            default:
                osc_message_delete(msg);
                return NULL;
        }

        // Check
        if ((ptr + arg_size) > size) {
            osc_message_delete(msg);
            return NULL;
        }

        // Decode
        switch (msg->tags[i]) {

            // 32-bit
            case 'i':
            case 'f':
            case 'r':
            case 'm':
                for (size_t j=0; j<4; ++j) {
                    msg->args[i].b[3 - j] = data[ptr + j];
                }
                break;

            // 64-bit
            case 'h':
            case 'd':
            case 't':
                for (size_t j=0; j<8; ++j) {
                    msg->args[i].b[7 - j] = data[ptr + j];
                }
                break;

            // char as 32-bit
            case 'c':
                msg->args[i].i32 = data[ptr + 3] & 0x7F;
                break;

            // True
            case 'T':
                msg->args[i].i32 = 1;
                break;

            // False / Null
            case 'F':
            case 'N':
                msg->args[i].i32 = 0;
                break;

            // Infinity
            case 'I':
                msg->args[i].f32 = 0x7F800000; // IEEE 754 +Inf
                break;

            // String
            case 's':
            case 'S':
                msg->args[i].str = osc_strdup((const char*)&data[ptr]);
                break;
        }

        // Next
        ptr += arg_size;
    }

    return msg;
}

static OscBundle* osc_parse_bundle (const uint8_t* data, size_t size) {

    const uint8_t magic[] = {'#', 'b', 'u', 'n', 'd', 'l', 'e', 0};

    // Too small to fit the header
    if (size < 16) {
        return NULL;
    }

    // Skip magic
    size_t ptr = 8;

    // Timestamp
    int64_t timestamp = 0;
    for (size_t i=0; i<8; ++i) {
        timestamp <<= 8;
        timestamp  |= data[ptr++];
    }

    // Allocate the bundle
    OscBundle* bundle = osc_bundle_create(timestamp);

    // Parse bundle items
    while (ptr < size) {

        // Size
        size_t len = 0;
        for (size_t i=0; i<4; ++i) {
            len <<= 8;
            len  |= data[ptr++];
        }

        // Check if the message is a bundle
        const int isBundle = (len > sizeof(magic)) &&
                             !memcmp(&data[ptr], magic, sizeof(magic));

        // Got a bundle
        if (isBundle) {

            OscBundle* bun = osc_parse_bundle(&data[ptr], len);
            if (!bun) {
                osc_bundle_delete(bundle);
                return NULL;
            }

            bun->next = bundle->bundles;
            bundle->bundles = bun;
        }

        // Got a message
        else {

            OscMessage* msg = osc_parse_message(&data[ptr], len);
            if (!msg) {
                osc_bundle_delete(bundle);
                return NULL;
            }

            msg->next = bundle->messages;
            bundle->messages = msg;
        }

        // Next
        ptr += len;
    }

    return bundle;
}

// ============================================================================

OscBundle* osc_parse (const uint8_t* data, size_t size) {

    // Check if the message is a bundle
    const uint8_t magic[] = {'#', 'b', 'u', 'n', 'd', 'l', 'e', 0};
    const int isBundle = (size > sizeof(magic)) &&
                         !memcmp(data, magic, sizeof(magic));

    // Parse bundle
    if (isBundle) {
        return osc_parse_bundle(data, size);
    }

    // Parse message and pack it into a Bundle
    else {

        OscMessage* msg = osc_parse_message (data, size);
        if (!msg) return NULL;

        OscBundle* bundle = osc_bundle_create(OSC_IMMEDIATE);
        bundle->messages = msg;

        return bundle;
    }
}
