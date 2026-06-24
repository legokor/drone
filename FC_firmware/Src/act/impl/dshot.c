#include <inttypes.h>
#include <math.h>
#include <stdio.h>
#include "config.h"
#include "log/log.h"
#include "sys/time.h"

#if config_ACT_MODE == config_ACT_MODE_DSHOT

#include "act/impl/dshot.h"
#include "act/act.h"
#include "err/err.h"
#include "irq/irq.h"
#include "utils/utils.h"

#include "stm32f4xx_hal_tim.h"

#include <stddef.h>
#include <stdint.h>
#include <string.h>

#define _act_dshot_FRAME_BITS 16
// #define _act_dshot_FRAME_BITSasdasd(bs) uint##bs##_t
// #define _act_dshot_FRAME_BITSasd(bs) _act_dshot_FRAME_BITSasdasd(bs)

// typedef _act_dshot_FRAME_BITSasd(_act_dshot_FRAME_BITS) _act_dshot_Frameasd;
typedef uint16_t _act_dshot_Frame;
typedef uint32_t _act_dshot_TimerValue;

static TIM_HandleTypeDef* volatile _dshot_timers[act_MOTOR_COUNT];
static volatile uint32_t _dshot_timerChannels[act_MOTOR_COUNT];

static volatile bool _dshot_dmaActive[act_MOTOR_COUNT] = { false };

// +zero at the end to keep pin at 0
static volatile __ALIGN_BEGIN _act_dshot_TimerValue
    _act_dshot_frontBuffer[act_MOTOR_COUNT][_act_dshot_FRAME_BITS + 1] __ALIGN_END = { 0 };

// for next frame
static volatile bool _act_dshot_speedBackBufferFilled[act_MOTOR_COUNT] = { false };
static volatile bool _act_dshot_commandBackBufferFilled[act_MOTOR_COUNT] = { false };
static volatile _act_dshot_TimerValue _act_dshot_speedBackBuffer[act_MOTOR_COUNT][_act_dshot_FRAME_BITS] = { 0 };
static volatile _act_dshot_TimerValue _act_dshot_commandBackBuffer[act_MOTOR_COUNT][_act_dshot_FRAME_BITS] = { 0 };

// https://betaflight.com/docs/development/API/Dshot
// https://github.com/betaflight/betaflight/blob/70141ae2386fc151aa76e73678794e12c79b2afa/src/main/drivers/dshot.c#L123
static _act_dshot_Frame _dshot_createDshotFrameFromRaw(_act_dshot_Frame raw) {
    bool telemetry = false;
    _act_dshot_Frame frame = (raw << 1) | (telemetry ? 1 : 0);
    _act_dshot_Frame crc = (frame ^ (frame >> 4) ^ (frame >> 8)) & 0xF;

    frame = (frame << 4) | crc;

    return frame;
}

#define _act_dshot_MIN_RAW_THROTTLE 48
#define _act_dshot_MAX_RAW_THROTTLE 2047

static uint16_t _act_dshot_calculateRawThrottle(float motor) {
    return utils_clamp( //
        (uint16_t) roundf(_act_dshot_MIN_RAW_THROTTLE + motor * _act_dshot_MAX_RAW_THROTTLE),
        _act_dshot_MIN_RAW_THROTTLE, //
        _act_dshot_MAX_RAW_THROTTLE  //
    );
}

static _act_dshot_Frame _dshot_createDshotSpeedFrame(uint16_t throttle) {
    return _dshot_createDshotFrameFromRaw(throttle);
}

static _act_dshot_Frame _dshot_createCommandDshotFrame(dshot_Command cmd) {
    return _dshot_createDshotFrameFromRaw(cmd);
}

/* static size_t _act_dshot_getActiveChannel(TIM_HandleTypeDef* htim) {
    switch (HAL_TIM_GetActiveChannel(htim)) {
        case HAL_TIM_ACTIVE_CHANNEL_1: return 1;
        case HAL_TIM_ACTIVE_CHANNEL_2: return 2;
        case HAL_TIM_ACTIVE_CHANNEL_3: return 3;
        case HAL_TIM_ACTIVE_CHANNEL_4: return 4;
        case HAL_TIM_ACTIVE_CHANNEL_CLEARED: err_unreachable("No active timer channel");
    }
} */

