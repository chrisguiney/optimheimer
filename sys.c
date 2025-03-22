//
// Created by chrisg on 3/21/25.
//

#include "oph.h"
#include "sys.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>

static void *oph_sys_calloc(oph_allocator_ctx ctx, size_t nmemb, size_t size);

static void oph_sys_free(oph_allocator_ctx ctx, void *ptr);

static struct oph_allocator default_allocator = {
    .ctx = nullptr,
    .calloc = oph_sys_calloc,
    .free = oph_sys_free,
};


static void *oph_sys_calloc(
    oph_allocator_ctx ctx,
    size_t nmemb,
    size_t size)
{
    void *ptr = calloc(nmemb, size);
    if(!ptr)
    {
        abort();
    }
    return ptr;
}

static void oph_sys_free(oph_allocator_ctx ctx, void *ptr)
{
    free(ptr);
}

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

void oph_sys_init(
    const struct oph_configuration *config,
    struct oph_sys *sys
    )
{
    memset(sys, 0, sizeof(*sys));
    sys->allocator = default_allocator;
}