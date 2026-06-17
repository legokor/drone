#include "act/impl/dshot.h"
#include "act/act.h"
#include "err/err.h"
#include "irq/irq.h"
#include "utils/utils.h"

#include "stm32f4xx_hal_tim.h"

#include <stddef.h>
#include <string.h>

typedef uint16_t _act_dshotFrame;

static TIM_HandleTypeDef* _dshot_timers[act_MOTOR_COUNT];
static uint32_t _dshot_timerChannels[act_MOTOR_COUNT];

#define _dshot_FRAME_BITS 16

// for DMA
static volatile bool _dshot_dmaActive[act_MOTOR_COUNT] = { false };
static uint16_t _act_dshotDmaBufferFront[act_MOTOR_COUNT][_dshot_FRAME_BITS];

// for next frame
static volatile bool _dshot_speedBackBufferFilled[act_MOTOR_COUNT] = { false };
static volatile bool _dshot_commandBackBufferFilled[act_MOTOR_COUNT] = { false };
static uint16_t _act_dshotDmaSpeedBufferBack[act_MOTOR_COUNT][_dshot_FRAME_BITS];
static uint16_t _act_dshotDmaCommandBufferBack[act_MOTOR_COUNT][_dshot_FRAME_BITS];

static _act_dshotFrame _dshot_createDshotFrameFromRaw(_act_dshotFrame throttleRaw) {
    bool telemetry = 0;
    _act_dshotFrame frame = throttleRaw | ((telemetry ? 1 : 0) << 11);

    _act_dshotFrame crc = (frame ^ (frame >> 4) ^ (frame >> 8)) & 0x0F;
    frame |= crc << 12;

    return frame;
}

static _act_dshotFrame _dshot_createDshotSpeedFrame(float motor) {
    return _dshot_createDshotFrameFromRaw(48 + utils_min(motor * 2000, 2000));
}

static _act_dshotFrame _dshot_createCommandDshotFrame(dshot_Command cmd) {
    return _dshot_createDshotFrameFromRaw(cmd);
}

static void _dshot_dmaCpltCallback(void* context) {
    size_t motor = (size_t) context;

    if (_dshot_commandBackBufferFilled[motor]) {
        memcpy(_act_dshotDmaBufferFront, _act_dshotDmaCommandBufferBack[motor], _dshot_FRAME_BITS);
        _dshot_speedBackBufferFilled[motor] = false;
    } else if (_dshot_speedBackBufferFilled[motor]) {
        memcpy(_act_dshotDmaBufferFront, _act_dshotDmaSpeedBufferBack[motor], _dshot_FRAME_BITS);
        _dshot_commandBackBufferFilled[motor] = false;
    } else {
        _dshot_dmaActive[motor] = false;
        return;
    }

    _dshot_dmaActive[motor] = true;

    HAL_TIM_PWM_Start_DMA(                                 //
        _dshot_timers[motor],                              //
        _dshot_timerChannels[motor],                       //
        (const uint32_t*) _act_dshotDmaBufferFront[motor], //
        _dshot_FRAME_BITS                                  //
    );
}

static void _dshot_dshotFrameToDmaPwmSignals(_act_dshotFrame frame, uint16_t buffer[_dshot_FRAME_BITS]) {
    for (size_t b = 0; b < _dshot_FRAME_BITS; b++) {
        buffer[b] = ((frame >> b) & 1) ? dshot_T1H : dshot_T0H;
    }
}

static void _dshot_output(_act_dshotFrame frames[act_MOTOR_COUNT], bool command) {
    for (size_t i = 0; i < act_MOTOR_COUNT; i++) {
        if (_dshot_dmaActive[i]) {
            if (command) {
                if (_dshot_commandBackBufferFilled[i]) {
                    err_ignorable("command buffer filled");
                    continue;
                }

                _dshot_dshotFrameToDmaPwmSignals(frames[i], _act_dshotDmaCommandBufferBack[i]);
                _dshot_commandBackBufferFilled[i] = true;
            } else {
                _dshot_dshotFrameToDmaPwmSignals(frames[i], _act_dshotDmaSpeedBufferBack[i]);
                _dshot_speedBackBufferFilled[i] = true;
            }
        } else {
            _dshot_dshotFrameToDmaPwmSignals(frames[i], _act_dshotDmaBufferFront[i]);

            _dshot_dmaActive[i] = true;
            HAL_TIM_PWM_Start_DMA(                             //
                _dshot_timers[i],                              //
                _dshot_timerChannels[i],                       //
                (const uint32_t*) _act_dshotDmaBufferFront[i], //
                utils_arrayCount(_act_dshotDmaBufferFront[i])  //
            );
        }
    }
}

void dshot_outputCommand(dshot_Command cmds[act_MOTOR_COUNT]) {
    _act_dshotFrame frames[act_MOTOR_COUNT];

    for (size_t i = 0; i < act_MOTOR_COUNT; i++) {
        frames[i] = _dshot_createCommandDshotFrame(cmds[i]);
    }

    _dshot_output(frames, true);
}

// needs to be called at least at the dshot_BIT_FREQ

void dshot_outputSpeed(float speeds[act_MOTOR_COUNT]) {
    _act_dshotFrame frames[act_MOTOR_COUNT];
    err_assert(!act_isArmed());

    for (size_t i = 0; i < act_MOTOR_COUNT; i++) {
        frames[i] = _dshot_createDshotSpeedFrame(speeds[i]);
    }

    _dshot_output(frames, false);
}

void dshot_init(TIM_HandleTypeDef* timers[act_MOTOR_COUNT], uint32_t timerChannels[act_MOTOR_COUNT]) {
    dshot_Command cmds[act_MOTOR_COUNT];

    for (size_t i = 0; i < act_MOTOR_COUNT; i++) {
        irq_subscribeToIrq(irq_TIM_PWM_DMA_CPLT, _dshot_dmaCpltCallback, (void*) i, timers[i]);

        _dshot_timerChannels[i] = timerChannels[i];
        _dshot_timers[i] = timers[i];

        cmds[i] = DSHOT_CMD_DISARM;
    }

    dshot_outputCommand(cmds);
}

void dshot_arm(void) {
    float speeds[act_MOTOR_COUNT];

    for (size_t i = 0; i < act_MOTOR_COUNT; i++) {
        speeds[i] = 0;
    }

    dshot_outputSpeed(speeds);
}

void dshot_disarm(void) {
    dshot_Command cmds[act_MOTOR_COUNT];

    for (size_t i = 0; i < act_MOTOR_COUNT; i++) {
        cmds[i] = DSHOT_CMD_DISARM;
    }

    dshot_outputCommand(cmds);
}
