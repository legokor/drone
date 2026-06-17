#ifndef CONFIG_H
#define CONFIG_H

#define config_LOG_TOPIC 0

// ACT
#define config_ACT_MODE_PWM 1
#define config_ACT_MODE_DSHOT 2
#define config_ACT_MODE config_ACT_MODE_DSHOT

#define config_ACT_LOOP_FREQ 800

#if config_ACT_MODE == config_ACT_MODE_PWM

// #define config_ACT_NO_PWM_ON_DISARM

#elif config_ACT_MODE == config_ACT_MODE_DSHOT

#define config_DSHOT_MODE 300

#endif
// ACT

// LLC
#define config_LLC_ROLL_PID_CONSTS 0.18f, 0, 0
#define config_LLC_PITCH_PID_CONSTS 0.18f, 0, 0
#define config_LLC_YAW_PID_CONSTS 0, 0, 0
// LLC

// RC
#define config_RC_CHAN_ARM 6
#define config_RC_CHAN_ROLL 0
#define config_RC_CHAN_PITCH 1
#define config_RC_CHAN_YAW 3
#define config_RC_CHAN_THRUST 2

#define config_RC_IDLE_DISARM_MS 200
// RC

#define config_BATTERY_CELL_COUNT 4
#define config_MIN_BATTERY_CELL_VOLTAGE 3.3f
#define config_BATTERY_CRITICAL_TIME_MS 1000
#define config_BATTERY_ADC_CONVERSION_FACTOR 7.68e-3 // (1 / 130.0f)
#define config_ESC_ADC_CONVERSION_FACTOR 1           // TODO: esc adc conversion factor

#endif // CONFIG_H
