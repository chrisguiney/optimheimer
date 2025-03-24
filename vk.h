//
// Created by chrisg on 3/23/25.
//

#ifndef OPH_VK_H
#define OPH_VK_H

#ifdef __cplusplus
extern "C"
{
#endif

struct oph_sys;
struct oph_vk_defaults;
struct oph_vk_state;

void oph_vk_init(struct oph_sys *sys,
                 const struct oph_vk_defaults *defaults,
                 struct oph_vk_state *vk);

#ifdef __cplusplus
};
#endif

#endif //OPH_VK_H
