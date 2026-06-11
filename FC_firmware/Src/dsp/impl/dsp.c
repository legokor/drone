#include "dsp/dsp.h"
#include "imu/imu.h"
#include "log/log.h"
#include "utils/utils.h"

#include <stddef.h>
#include <stdint.h>

#include "arm_math.h"
#include "stm32f4xx_hal.h"

// the definitions for the inputs / outputs
#define _dsp_DATA_IMPL(type, name)  \
    static type _dsp_##name;        \
    type dsp_get##name(void) {      \
        return _dsp_##name;         \
    }                               \
                                    \
    void dsp_set##name(type name) { \
        _dsp_##name = name;         \
    }

_dsp_DATA

#undef _dsp_DATA_IMPL

static uint32_t _dsp_lastRun = 0;

// static imu_Vec3 _dsp_angleGyr = { 0 };

// 0: full accel
// 1: full gyro
static float _dsp_alpha = 0.97;

void dsp_init(void) {
    log_debug("Initializing dsp...");

    _dsp_lastRun = HAL_GetTick();
}

static imu_Vec3 _dsp_avg(const imu_Vec3* buffer, size_t bufferSize) {
    imu_Vec3 sum = (imu_Vec3){ .x = 0, .y = 0, .z = 0 };

    for (size_t i = 0; i < bufferSize; i++) {
        imu_Vec3 tmp = (imu_Vec3){ .x = 0, .y = 0, .z = 0 };
        arm_add_f32(buffer[i].arr, sum.arr, tmp.arr, 3);
        sum = tmp;
    }

    imu_Vec3 div = (imu_Vec3){
        .x = 1.0f / bufferSize,
        .y = 1.0f / bufferSize,
        .z = 1.0f / bufferSize,
    };

    imu_Vec3 ret;
    arm_mult_f32(sum.arr, div.arr, ret.arr, 3);

    return ret;
}

static imu_Vec3 _rotate_vector(imu_Vec3 vec, imu_Vec3 angles) {
    imu_Vec3 rotated;

    float cosx = cosf(angles.x); // Roll
    float sinx = sinf(angles.x);

    float cosy = cosf(angles.y); // Pitch
    float siny = sinf(angles.y);

    float cosz = cosf(angles.z); // Yaw
    float sinz = sinf(angles.z);

    rotated.x = vec.x * (cosy * cosz) 
              + vec.y * (sinx * siny * cosz - cosx * sinz) 
              + vec.z * (cosx * siny * cosz + sinx * sinz);

    rotated.y = vec.x * (cosy * sinz) 
              + vec.y * (sinx * siny * sinz + cosx * cosz) 
              + vec.z * (cosx * siny * sinz - sinx * cosz);

    rotated.z = vec.x * (-siny) 
              + vec.y * (sinx * cosy) 
              + vec.z * (cosx * cosy);

    return rotated;
}

static size_t _dsp_accAvgP = 0, _dsp_gyrAvgP = 0;
static imu_Vec3 _dsp_accAvgData[32] = { 0 };
static imu_Vec3 _dsp_gyrAvgData[32] = { 0 };

void dsp_update(void) {
    // TODO: use a timer for usec resolution
    uint32_t now = HAL_GetTick();
    float dt = 0.001 * (now - _dsp_lastRun);
    _dsp_lastRun = now;

    imu_Vec3 inAcc = dsp_getInAcc();
    imu_Vec3 inGyr = dsp_getInGyr();

    imu_Vec3 offsets = (imu_Vec3){
        .x = 0.0f,
        .y = 0.0f,
        .z = utils_degToRad(-45),
    };
    inAcc = _rotate_vector(inAcc, offsets);
    inGyr = _rotate_vector(inGyr, offsets);

    _dsp_accAvgData[_dsp_accAvgP++] = inAcc;
    _dsp_accAvgP %= utils_arrayCount(_dsp_accAvgData);
    inAcc = _dsp_avg(_dsp_accAvgData, utils_arrayCount(_dsp_accAvgData));

    _dsp_gyrAvgData[_dsp_gyrAvgP++] = inGyr;
    _dsp_gyrAvgP %= utils_arrayCount(_dsp_gyrAvgData);
    inGyr = _dsp_avg(_dsp_gyrAvgData, utils_arrayCount(_dsp_gyrAvgData));

    static imu_Vec3 _dsp_gyrIntegral = { 0 };

    // integrate gyro
    // TODO: check if this works

    // inGyr *= dt (arm_scale_f32 supports in-place operation)
    arm_scale_f32(inGyr.arr, dt, inGyr.arr, 3);

    // tmp = _dsp_gyrIntegral + inGyr
    imu_Vec3 tmp;
    arm_add_f32(_dsp_gyrIntegral.arr, inGyr.arr, tmp.arr, 3);
    _dsp_gyrIntegral = tmp;

    // float rollA, pitchA;
    // arm_atan2_f32(inAcc.x, inAcc.z, &rollA);
    // arm_atan2_f32(inAcc.y, inAcc.z, &pitchA);

    // TODO: worse?
    float rollA = atan2f(inAcc.x, inAcc.z);
    float pitchA = atan2f(inAcc.y, inAcc.z);

    // complementary filter
    static float _dps_rollComp = 0, _dsp_pitchComp = 0;

    _dps_rollComp = rollA * (1 - _dsp_alpha) + _dsp_alpha * (_dps_rollComp + inGyr.y);
    _dsp_pitchComp = pitchA * (1 - _dsp_alpha) + _dsp_alpha * (_dsp_pitchComp + inGyr.x);

    imu_Vec3 out = (imu_Vec3){
        .roll = _dps_rollComp,
        .pitch = _dsp_pitchComp,
        .yaw = _dsp_gyrIntegral.yaw - utils_PI * 0.25f,
    };

    dsp_setOutAng(out);
}
