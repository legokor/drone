#ifndef DSP_H
#define DSP_H

#include "imu/imu.h"

// TODO: simplify...

/// some X macro magic to define inputs / outputs for the module

// the declaration for the data item
#define _dsp_DATA_IMPL(type, name)          \
    [[nodiscard]] type dsp_get##name(void); \
    void dsp_set##name(type name);

// this will get expanded to either the declaration (dsp.h)
// or the definition (dsp.c)
#define _dsp_DATA_ITEM(type, name) _dsp_DATA_IMPL(type, name)

// we define the actual input / output values
#define _dsp_DATA                         \
    /* input */                           \
    _dsp_DATA_ITEM(imu_Vec3, InAcc) /**/  \
    _dsp_DATA_ITEM(imu_Vec3, InGyr) /**/  \
    _dsp_DATA_ITEM(float, InBar)    /**/  \
                                          \
    /* output */                          \
    _dsp_DATA_ITEM(imu_Vec3, OutAng) /**/ \
    _dsp_DATA_ITEM(imu_Vec3, OutVel) /**/ \
    _dsp_DATA_ITEM(float, OutHeight)

_dsp_DATA

// undef the impl, so we can redefine it later
#undef _dsp_DATA_IMPL

void dsp_init(void);
void dsp_update(void);

#endif // DSP_H
