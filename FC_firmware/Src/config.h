#ifndef CONFIG_H
#define CONFIG_H

#define config_LOG_TOPIC 0

#define config_ACT_FREQ 50

#define config_LLC_ROLL_PID_CONSTS 0, 0, 0
#define config_LLC_PITCH_PID_CONSTS 0, 0, 0
#define config_LLC_YAW_PID_CONSTS 0, 0, 0

// TODO: arm channel
#define config_RC_CHAN_ARM 6
#define config_RC_CHAN_ROLL 0
#define config_RC_CHAN_PITCH 1
#define config_RC_CHAN_YAW 3
#define config_RC_CHAN_THRUST 2

#define config_NO_RC_DISARM_MS 200

#define config_BATTERY_CELL_COUNT 4
#define config_MIN_BATTERY_CELL_VOLTAGE 3.3f
#define config_BATTERY_CRITICAL_TIME_MS 1000
#define config_BATTERY_ADC_CONVERSION_FACTOR 7.68e-3 // (1 / 130.0f)
#define config_ESC_ADC_CONVERSION_FACTOR 1           // TODO: esc adc conversion factor

#endif // CONFIG_H
