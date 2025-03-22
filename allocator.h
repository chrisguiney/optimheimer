//
// Created by chrisg on 3/22/25.
//

#ifndef OPH_ALLOCATOR_H
#define OPH_ALLOCATOR_H

#ifdef __cplusplus
extern "C" {
#endif

void* oph_calloc(
    struct oph_sys* sys,
    size_t nmemb,
    size_t size
);

#ifdef __cplusplus
};
#endif

#endif //OPH_ALLOCATOR_H
