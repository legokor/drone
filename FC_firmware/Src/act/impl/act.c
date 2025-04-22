#include "act/act.h"
#include "log/log.h"
#include "act.h"
#include "math.h"


act_Motor motor_1;
act_Motor motor_2;
act_Motor motor_3;
act_Motor motor_4;



void act_Init(TIM_HandleTypeDef * tim1,uint32_t ch1,TIM_HandleTypeDef * tim2,uint32_t ch2,TIM_HandleTypeDef * tim3,uint32_t ch3,TIM_HandleTypeDef * tim4,uint32_t ch4)
{

	//log_Debug("Initalizing act...");

    act_CreateMotor(&motor_1, tim1, ch1, 1);
    act_CreateMotor(&motor_2, tim2, ch2, 2);
    act_CreateMotor(&motor_3, tim3, ch3, 3);
    act_CreateMotor(&motor_4, tim4, ch4, 4);
}

void act_createMotor(act_Motor* motor,TIM_HandleTypeDef * timer,uint32_t channel, uint8_t number)
{

	motor->timer=timer;
	motor->channel = channel;
	motor->number=number;

	act_PwmInit(timer, channel);

	HAL_TIM_PWM_Start(motor->timer, motor->channel);

	__HAL_TIM_SET_COMPARE(motor->timer, motor->channel,PWM_min + (0 * PWM_range / 100));

}

void act_PwmInit(TIM_HandleTypeDef * timer, uint32_t channel)
{
	  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
	  TIM_MasterConfigTypeDef sMasterConfig = {0};
	  TIM_OC_InitTypeDef sConfigOC = {0};

	  //timer->Instance = timer;
	  timer->Init.Prescaler = 32-1;  //32 MHz clock
	  timer->Init.CounterMode = TIM_COUNTERMODE_UP;
	  timer->Init.Period = 20000;
	  timer->Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
	  timer->Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
	  if (HAL_TIM_Base_Init(timer) != HAL_OK)
	  {
	    Error_Handler();
	  }
	  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
	  if (HAL_TIM_ConfigClockSource(timer, &sClockSourceConfig) != HAL_OK)
	  {
	    Error_Handler();
	  }
	  if (HAL_TIM_PWM_Init(timer) != HAL_OK)
	  {
	    Error_Handler();
	  }
	  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
	  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
	  if (HAL_TIMEx_MasterConfigSynchronization(timer, &sMasterConfig) != HAL_OK)
	  {
	    Error_Handler();
	  }
	  sConfigOC.OCMode = TIM_OCMODE_PWM1;
	  sConfigOC.Pulse = 0;
	  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
	  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
	  if (HAL_TIM_PWM_ConfigChannel(timer, &sConfigOC, channel) != HAL_OK)
	  {
	    Error_Handler();
	  }
	  HAL_TIM_MspPostInit(timer);

}

void act_setMotorSpeed( uint8_t number ,uint8_t percent)
{
	act_Motor motor;

	if (number == 1) {
	    motor = motor_1;
	} else if (number == 2) {
	    motor = motor_2;
	} else if (number == 3) {
	    motor = motor_3;
	}else if (number == 4) {
	    motor = motor_4;
	} else {
		//log_Debug("not valid motor...");
	    return;
	}

	 __HAL_TIM_SET_COMPARE(motor.timer, motor.channel,PWM_min + (percent * PWM_range / 100));
	 //log_Debug("beallitva %d. motor\r\n", motor.number);
}

void act_setMMX(float thrust,float yaw,float pitch,float roll)
{
	uint32_t speed_1= fmin(1000,(thrust + yaw + pitch + roll)*1000);
	uint32_t speed_2= fmin(1000,(thrust - yaw + pitch - roll)*1000);
	uint32_t speed_3= fmin(1000,(thrust + yaw - pitch + roll)*1000);
	uint32_t speed_4= fmin(1000,(thrust - yaw - pitch - roll)*1000);

	//log_Debug("1 :%d 2 :%d 3 :%d 4 :%d\r\n",speed_1,speed_2,speed_3,speed_4);

	act_setMotorSpeed( 1,speed_1);
	act_setMotorSpeed( 2,speed_2);
	act_setMotorSpeed( 3,speed_3);
	act_setMotorSpeed( 4,speed_4);
}
