#ifndef OPH_DEVICE_H
#define OPH_DEVICE_H

#include "oph.h"

#ifdef __cplusplus
extern "C"
{
#endif

void oph_device_init(struct oph_device *dev);

void oph_device_draw(struct oph_device *dev);

#ifdef __cplusplus
}
#endif

#endif //OPH_DEVICE_H
