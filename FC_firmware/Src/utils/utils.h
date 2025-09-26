#ifndef UTILS_H
#define UTILS_H

#define utils_QUOTE(str) #str
#define utils_EXPAND_AND_QUOTE(str) utils_QUOTE(str)

#define utils_PI 3.14159265358979323846f

#define utils_RAD_TO_DEG(r) ((r) * 180 / utils_PI)
#define utils_DEG_TO_RAD(d) (180 * (d) / utils_PI)

#define utils_MIN(a, b) ((a) < (b) ? (a) : (b))
#define utils_MAX(a, b) ((a) > (b) ? (a) : (b))

#endif // UTILS_H
