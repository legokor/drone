#include "utils/utils.h"
#include "stm32f4xx_hal.h"

uint32_t utils_GetMsSinceStartup(void) {
    return HAL_GetTick();
}
