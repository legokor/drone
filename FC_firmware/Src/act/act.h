#ifndef ACT_H
#define ACT_H

#include "tim.h"

#define PWM_min 1000    //timerek counter period 20000
#define PWM_range 1000


typedef struct {
    TIM_HandleTypeDef* timer;
    uint32_t channel;
    uint8_t number;
} act_Motor;


void act_Init(TIM_HandleTypeDef * tim1,uint32_t ch1,TIM_HandleTypeDef * tim2,uint32_t ch2,TIM_HandleTypeDef * tim3,uint32_t ch3,TIM_HandleTypeDef * tim4,uint32_t ch4);
void act_CreateMotor(act_Motor* motor,TIM_HandleTypeDef * timer,uint32_t channel,uint8_t number);
void act_PwmInit(TIM_HandleTypeDef * timer, uint32_t channel);
void act_SetMotorSpeed( uint8_t number ,uint8_t percent);
void act_SetMMX(float thrust,float yaw,float pitch,float roll);


#endif // ACT_H
