#include "allocator.h"

#include "oph.h"

#include <assert.h>

void *oph_calloc(struct oph_sys *sys, size_t nmemb, size_t size)
{
    assert(sys != nullptr);
    assert(sys->allocator.calloc != nullptr);
    assert(nmemb > 0);
    assert(size > 0);
    return sys->allocator.calloc(sys->allocator.ctx, nmemb, size);
}