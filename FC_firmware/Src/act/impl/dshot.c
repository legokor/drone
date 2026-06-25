#include <math.h>
#include <stdio.h>
#include "config.h"
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

typedef uint16_t _act_dshot_Frame;
typedef uint32_t _act_dshot_TimerValue;

static TIM_HandleTypeDef* _dshot_timer;

static volatile bool _dshot_dmaActive = false;

// +zero at the end to keep pin at 0
static volatile __ALIGN_BEGIN _act_dshot_TimerValue
    _act_dshot_frontBuffer[act_MOTOR_COUNT * (_act_dshot_FRAME_BITS + 1)] __ALIGN_END = { 0 };

// for next frame
static volatile bool _act_dshot_speedBackBufferFilled = false;
static volatile bool _act_dshot_commandBackBufferFilled = false;
static volatile _act_dshot_TimerValue _act_dshot_speedBackBuffer[act_MOTOR_COUNT * _act_dshot_FRAME_BITS] = { 0 };
static volatile _act_dshot_TimerValue _act_dshot_commandBackBuffer[act_MOTOR_COUNT * _act_dshot_FRAME_BITS] = { 0 };

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

static void _dshot_dmaCpltCallback(void* context) {
    (void) context;

    if (_act_dshot_commandBackBufferFilled) {
        for (size_t i = 0; i < act_MOTOR_COUNT * _act_dshot_FRAME_BITS; i++) {
            _act_dshot_frontBuffer[i] = _act_dshot_commandBackBuffer[i];
        }

        _act_dshot_commandBackBufferFilled = false;
    } else if (_act_dshot_speedBackBufferFilled) {
        for (size_t i = 0; i < act_MOTOR_COUNT * _act_dshot_FRAME_BITS; i++) {
            _act_dshot_frontBuffer[i] = _act_dshot_speedBackBuffer[i];
        }

        _act_dshot_speedBackBufferFilled = false;
    } else {
        _dshot_dmaActive = false;
        return;
    }

    HAL_TIM_DMABurst_WriteStop(_dshot_timer, TIM_DMA_UPDATE);

    HAL_TIM_DMABurst_MultiWriteStart(             //
        _dshot_timer,                             //
        TIM_DMABASE_CCR1,                         //
        TIM_DMA_UPDATE,                           //
        (const uint32_t*) _act_dshot_frontBuffer, //
        TIM_DMABURSTLENGTH_4TRANSFERS,            //
        utils_arrayCount(_act_dshot_frontBuffer)  //
    );
}

static void _act_dshot_frameToDmaPwmSignals( //
    _act_dshot_Frame frames[act_MOTOR_COUNT],
    volatile _act_dshot_TimerValue* buffer //
) {
    for (size_t m = 0; m < act_MOTOR_COUNT; m++) {
        for (size_t bit = 0; bit < _act_dshot_FRAME_BITS; bit++) {
            size_t bitMSB = (_act_dshot_FRAME_BITS - 1) - bit;

            float us = ((frames[m] >> bitMSB) & 1) ? act_dshot_1_T_US : act_dshot_0_T_US;
            _act_dshot_TimerValue duty = _dshot_timer->Init.Period * (us / act_dshot_BIT_T_US);

            buffer[bit * act_MOTOR_COUNT + m] = duty;
        }
    }
}

#define _act_dshot_ARMING_TIME_MS 3000

static void _dshot_output(_act_dshot_Frame frames[act_MOTOR_COUNT], bool command) {
    if (_dshot_dmaActive) {
        if (command) {
            if (_act_dshot_commandBackBufferFilled) {
                err_ignorable("command buffer filled");
                return;
            }

            __disable_irq();

            _act_dshot_frameToDmaPwmSignals(frames, _act_dshot_commandBackBuffer);
            _act_dshot_commandBackBufferFilled = true;

            __enable_irq();
        } else {
            __disable_irq();

            _act_dshot_frameToDmaPwmSignals(frames, _act_dshot_speedBackBuffer);
            _act_dshot_speedBackBufferFilled = true;

            __enable_irq();
        }
    } else {
        _act_dshot_frameToDmaPwmSignals(frames, _act_dshot_frontBuffer);

        HAL_TIM_DMABurst_WriteStop(_dshot_timer, TIM_DMA_UPDATE);

        _dshot_dmaActive = true;
        HAL_TIM_DMABurst_MultiWriteStart(             //
            _dshot_timer,                             //
            TIM_DMABASE_CCR1,                         //
            TIM_DMA_UPDATE,                           //
            (const uint32_t*) _act_dshot_frontBuffer, //
            TIM_DMABURSTLENGTH_4TRANSFERS,            // TODO: motor count, timer channel count...?
            utils_arrayCount(_act_dshot_frontBuffer)  //
        );
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

// TODO: document: has to be given timer with 4 channels

void act_dshot_init(TIM_HandleTypeDef* timer) {
    _dshot_timer = timer;

    _act_dshot_commandBackBufferFilled = false;
    _act_dshot_speedBackBufferFilled = false;
    _dshot_dmaActive = false;

    memset((void*) _act_dshot_frontBuffer, 0, sizeof(_act_dshot_frontBuffer));

    irq_subscribeToIrq(irq_TIM_PERIOD_ELAPSED, _dshot_dmaCpltCallback, nullptr, timer);
    HAL_TIM_PWM_Start(_dshot_timer, TIM_CHANNEL_1);
    HAL_TIM_PWM_Start(_dshot_timer, TIM_CHANNEL_2);
    HAL_TIM_PWM_Start(_dshot_timer, TIM_CHANNEL_3);
    HAL_TIM_PWM_Start(_dshot_timer, TIM_CHANNEL_4);

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
