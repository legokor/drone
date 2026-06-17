#ifndef DSHOT_H
#define DSHOT_H

#include "act/act.h"

#if config_DSHOT_MODE == 150

#define dshot_T1H 5.00
#define dshot_T0H 2.50
#define dshot_TBIT 6.67

#elif config_DSHOT_MODE == 300

#define dshot_TBIT 3.33
#define dshot_T1H 2.50
#define dshot_T0H 1.25

#elif config_DSHOT_MODE == 600

#define dshot_TBIT 1.67
#define dshot_T1H 1.25
#define dshot_T0H 0.625

#elif config_DSHOT_MODE == 1200

#define dshot_TBIT 0.83
#define dshot_T1H 0.625
#define dshot_T0H 0.313

#elif config_MODE != config_DSHOT_MODE

#error "config_DSHOT_MODE invalid, valid values are 150, 300, 600, and 1200"

#else // for auto complete

#define dshot_TBIT 0
#define dshot_T1H 0
#define dshot_T0H 0

#endif

#define dshot_BIT_FREQ (1.0f / dshot_TBIT)

// https://betaflight.com/docs/development/API/Dshot#special-commands
typedef enum act_DShotCommand {
    DSHOT_CMD_DISARM = 0,

    // Wait at least length of beep (260ms) before next command
    DSHOT_CMD_BEEP1 = 1,

    // Wait at least length of beep (260ms) before next command
    DSHOT_CMD_BEEP2 = 2,

    // Wait at least length of beep (260ms) before next command
    DSHOT_CMD_BEEP3 = 3,

    // Wait at least length of beep (260ms) before next command
    DSHOT_CMD_BEEP4 = 4,

    // Wait at least length of beep (260ms) before next command
    DSHOT_CMD_BEEP5 = 5,

    // Wait at least 12ms before next command
    DSHOT_CMD_ESC_INFO = 6,

    // Need 6x
    DSHOT_CMD_SPIN_DIRECTION_1 = 7,

    // Need 6x
    DSHOT_CMD_SPIN_DIRECTION_2 = 8,

    // Need 6x
    DSHOT_CMD_3D_MODE_OFF = 9,

    // Need 6x
    DSHOT_CMD_3D_MODE_ON = 10,

    // Need 6x, wait at least 35ms before next command
    DSHOT_CMD_SAVE_SETTINGS = 12,

    // Need 6x
    DSHOT_CMD_SPIN_DIRECTION_NORMAL = 20,

    // Need 6x
    DSHOT_CMD_SPIN_DIRECTION_REVERSED = 21,

    DSHOT_CMD_LED0_ON = 22,
    DSHOT_CMD_LED1_ON = 23,
    DSHOT_CMD_LED2_ON = 24,
    DSHOT_CMD_LED3_ON = 25,
    DSHOT_CMD_LED0_OFF = 26,
    DSHOT_CMD_LED1_OFF = 27,
    DSHOT_CMD_LED2_OFF = 28,
    DSHOT_CMD_LED3_OFF = 29,

    // mode on/Off
    Audio_Stream = 30,

    // Mode on/Off
    Silent = 31,
} dshot_Command;

void dshot_init(TIM_HandleTypeDef* timers[act_MOTOR_COUNT], uint32_t timerChannels[act_MOTOR_COUNT]);

void dshot_outputSpeed(float speeds[act_MOTOR_COUNT]);
void dshot_outputCommand(dshot_Command cmds[act_MOTOR_COUNT]);

void dshot_arm(void);
void dshot_disarm(void);

#endif // DSHOT_H
