#include "addr_leds.h"
//#include "tim.h"

extern TIM_HandleTypeDef htim17;

#define LOG_1 LOG_HIGH_TIME_SLOT
#define LOG_0 LOG_LOW_TIME_SLOT
#define RESET RESET_TIME_SLOT

uint8_t rgb_data_buff[NUMBER_OF_LEDS][3];

#define PWM_BUFF_SIZE (BITS_PER_LED * 2)
//#define PWM_BUFF_SIZE ((BITS_PER_LED * NUMBER_OF_LEDS) + RESET)
uint8_t pwm_buff[PWM_BUFF_SIZE] = {0};

uint8_t curr_led;	// current led
uint8_t start;
uint8_t stop;

static uint32_t get_color(uint8_t index)
{
	return (uint32_t)(rgb_data_buff[curr_led][LED_G] << 12
			| rgb_data_buff[curr_led][LED_R] << 8
			| rgb_data_buff[curr_led][LED_B]);
}

static void set_pwm_data(uint16_t offset_buff)
{
	uint32_t color = get_color(curr_led++);
	for (int8_t i = (BITS_PER_LED - 1); i >= 0; i--) {
		pwm_buff[i + offset_buff] = (color & (1 << i)) ?  LOG_1 : LOG_0;
	}
}

static void set_pwm_zero(void)
{
	for (int8_t i = 0; i < PWM_BUFF_SIZE; i++) {
		pwm_buff[i] = 0;
	}
}

void set_color(uint8_t led, uint8_t r, uint8_t g, uint8_t b)
{
	rgb_data_buff[led][LED_R] = r;
	rgb_data_buff[led][LED_G] = g;
	rgb_data_buff[led][LED_B] = b;
}

int8_t leds_update(void)
{
	if (HAL_TIM_PWM_GetState(&htim17) != HAL_TIM_STATE_READY)
		return -1;

	curr_led = 0;
	start = 0;
	stop = 0;

    // Set initial LED colors
//	set_pwm_data();
	set_pwm_zero();

	__HAL_TIM_SET_COUNTER(&htim17, 0);
	HAL_TIM_Base_Stop_DMA(&htim17);
	HAL_TIM_PWM_Start_DMA(&htim17, TIM_CHANNEL_1, (uint32_t*)pwm_buff, PWM_BUFF_SIZE);

	return 0;
}

void leds_init(void)
{

}

void pwm_set_data_it(uint16_t offset)
{
	if (start < 2) {
		set_pwm_zero();
		start++;
	}
	else if (curr_led < NUMBER_OF_LEDS) {
		set_pwm_data(offset);
	}
	else if (curr_led >= NUMBER_OF_LEDS && stop < 4) {
		set_pwm_zero();
		stop++;
	}
	else {
		HAL_TIM_PWM_Stop_DMA(&htim17, TIM_CHANNEL_1);
	}
}

void HAL_TIM_PWM_PulseFinishedCallback(TIM_HandleTypeDef *htim)
{
	if (htim->Instance == TIM17
			&& htim->Channel == HAL_TIM_ACTIVE_CHANNEL_1)
	{
		pwm_set_data_it(PWM_BUFF_SIZE / 2);
	}
}

void HAL_TIM_PWM_PulseFinishedHalfCpltCallback(TIM_HandleTypeDef *htim)
{
	if (htim->Instance == TIM17
			&& htim->Channel == HAL_TIM_ACTIVE_CHANNEL_1)
	{
		pwm_set_data_it(0);
	}
}

/* EOF ************************************************************************/
