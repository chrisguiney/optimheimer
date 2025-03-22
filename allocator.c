#include "oph.h"
#include "allocator.h"

#include <assert.h>


void *oph_calloc(
    struct oph_sys *sys,
    size_t nmemb,
    size_t size
    )
{
    assert(sys != nullptr);
    assert(sys->allocator.calloc != nullptr);
    return sys->allocator.calloc(sys->allocator.ctx, nmemb, size);
}