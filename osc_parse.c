#include "osc.h"

#include <string.h>

// ============================================================================

OscBundle* osc_parse (const uint8_t* data, size_t size);

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
                msg->args[i].i32 =  (data[ptr + 0] << 24) |
                                    (data[ptr + 1] << 16) |
                                    (data[ptr + 2] <<  8) |
                                    (data[ptr + 3]      );
                break;

            // 64-bit
            case 'h':
            case 'd':
            case 't':
                msg->args[i].i64 =  (data[ptr + 0] << 56) |
                                    (data[ptr + 1] << 48) |
                                    (data[ptr + 2] << 40) |
                                    (data[ptr + 3] << 32) |
                                    (data[ptr + 4] << 24) |
                                    (data[ptr + 5] << 16) |
                                    (data[ptr + 6] <<  8) |
                                    (data[ptr + 7]      );
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

    OscBundle* bundle = NULL;

    return bundle;
}

OscBundle* osc_parse (const uint8_t* data, size_t size) {

    // Check if the message is a bundle
    const uint8_t magic[] = {'#', 'b', 'u', 'n', 'd', 'l', 'e'};
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
