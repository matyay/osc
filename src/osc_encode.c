#include "osc.h"

#include <string.h>

// ============================================================================

int osc_encode_message (const OscMessage* msg, uint8_t** pdata, size_t* psize)
{
    // Compute buffer size
    size_t size = 0;

    size += strlen(msg->addr) + 1;
    if (size & 3) size = (size & ~3) + 4;

    size += strlen(msg->tags) + 2;
    if (size & 3) size = (size & ~3) + 4;

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
                return -1;
        }

        size += arg_size;
    }

    // Allocate the buffer
    uint8_t* data = (uint8_t*)osc_malloc(size);
    size_t   ptr  = 0;
    size_t   len;

    // Encode data
    strcpy((char*)&data[ptr], msg->addr);
    ptr += strlen(msg->addr) + 1;
    for (; ptr & 3; ++ptr) data[ptr] = 0;

    data[ptr++] = ',';
    strcpy((char*)&data[ptr], msg->tags);
    ptr += strlen(msg->tags) + 1;
    for (; ptr & 3; ++ptr) data[ptr] = 0;

    for (size_t i=0; i<strlen(msg->tags); ++i) {
        switch (msg->tags[i]) {

            // 32-bit
            case 'i':
            case 'f':
            case 'r':
                for (size_t j=0; j<4; ++j) {
                    data[ptr++] = msg->args[i].b[3 - j];
                }
                break;

            // 64-bit
            case 'h':
            case 'd':
            case 't':
                for (size_t j=0; j<4; ++j) {
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

            // String
            case 's':
            case 'S':
                len = strlen(msg->args[i].str);
                memcpy(&data[ptr], msg->args[i].str, len);
                ptr += len; data[ptr++] = 0;
                for (; ptr & 3; ++ptr) data[ptr] = 0;
                break;
        }
    }

    // Finish
    *psize = size;
    *pdata = data;

    return 0;
}
