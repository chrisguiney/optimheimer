//
// Created by chrisg on 3/22/25.
//

#ifndef OPH_ALLOCATOR_H
#define OPH_ALLOCATOR_H

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

struct oph_sys;

void* oph_calloc(
    struct oph_sys* sys,
    size_t nmemb,
    size_t size
);

#ifdef __cplusplus
};
#endif

#endif //OPH_ALLOCATOR_H
