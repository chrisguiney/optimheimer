//
// Created by chrisg on 3/21/25.
//

#ifndef OPTIMHEIMER_SYS_H
#define OPTIMHEIMER_SYS_H

#ifdef __cplusplus
extern "C" {
#endif

struct oph_sys;
struct oph_allocator;

void *oph_calloc(
    struct oph_sys *sys,
    size_t nmemb,
    size_t size
);

void oph_sys_init(
    const struct oph_configuration *config,
    struct oph_sys* sys
);

#ifdef __cplusplus
};
#endif

#endif //OPTIMHEIMER_SYS_H
