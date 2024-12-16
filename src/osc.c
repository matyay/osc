#include "osc.h"

#include <string.h>

// ============================================================================

char* osc_strdup (const char* str) {
    size_t len = strlen(str);
    char*  res = (char*)osc_malloc(len + 1);

    strcpy(res, str);
    return res;
}

// ============================================================================

OscMessage* osc_message_create (const char* tags) {

    // Tag string cannot be NULL
    if (tags == NULL) {
        return NULL;
    }

    // Allocate the message
    OscMessage* msg = (OscMessage*)osc_malloc(sizeof(OscMessage));
    msg->addr = NULL;
    msg->tags = osc_strdup(tags);
    msg->next = NULL;

    // Allocate & clear args
    size_t asize = sizeof(OscArgument) * strlen(tags);
    if (asize) {
        msg->args = (OscArgument*)osc_malloc(asize);
        memset((void*)msg->args, 0, asize);
    }
    else {
        msg->args = NULL;
    }

    return msg;
}

OscMessage* osc_message_delete (const OscMessage* msg) {

    if (!msg) {
        return NULL;
    }

    // Free arguments (especially strings)
    if (msg->args) {
        for (size_t i=0; i<strlen(msg->tags); ++i) {
            if (msg->tags[i] == 's' || msg->tags[i] == 'S') {
                if (msg->args[i].str) {
                    osc_free((void*)msg->args[i].str);
                }
            }
        }

        osc_free((void*)msg->args);
    }

    // Free tags string
    if (msg->tags) {
        osc_free((void*)msg->tags);
    }

    // Free address string
    if (msg->addr) {
        osc_free((void*)msg->addr);
    }

    // Free the message
    osc_free((void*)msg);

    return NULL;
}

// ============================================================================

OscBundle* osc_bundle_create (int64_t timestamp) {

    OscBundle* bundle = (OscBundle*)osc_malloc(sizeof(OscBundle));
    bundle->timestamp = timestamp;
    bundle->messages  = NULL;
    bundle->bundles   = NULL;
    bundle->next      = NULL;

    return bundle;
}

OscBundle* osc_bundle_delete (const OscBundle* bundle) {

    if (!bundle) {
        return NULL;
    }

    // Delete member messages
    for (const OscMessage* curr = bundle->messages; curr;) {
        const OscMessage* next = curr->next;
        osc_message_delete(curr);
        curr = next;
    }

    // Delete member bundles
    for (const OscBundle* curr = bundle->bundles; curr;) {
        const OscBundle* next = curr->next;
        osc_bundle_delete(curr);
        curr = next;
    }

    // Delete self
    osc_free((void*)bundle);

    return NULL;
}

void osc_bundle_add_message (OscBundle* bundle, OscMessage* msg) {
    msg->next = bundle->messages;
    bundle->messages = msg;
}

void osc_bundle_add_bundle (OscBundle* bundle, OscBundle* other) {
    other->next = bundle->bundles;
    bundle->bundles = other;
}
