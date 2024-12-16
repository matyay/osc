#include <stdlib.h>

// ============================================================================

void* osc_malloc (size_t size) {
    return malloc(size);
}

void osc_free (void* ptr) {
    free(ptr);
}
