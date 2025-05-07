#include "act/act.h"
#include "log/log.h"

#include <stdbool.h>
#include "arm_math.h"

static bool act_Armed = false;

typedef struct {
    TIM_HandleTypeDef* timer;
    uint32_t channel;
} act_Motor;

static act_Motor act_Motors[4];

static void act_InitMotor(act_Motor* motor, TIM_HandleTypeDef* timer, uint32_t channel);
static void act_InitPwm(TIM_HandleTypeDef* timer, uint32_t channel);

void act_Init(TIM_HandleTypeDef* timers[act_MOTOR_COUNT], uint32_t* channels[act_MOTOR_COUNT]) {
    // log_Debug("Initalizing act...");

    for (uint i = 0; i < act_MOTOR_COUNT; i++) {
        act_InitMotor(&act_Motors[i], timers[i], channels[i]);
    }
}

static void act_InitMotor(act_Motor* motor, TIM_HandleTypeDef* timer, uint32_t channel) {
    motor->channel = channel;
    motor->timer = timer;

    act_InitPwm(timer, channel);

    __HAL_TIM_SET_COMPARE(motor->timer, motor->channel, act_PWM_MIN + (0 * act_PWM_RANGE / 100));
    HAL_TIM_PWM_Start(motor->timer, motor->channel);
}

static void act_InitPwm(TIM_HandleTypeDef* timer, uint32_t channel) {

    TIM_ClockConfigTypeDef sClockSourceConfig = { 0 };
    TIM_MasterConfigTypeDef sMasterConfig = { 0 };
    TIM_OC_InitTypeDef sConfigOC = { 0 };

    // timer->Instance = timer;
    timer->Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
    timer->Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
    timer->Init.CounterMode = TIM_COUNTERMODE_UP;
    timer->Init.Period = act_PWM_RANGE * 10;

    // TODO: this is for 32 mhz clock source
    timer->Init.Prescaler = 32 - 1;

    if (HAL_TIM_Base_Init(timer) != HAL_OK) {
        Error_Handler();
    }

    sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
    if (HAL_TIM_ConfigClockSource(timer, &sClockSourceConfig) != HAL_OK) {
        Error_Handler();
    }

    if (HAL_TIM_PWM_Init(timer) != HAL_OK) {
        Error_Handler();
    }

    sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
    sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;

    if (HAL_TIMEx_MasterConfigSynchronization(timer, &sMasterConfig) != HAL_OK) {
        Error_Handler();
    }

    sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
    sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
    sConfigOC.OCMode = TIM_OCMODE_PWM1;
    sConfigOC.Pulse = 0;

    if (HAL_TIM_PWM_ConfigChannel(timer, &sConfigOC, channel) != HAL_OK) {
        Error_Handler();
    }

    HAL_TIM_MspPostInit(timer);
}

static void act_SetMotorSpeed(uint8_t idx, uint16_t percent) {
    assert(idx < act_MOTOR_COUNT);

    act_Motor motor = act_Motors[idx];
    __HAL_TIM_SET_COMPARE(motor.timer, motor.channel, act_PWM_MIN + (percent * act_PWM_RANGE / 100));

    // log_Debug("beallitva %d. motor\r\n", motor.number);
}

void act_SetMMX(float thrust, float yaw, float pitch, float roll) {
    assert(act_MOTOR_COUNT == 4);

    if (!act_Armed) {
        act_Disarm();
        return;
    }

    float speed[act_MOTOR_COUNT] = {
        thrust + yaw + pitch + roll, //
        thrust - yaw + pitch - roll, //
        thrust + yaw - pitch + roll, //
        thrust - yaw - pitch - roll  //
    };

    float tmp[act_MOTOR_COUNT];

    arm_scale_f32(speed, act_PWM_RANGE, tmp, act_MOTOR_COUNT);
    arm_clip_f32(tmp, speed, act_PWM_MIN, act_PWM_MAX, act_MOTOR_COUNT);

    for (int i = 0; i < act_MOTOR_COUNT; i++) {
        act_SetMotorSpeed(i, speed[i]);
    }

    // log_Debug("1:%d 2:%d 3:%d 4:%d\n", speed[0], speed[1], speed[2], speed[3]);
}

void act_Arm(void) {
    assert_param(act_Armed == false);

    act_Armed = true;
}

void act_Disarm(void) {
    for (int i = 0; i < act_MOTOR_COUNT; i++) {
        act_SetMotorSpeed(i, 0);
    }

    assert_param(act_Armed == false);
    act_Armed = false;
}
