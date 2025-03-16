#ifndef DEVICE_H
#define DEVICE_H

#include "oph.h"

#ifdef __cplusplus
extern "C" {
#endif

void oph_device_init(struct oph_device *dev);

void oph_device_draw(struct oph_device* dev);

#ifdef __cplusplus
}
#endif

#endif //DEVICE_H
