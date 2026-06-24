#ifndef DSHOT_H
#define DSHOT_H

#include "act/act.h"
#include "config.h"

#if config_DSHOT_MODE == 150

#define act_dshot_1_T_US 5.00f
#define act_dshot_0_T_US 2.50f
#define act_dshot_BIT_T_US 6.67f

#elif config_DSHOT_MODE == 300

#define act_dshot_BIT_T_US 3.33f
#define act_dshot_1_T_US 2.50f
#define act_dshot_0_T_US 1.25f

#elif config_DSHOT_MODE == 600

#define act_dshot_BIT_T_US 1.67f
#define act_dshot_1_T_US 1.25f
#define act_dshot_0_T_US 0.625f

#elif config_DSHOT_MODE == 1200

#define act_dshot_BIT_T_US 0.83f
#define act_dshot_1_T_US 0.625f
#define act_dshot_0_T_US 0.313f

#elif config_MODE != config_DSHOT_MODE

#error "config_DSHOT_MODE invalid, valid values are 150, 300, 600, and 1200"

#else // for auto complete

#define act_dshot_BIT_T_US 0.0f
#define act_dshot_1_T_US 0.0f
#define act_dshot_0_T_US 0.0f

#endif

#define dshot_BIT_FREQ (1.0f / act_dshot_BIT_T_US)

// https://betaflight.com/docs/development/API/Dshot#special-commands
typedef enum act_dshot_Command {
    act_dshot_CMD_DISARM = 0,

    // Wait at least length of beep (260ms) before next command
    act_dshot_CMD_BEEP1 = 1,

    // Wait at least length of beep (260ms) before next command
    act_dshot_CMD_BEEP2 = 2,

    // Wait at least length of beep (260ms) before next command
    act_dshot_CMD_BEEP3 = 3,

    // Wait at least length of beep (260ms) before next command
    act_dshot_CMD_BEEP4 = 4,

    // Wait at least length of beep (260ms) before next command
    act_dshot_CMD_BEEP5 = 5,

    // Wait at least 12ms before next command
    act_dshot_CMD_ESC_INFO = 6,

    // Need 6x
    act_dshot_CMD_SPIN_DIRECTION_1 = 7,

    // Need 6x
    act_dshot_CMD_SPIN_DIRECTION_2 = 8,

    // Need 6x
    act_dshot_CMD_3D_MODE_OFF = 9,

    // Need 6x
    act_dshot_CMD_3D_MODE_ON = 10,

    // Need 6x, wait at least 35ms before next command
    act_dshot_CMD_SAVE_SETTINGS = 12,

    // Need 6x
    act_dshot_CMD_SPIN_DIRECTION_NORMAL = 20,

    // Need 6x
    act_dshot_CMD_SPIN_DIRECTION_REVERSED = 21,

    act_dshot_CMD_LED0_ON = 22,
    act_dshot_CMD_LED1_ON = 23,
    act_dshot_CMD_LED2_ON = 24,
    act_dshot_CMD_LED3_ON = 25,
    act_dshot_CMD_LED0_OFF = 26,
    act_dshot_CMD_LED1_OFF = 27,
    act_dshot_CMD_LED2_OFF = 28,
    act_dshot_CMD_LED3_OFF = 29,

    // mode on/Off
    act_dshot_CMD_AUDIO_STREAM = 30,

    // Mode on/Off
    act_dshot_CMD_SILENT = 31,
} dshot_Command;

void act_dshot_init(TIM_HandleTypeDef* const timers[act_MOTOR_COUNT], const uint32_t timerChannels[act_MOTOR_COUNT]);

void act_dshot_outputSpeed(float speeds[act_MOTOR_COUNT], act_FinalSignalTelemetry* out);
void act_dshot_outputCommand(dshot_Command cmds[act_MOTOR_COUNT]);

void act_dshot_arm(void);
void act_dshot_disarm(void);

#endif // DSHOT_H