static void _dshot_dmaCpltCallback(void* context) {
    size_t motor = (size_t) context;

    if (_act_dshot_commandBackBufferFilled[motor]) {
        for (size_t i = 0; i < _act_dshot_FRAME_BITS; i++) {
            _act_dshot_frontBuffer[motor][i] = _act_dshot_commandBackBuffer[motor][i];
        }

        _act_dshot_commandBackBufferFilled[motor] = false;
    } else if (_act_dshot_speedBackBufferFilled[motor]) {
        for (size_t i = 0; i < _act_dshot_FRAME_BITS; i++) {
            _act_dshot_frontBuffer[motor][i] = _act_dshot_speedBackBuffer[motor][i];
        }

        _act_dshot_speedBackBufferFilled[motor] = false;
    } else {
        _dshot_dmaActive[motor] = false;
        return;
    }

    HAL_TIM_PWM_Start_DMA(                               //
        _dshot_timers[motor],                            //
        _dshot_timerChannels[motor],                     //
        (const uint32_t*) _act_dshot_frontBuffer[motor], //
        utils_arrayCount(_act_dshot_frontBuffer[motor])  //
    );
}

static void _act_dshot_frameToDmaPwmSignals( //
    size_t motor,
    _act_dshot_Frame frame,
    volatile _act_dshot_TimerValue buffer[_act_dshot_FRAME_BITS] //
) {
    for (size_t bit = 0; bit < _act_dshot_FRAME_BITS; bit++) {
        size_t bitMSB = (_act_dshot_FRAME_BITS - 1) - bit;

        float us = ((frame >> bitMSB) & 1) ? act_dshot_1_T_US : act_dshot_0_T_US;
        _act_dshot_TimerValue duty = _dshot_timers[motor]->Init.Period * (us / act_dshot_BIT_T_US);

        buffer[bit] = duty;
    }
}

char* uint_buffer_to_hex_string(const uint32_t* buf, size_t len) {
    // Each byte -> 2 hex chars + space + null terminator
    size_t out_len = len * 5 + 1;
    char* out = malloc(out_len);
    if (!out)
        return NULL;

    char* p = out;
    for (size_t i = 0; i < len; i++) {
        p += sprintf(p, "%" PRIu32 " ", buf[i]);
    }

    *p = '\0';
    return out;
}

#define _act_dshot_ARMING_TIME_MS 3000

static void _dshot_output(_act_dshot_Frame frames[act_MOTOR_COUNT], bool command) {
    for (size_t i = 0; i < act_MOTOR_COUNT; i++) {
        if (_dshot_dmaActive[i]) {
            if (command) {
                if (_act_dshot_commandBackBufferFilled[i]) {
                    err_ignorable("command buffer filled");
                    continue;
                }

                __disable_irq();

                _act_dshot_frameToDmaPwmSignals(i, frames[i], _act_dshot_commandBackBuffer[i]);
                _act_dshot_commandBackBufferFilled[i] = true;

                __enable_irq();

            } else {
                __disable_irq();

                _act_dshot_frameToDmaPwmSignals(i, frames[i], _act_dshot_speedBackBuffer[i]);
                _act_dshot_speedBackBufferFilled[i] = true;

                __enable_irq();
            }
        } else {
            _act_dshot_frameToDmaPwmSignals(i, frames[i], _act_dshot_frontBuffer[i]);

            // char* s = uint_buffer_to_hex_string((const uint32_t*) _act_dshot_frontBuffer[i],
            //                                     utils_arrayCount(_act_dshot_frontBuffer[i]));
            // log_raw("mot%d: to front buffer: %s", i, s);
            // free(s);

            //
            //     }
            // }

            // for (size_t i = 0; i < act_MOTOR_COUNT; i++) {
            //     if (_dshot_dmaActive[i]) {
            //

            _dshot_dmaActive[i] = true;
            HAL_TIM_PWM_Start_DMA(                           //
                _dshot_timers[i],                            //
                _dshot_timerChannels[i],                     //
                (const uint32_t*) _act_dshot_frontBuffer[i], //
                utils_arrayCount(_act_dshot_frontBuffer[i])  //
            );
        }
    }
}

