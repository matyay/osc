#ifndef OSC_H
#define OSC_H

#include <stdint.h>
#include <stdlib.h>

// ============================================================================

// OSC "immediate" time tag
#define OSC_IMMEDIATE 1LL

// ============================================================================

// OSC message argument
typedef union _OscArgument {
    int32_t i32;
    float   f32;
    int64_t i64;
    double  f64;
    char*   str;
    uint8_t b[8];
} OscArgument;

// OSC message
typedef struct _OscMessage {

    char*           addr;   // Address string
    char*           tags;   // Tag string
    OscArgument*    args;   // Argument array (of length of the tag string)

    struct _OscMessage* next;

} OscMessage;

// OSC bundle (linked list)
typedef struct _OscBundle {

    int64_t             timestamp;  // Timestamp
    struct _OscMessage* messages;   // Messages
    struct _OscBundle*  bundles;    // Sub-bundles

    struct _OscBundle*  next;

} OscBundle;

// ============================================================================

extern void* osc_malloc (size_t size);
extern void  osc_free   (void* ptr);

char* osc_strdup (const char* str);

// ============================================================================

OscMessage* osc_message_create  (const char* tags);
OscMessage* osc_message_delete  (const OscMessage* msg);

OscBundle* osc_bundle_create    (int64_t timestamp);
OscBundle* osc_bundle_delete    (const OscBundle* bundle);

void osc_bundle_add_message (OscBundle* bundle, OscMessage* msg);
void osc_bundle_add_bundle  (OscBundle* bundle, OscMessage* other);

// ============================================================================

OscBundle* osc_parse (const uint8_t* data, size_t size);

// ============================================================================

int osc_encode_message (const OscMessage* msg, uint8_t** pdata, size_t* psize);
int osc_encode_bundle (const OscBundle* bundle, uint8_t** pdata, size_t* psize);

// ============================================================================

#endif // OSC_H
