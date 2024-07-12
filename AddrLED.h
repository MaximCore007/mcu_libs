#ifndef ADDRESSABLELED_ADDRLED_H_
#define ADDRESSABLELED_ADDRLED_H_

#include "main.h"

#define FPU

#define NUM_PIXELS 12	///< Pixel quantity

#define USE_GAMMA_CORRECTION 0	///< Gamma-correction should fix red&green, try for yourself

#define APB1
#define TIM_NUM	   17	///< Timer number
#define TIM_CH	   TIM_CHANNEL_1  ///< Timer's PWM channel
#define DMA_HANDLE hdma_tim17_ch1_up  ///< DMA Channel
//#define DMA_SIZE_WORD
//#define DMA_SIZE_HWORD
#define DMA_SIZE_BYTE

typedef enum ALED_Status {
	ALED_READY = HAL_TIM_STATE_READY,
	ALED_BUSY = HAL_TIM_STATE_BUSY,
	ALED_ERROR = HAL_TIM_STATE_ERROR,
}ALDED_Status_t;

void ALED_Init(void);
ALDED_Status_t ALED_Show(void);
ALDED_Status_t ALED_Status(void);
void ALED_SetBrightness(uint8_t brigtness);
void ALED_SetRGB(uint16_t led, uint8_t red, uint8_t green, uint8_t blue);
void ALED_FillRGB(uint8_t red, uint8_t green, uint8_t blue);
void ALED_SetHSV(uint16_t led, uint16_t hue, uint8_t sat, uint8_t val);
void ALED_FillHSV(uint16_t hue, uint8_t sat, uint8_t val);
void ALED_SetWhite(uint16_t led, uint8_t white);
void ALED_FillWhite(uint8_t white);
void ALED_FillOFF(void);

#endif /* ADDRESSABLELED_ADDRLED_H_ */