void act_dshot_outputCommand(dshot_Command cmds[act_MOTOR_COUNT]) {
    _act_dshot_Frame frames[act_MOTOR_COUNT];

    for (size_t i = 0; i < act_MOTOR_COUNT; i++) {
        frames[i] = _dshot_createCommandDshotFrame(cmds[i]);
    }

    _dshot_output(frames, true);
}

// needs to be called at least at the dshot_BIT_FREQ

void act_dshot_outputSpeed(float speeds[act_MOTOR_COUNT], act_FinalSignalTelemetry* out) {

    sys_TimeUS now = sys_time();
    bool ready = sys_timeDiff(act_getArmTime(), now) > _act_dshot_ARMING_TIME_MS * 1000;

    _act_dshot_Frame frames[act_MOTOR_COUNT];

    for (size_t i = 0; i < act_MOTOR_COUNT; i++) {
        uint16_t throttle = _act_dshot_calculateRawThrottle(ready ? speeds[i] : 0);

        if (out != nullptr) {
            out->dshotThrottles[i] = throttle - _act_dshot_MIN_RAW_THROTTLE;
        }

        frames[i] = _dshot_createDshotSpeedFrame(throttle);
    }

    _dshot_output(frames, false);
}

void act_dshot_init(TIM_HandleTypeDef* const timers[act_MOTOR_COUNT], const uint32_t timerChannels[act_MOTOR_COUNT]) {

    for (size_t i = 0; i < act_MOTOR_COUNT; i++) {
        _dshot_timerChannels[i] = timerChannels[i];
        _dshot_timers[i] = timers[i];

        _act_dshot_commandBackBufferFilled[i] = false;
        _act_dshot_speedBackBufferFilled[i] = false;
        _dshot_dmaActive[i] = false;

        // bool timerRegged = false;
        // for (size_t j = 0; j < i; j++) {
        //     if (timers[i] == timers[j]) {
        //         timerRegged = true;
        //         break;
        //     }
        // }

        // if (!timerRegged)
        irq_subscribeToIrq(irq_TIM_PWM_DMA_CPLT, _dshot_dmaCpltCallback, (void*) i, timers[i]);
    }

    act_dshot_disarm();
}

void act_dshot_arm(void) {
    float speeds[act_MOTOR_COUNT];

    for (size_t i = 0; i < act_MOTOR_COUNT; i++) {
        speeds[i] = 0.0f;
    }

    act_dshot_outputSpeed(speeds, nullptr);
}

void act_dshot_disarm(void) {
    dshot_Command cmds[act_MOTOR_COUNT];

    for (size_t i = 0; i < act_MOTOR_COUNT; i++) {
        cmds[i] = act_dshot_CMD_DISARM;
    }

    act_dshot_outputCommand(cmds);
}

#endif

#define DEFINE_SCALE_FN(name, in_start, in_end, out_start, out_end)             \
    static inline float name(float input) {                                     \
        return (input * (SCALE_FACTOR(in_start, in_end, out_start, out_end))) + \
               (SCALE_OFFSET(in_start, in_end, out_start, out_end));            \
    }

#define PWM_RANGE_MIN 1000
#define PWM_RANGE_MAX 2000

#define DSHOT_MIN_THROTTLE (48)
#define DSHOT_MAX_THROTTLE (2047)
#define DSHOT_RANGE (DSHOT_MAX_THROTTLE - DSHOT_MIN_THROTTLE)

#define SCALE_FACTOR(in_start, in_end, out_start, out_end) \
    ((float) ((out_end) - (out_start)) / (float) ((in_end) - (in_start)))

#define SCALE_OFFSET(in_start, in_end, out_start, out_end) \
    ((float) (out_start) - (SCALE_FACTOR((in_start), (in_end), (out_start), (out_end)) * (float) (in_start)))

DEFINE_SCALE_FN(scaleRangeDshotFromExternal, PWM_RANGE_MIN + 1, PWM_RANGE_MAX, DSHOT_MIN_THROTTLE, DSHOT_MAX_THROTTLE)

float scaleRangeDshotFromExternalShort(float input) {
    return input * 1999.0f / 1001.0f + 48.0f - 1999.0f / 999.0f * 1001.0f;
}
