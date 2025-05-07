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

void act_Init(TIM_HandleTypeDef* timers[act_MOTOR_COUNT], uint32_t* channels[act_MOTOR_COUNT]) {
    // log_Debug("Initalizing act...");

    for (uint i = 0; i < act_MOTOR_COUNT; i++) {
        act_InitMotor(&act_Motors[i], timers[i], channels[i]);
    }
}

static void act_InitMotor(act_Motor* motor, TIM_HandleTypeDef* timer, uint32_t channel) {
    motor->channel = channel;
    motor->timer = timer;

    __HAL_TIM_SET_COMPARE(motor->timer, motor->channel, act_PWM_MIN + (0 * act_PWM_RANGE / 100));
    HAL_TIM_PWM_Start(motor->timer, motor->channel);
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
