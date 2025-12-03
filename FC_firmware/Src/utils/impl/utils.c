#include "utils/utils.h"

const float utils_PI = 3.14159265358979323846f;

float utils_radToDeg(float r) {
    return r * 180 / utils_PI;
}
float utils_degToRad(float d) {
    return d * utils_PI / 180;
}

float utils_mapF(float val, float s1, float e1, float s2, float e2) {
    return s2 + (e2 - s2) * ((val - s1) / (e1 - s1));
}
double utils_mapD(double val, double s1, double e1, double s2, double e2) {
    return s2 + (e2 - s2) * ((val - s1) / (e1 - s1));
}

#define __utils_DEFINE_MIN_MAX_CLAMP(type, type_short)           \
    type utils_min##type_short(type a, type b) {                 \
        return a < b ? a : b;                                    \
    }                                                            \
    type utils_max##type_short(type a, type b) {                 \
        return a > b ? a : b;                                    \
    }                                                            \
    type utils_clamp##type_short(type val, type min, type max) { \
        if (val < min)                                           \
            return min;                                          \
        if (val > max)                                           \
            return max;                                          \
        return val;                                              \
    }

__utils_DEFINE_MIN_MAX_CLAMP(int8_t, I8)
__utils_DEFINE_MIN_MAX_CLAMP(int16_t, I16)
__utils_DEFINE_MIN_MAX_CLAMP(int32_t, I32)
__utils_DEFINE_MIN_MAX_CLAMP(int64_t, I64)
__utils_DEFINE_MIN_MAX_CLAMP(uint8_t, U8)
__utils_DEFINE_MIN_MAX_CLAMP(uint16_t, U16)
__utils_DEFINE_MIN_MAX_CLAMP(uint32_t, U32)
__utils_DEFINE_MIN_MAX_CLAMP(uint64_t, U64)
__utils_DEFINE_MIN_MAX_CLAMP(float, F)
__utils_DEFINE_MIN_MAX_CLAMP(double, D)
