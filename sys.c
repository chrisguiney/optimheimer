//
// Created by chrisg on 3/21/25.
//

#include "oph.h"
#include "sys.h"

#include <stdlib.h>
#include <string.h>

#include <sqlite3.h>

static struct oph_allocator default_allocator = {
    .ctx = nullptr,
    .calloc = oph_sys_calloc,
    .free = oph_sys_free,
};

void *oph_sys_calloc(
    void *ctx,
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

void oph_sys_free(void* ctx, void *ptr)
{
    free(ptr);
}

void oph_sys_init(
    const struct oph_configuration *config,
    struct oph_sys *sys
    )
{
    memset(sys, 0, sizeof(*sys));
    sys->allocator = default_allocator;
    sqlite3_open(config->db_path, &sys->db);
}

void oph_sys_destroy(struct oph_sys *sys)
{
    sqlite3_close(sys->db);
}