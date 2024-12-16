#include "osc.h"

#include <string.h>

// ============================================================================

static size_t osc_message_size (const OscMessage* msg) {

    size_t size = 0;

    // Address string + padding
    size += strlen(msg->addr) + 1;
    if (size & 3) size = (size & ~3) + 4;

    // Tag string + padding
    size += strlen(msg->tags) + 2;
    if (size & 3) size = (size & ~3) + 4;

    // Arguments
    for (size_t i=0; i<strlen(msg->tags); ++i) {

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
                arg_size = strlen(msg->args[i].str) + 1;
                if (arg_size & 3) arg_size = (arg_size & ~3) + 4;
                break;

            // Unknown, error
            default:
                return 0;
        }

        size += arg_size;
    }

    return size;
}

static size_t osc_bundle_size (const OscBundle* bundle) {

    size_t size = 0;

    // Header + timestamp
    size += 8 + 8;

    // Messages
    for (const OscMessage* msg = bundle->messages; msg; msg = msg->next) {
        size_t len = osc_message_size(msg);
        if (!len) return 0;

        size += 4 + len;
    }

    // Bundles
    for (const OscBundle* bun = bundle->bundles; bun; bun = bun->next) {
        size_t len = osc_bundle_size(bun);
        if (!len) return 0;

        size += 4 + len;
    }

    return size;
}

// ============================================================================

int osc_encode_message (const OscMessage* msg, uint8_t** pdata, size_t* psize)
{
    // Allocate buffer if needed
    uint8_t* data = *pdata;
    if (data == NULL) {

        // Compute message size
        size_t size = osc_message_size(msg);
        if (size == 0) return -1;

        // Allocate the buffer
        data = (uint8_t*)osc_malloc(size);
        if (!data) return -1;
    }

    size_t ptr = 0;
    size_t len;

    // Encode address string
    strcpy((char*)&data[ptr], msg->addr);
    ptr += strlen(msg->addr) + 1;
    for (; ptr & 3; ++ptr) data[ptr] = 0;

    // Encode tags string
    data[ptr++] = ',';
    strcpy((char*)&data[ptr], msg->tags);
    ptr += strlen(msg->tags) + 1;
    for (; ptr & 3; ++ptr) data[ptr] = 0;

    // Encode arguments
    for (size_t i=0; i<strlen(msg->tags); ++i) {
        switch (msg->tags[i]) {

            // 32-bit
            case 'i':
            case 'f':
            case 'r':
            case 'm':
                for (size_t j=0; j<4; ++j) {
                    data[ptr++] = msg->args[i].b[3 - j];
                }
                break;

            // 64-bit
            case 'h':
            case 'd':
            case 't':
                for (size_t j=0; j<8; ++j) {
                    data[ptr++] = msg->args[i].b[7 - j];
                }
                break;

            // char as 32-bit
            case 'c':
                data[ptr++] = 0;
                data[ptr++] = 0;
                data[ptr++] = 0;
                data[ptr++] = msg->args[i].i32 & 0x7F;
                break;

            // Data-less
            case 'T':
            case 'F':
            case 'N':
            case 'I':
                break;

            // String
            case 's':
            case 'S':
                len = strlen(msg->args[i].str);
                memcpy(&data[ptr], msg->args[i].str, len);
                ptr += len; data[ptr++] = 0;
                for (; ptr & 3; ++ptr) data[ptr] = 0;
                break;

            // Unknown, error
            default:
                // FIXME: Free buffer if allocated !
                return -1;
        }
    }

    // Finish
    *pdata = data;
    if (psize) {
        *psize = ptr;
    }

    return 0;
}

int osc_encode_bundle (const OscBundle* bundle, uint8_t** pdata, size_t* psize)
{
    // Allocate buffer if needed
    uint8_t* data = *pdata;
    if (data == NULL) {

        // Compute bundle size
        size_t size = osc_bundle_size(bundle);
        if (size == 0) return -1;

        // Allocate the buffer
        data = (uint8_t*)osc_malloc(size);
        if (!data) return -1;
    }

    size_t ptr = 0;

    // Magic word
    const uint8_t magic[] = {'#', 'b', 'u', 'n', 'd', 'l', 'e', 0};
    memcpy(&data[ptr], magic, sizeof(magic)); ptr += sizeof(magic);

    // Timestamp
    for (size_t i=0; i<8; ++i) {
        data[ptr++] = ((bundle->timestamp << (8 * i)) >> 56) & 0xFF;
    }

    // Messages
    for (const OscMessage* msg = bundle->messages; msg; msg = msg->next) {

        // Content
        uint8_t* buf = &data[ptr + 4];
        size_t   len = 0;

        if (osc_encode_message(msg, &buf, &len)) {
            // FIXME: Free buffer if allocated !
            return -1;
        }

        // Size
        for (size_t i=0; i<4; ++i) {
            data[ptr++] = ((len << (8 * i)) >> 24) & 0xFF;
        }

        ptr += len;
    }

    // Bundles
    for (const OscBundle* bun = bundle->bundles; bun; bun = bun->next) {

        // Content
        uint8_t* buf = &data[ptr + 4];
        size_t   len = 0;

        if (osc_encode_bundle(bun, &buf, &len)) {
            // FIXME: Free buffer if allocated !
            return -1;
        }

        // Size
        for (size_t i=0; i<4; ++i) {
            data[ptr++] = ((len << (8 * i)) >> 24) & 0xFF;
        }

        ptr += len;
    }

    // Finish
    *pdata = data;
    if (psize) {
        *psize = ptr;
    }

    return 0;
}
