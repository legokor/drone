#ifndef UTILS_H
#define UTILS_H

#include <stdint.h>

#define utils_QUOTE(str) #str
#define utils_EXPAND_AND_QUOTE(str) utils_QUOTE(str)

extern const float utils_PI;

float utils_radToDeg(float r);
float utils_degToRad(float d);

#define __utils_DECLARE_MIN_MAX_CLAMP(type, type_short) \
    type utils_min##type_short(type a, type b);         \
    type utils_max##type_short(type a, type b);         \
    type utils_clamp##type_short(type val, type min, type max)

__utils_DECLARE_MIN_MAX_CLAMP(int8_t, I8);
__utils_DECLARE_MIN_MAX_CLAMP(int16_t, I16);
__utils_DECLARE_MIN_MAX_CLAMP(int32_t, I32);
__utils_DECLARE_MIN_MAX_CLAMP(int64_t, I64);
__utils_DECLARE_MIN_MAX_CLAMP(uint8_t, U8);
__utils_DECLARE_MIN_MAX_CLAMP(uint16_t, U16);
__utils_DECLARE_MIN_MAX_CLAMP(uint32_t, U32);
__utils_DECLARE_MIN_MAX_CLAMP(uint64_t, U64);
__utils_DECLARE_MIN_MAX_CLAMP(float, F);
__utils_DECLARE_MIN_MAX_CLAMP(double, D);

#undef __utils_DECLARE_MIN_MAX_CLAMP

#define utils_min(a, b)         \
    _Generic((a),               \
        int8_t: utils_minI8,    \
        int16_t: utils_minI16,  \
        int32_t: utils_minI32,  \
        int64_t: utils_minI64,  \
        uint8_t: utils_minU8,   \
        uint16_t: utils_minU16, \
        uint32_t: utils_minU32, \
        uint64_t: utils_minU64, \
        float: utils_minF,      \
        double: utils_minD)(a, b)

#define utils_max(a, b)         \
    _Generic((a),               \
        int8_t: utils_maxI8,    \
        int16_t: utils_maxI16,  \
        int32_t: utils_maxI32,  \
        int64_t: utils_maxI64,  \
        uint8_t: utils_maxU8,   \
        uint16_t: utils_maxU16, \
        uint32_t: utils_maxU32, \
        uint64_t: utils_maxU64, \
        float: utils_maxF,      \
        double: utils_maxD)(a, b)

#define utils_clamp(val, min, max) \
    _Generic((val),                \
        int8_t: utils_clampI8,     \
        int16_t: utils_clampI16,   \
        int32_t: utils_clampI32,   \
        int64_t: utils_clampI64,   \
        uint8_t: utils_clampU8,    \
        uint16_t: utils_clampU16,  \
        uint32_t: utils_clampU32,  \
        uint64_t: utils_clampU64,  \
        float: utils_clampF,       \
        double: utils_clampD)(val, min, max)

float utils_mapF(float val, float s1, float e1, float s2, float e2);
double utils_mapD(double val, double s1, double e1, double s2, double e2);

#define utils_map(val, s1, s2, e1, e2) \
    _Generic((val), /* */              \
        float: utils_mapF,             \
        double: utils_mapD)(val, s1, s2, e1, e2)

#define utils_arrayCount(arr) (sizeof(arr) / sizeof(arr[0]))

#endif // UTILS_H
