#include "AddrLED.h"
#include "string.h"

#ifndef FPU
#include "utility.h"
#else
#include "math.h"
#endif

/// Timer handler
#if TIM_NUM == 1
#define TIM_HANDLE  htim1
#elif TIM_NUM == 2
#define TIM_HANDLE  htim2
#elif TIM_NUM == 3
#define TIM_HANDLE  htim3
#elif TIM_NUM == 4
#define TIM_HANDLE  htim4
#elif TIM_NUM == 5
#define TIM_HANDLE  htim5
#elif TIM_NUM == 8
#define TIM_HANDLE  htim8
#elif TIM_NUM == 17
#define TIM_HANDLE  htim17
#else
#error Wrong timer! Fix it in ARGB.h string 41
#warning If you shure, set TIM_HANDLE and APB ring by yourself
#endif

/// DMA Size
#if defined(DMA_SIZE_BYTE)
typedef uint8_t dma_siz;
#elif defined(DMA_SIZE_HWORD)
typedef uint16_t dma_siz;
#elif defined(DMA_SIZE_WORD)
typedef uint32_t dma_siz;
#endif

typedef uint8_t rgb_siz;

extern TIM_HandleTypeDef (TIM_HANDLE);  ///< Timer handler
extern DMA_HandleTypeDef (DMA_HANDLE);  ///< DMA handler

uint8_t PWM_HI;    ///< PWM Code HI Log.1 period
uint8_t PWM_LO;    ///< PWM Code LO Log.1 period

#ifdef SK6812
#define NUM_BYTES (4 * NUM_PIXELS) ///< Strip size in bytes
#define PWM_BUF_LEN (4 * 8 * 2)    ///< Pack len * 8 bit * 2 LEDs
#else
#define RGB_BUF_LEN (3 * NUM_PIXELS) ///< Strip size in bytes
#define PWM_BUF_LEN (3 * 8 * 2)    ///< Pack len * 8 bit * 2 LEDs
#endif

/// Static LED buffer
rgb_siz RGB_BUF[RGB_BUF_LEN] = {0,};

/// Timer PWM value buffer
dma_siz PWM_BUF[PWM_BUF_LEN] = {0,};
/// PWM buffer iterator
uint16_t BUF_COUNTER = 0;

uint8_t RGB_BR = 255;     ///< LED Global brightness


static void ALED_TIM_PWM_Stop(TIM_HandleTypeDef *htim)
{
	/* Disable the TIM Capture/Compare 1 DMA request */
	__HAL_TIM_DISABLE_DMA(htim, TIM_DMA_CC1);
	(void)HAL_DMA_Abort_IT(htim->hdma[TIM_DMA_ID_CC1]);

	/* Disable the Capture compare channel */
	//TIM_CCxChannelCmd(htim->Instance, TIM_CH, TIM_CCx_DISABLE);

//	if (IS_TIM_BREAK_INSTANCE(htim->Instance) != RESET)
//	{
//	  /* Disable the Main Output */
//	  __HAL_TIM_MOE_DISABLE(htim);
//	}

	/* Disable the Peripheral */
	__HAL_TIM_DISABLE(htim);

	/* Set the TIM channel state */
	TIM_CHANNEL_STATE_SET(htim, TIM_CH, HAL_TIM_CHANNEL_STATE_READY);
}


/* Callbacks functions ------------------------------------------------------ */
//void ALED_TIM_PWM_PulseFinishedCallback(DMA_HandleTypeDef *hdma)
void HAL_TIM_PWM_PulseFinishedCallback(TIM_HandleTypeDef *htim)
{
	if (htim->Instance == TIM17) {
		if (BUF_COUNTER < NUM_PIXELS) {
			// fill second part of buffer
			for (uint8_t i = 0; i < 8; i++) {
				PWM_BUF[i + 24] = (((RGB_BUF[3 * BUF_COUNTER] << i) & 0x80) > 0) ? PWM_HI : PWM_LO;
				PWM_BUF[i + 32] = (((RGB_BUF[3 * BUF_COUNTER + 1] << i) & 0x80) > 0) ? PWM_HI : PWM_LO;
				PWM_BUF[i + 40] = (((RGB_BUF[3 * BUF_COUNTER + 2] << i) & 0x80) > 0) ? PWM_HI : PWM_LO;
			}
			BUF_COUNTER++;
		} else if (BUF_COUNTER < NUM_PIXELS + 2) { // if RET transfer
			memset((dma_siz *) &PWM_BUF[PWM_BUF_LEN / 2], 0, (PWM_BUF_LEN / 2)*sizeof(dma_siz)); // second part
			BUF_COUNTER++;
		} else {
			BUF_COUNTER = 0;
//	        HAL_TIM_PWM_Stop_DMA(&TIM_HANDLE, TIM_CH);
			ALED_TIM_PWM_Stop(htim);
		}
	}
}

//void ALED_TIM_PWM_PulseFinishedHalfCpltCallback(TIM_HandleTypeDef *htim)
void HAL_TIM_PWM_PulseFinishedHalfCpltCallback(TIM_HandleTypeDef *htim)
{
	if (htim->Instance == TIM17) {
		if (BUF_COUNTER == 0) {
			return;
		}
		else if (BUF_COUNTER < NUM_PIXELS) {
			// fill first part of buffer
			for (uint8_t i = 0; i < 8; i++) {
				PWM_BUF[i] = (((RGB_BUF[3 * BUF_COUNTER] << i) & 0x80) > 0) ? PWM_HI : PWM_LO;
				PWM_BUF[i + 8] = (((RGB_BUF[3 * BUF_COUNTER + 1] << i) & 0x80) > 0) ? PWM_HI : PWM_LO;
				PWM_BUF[i + 16] = (((RGB_BUF[3 * BUF_COUNTER + 2] << i) & 0x80) > 0) ? PWM_HI : PWM_LO;
			}
			BUF_COUNTER++;
		} else if (BUF_COUNTER < NUM_PIXELS + 2) { // if RET transfer
			memset((dma_siz *) &PWM_BUF[0], 0, (PWM_BUF_LEN / 2)*sizeof(dma_siz)); // first part
			BUF_COUNTER++;
		}
	}
}

/* Private definition functions --------------------------------------------- */
/**
 * @brief Private method for gamma correction
 * @param[in] x Param to scale
 * @param[in] scale Scale coefficient
 * @return Scaled value
 */
static inline uint8_t scale8(uint8_t x, uint8_t scale)
{
    return ((uint16_t) x * scale) >> 8;
}

/**
 * @brief Convert color in HSV to RGB
 * @param[in] hue HUE (color) [0..360]
 * @param[in] sat Saturation  [0..100]
 * @param[in] val Value (brightness) [0..100]
 * @param[out] _r Pointer to RED component value
 * @param[out] _g Pointer to GREEN component value
 * @param[out] _b Pointer to BLUE component value
 */
static void HSV2RGB(uint16_t hue, uint8_t sat, uint8_t val, uint8_t *_r, uint8_t *_g, uint8_t *_b)
{
    if (sat == 0) { // if white color
        *_r = *_g = *_b = val;
        return;
    }
#ifndef FPU
    fixp_t h = fix_division(fix_int2fixed(hue) , fix_int2fixed(360));
    fixp_t s = fix_division(fix_int2fixed(sat) , fix_int2fixed(100));
    fixp_t v = fix_division(fix_int2fixed(val) , fix_int2fixed(100));

    fixp_t h6 = fix_multiply(h, fix_int2fixed(6));
    fixp_t fx_i = fix_floor(h6);
    fixp_t f = h6 - fx_i;

    fixp_t fx_1 = fix_int2fixed(1);
    fixp_t fx_255 = fix_int2fixed(255);
    uint8_t p = (uint8_t)fix_fixed2int( fix_multiply(fix_multiply(v , (fx_1 - s)) , fx_255) );
    uint8_t q = (uint8_t)fix_fixed2int( fix_multiply(fix_multiply(v , (fx_1 - fix_multiply(f , s))) , fx_255) );
    uint8_t t = (uint8_t)fix_fixed2int( fix_multiply(fix_multiply(v , (fx_1 - fix_multiply((fx_1 - f) , s))) , fx_255) );
    int32_t i = fix_fixed2int(fx_i);
#else
    /*
     * Float is smoother but check for FPU (Floating point unit) in your MCU
     * Otherwise it will take longer time in the code
     * FPU is in: F3/L3 and greater
     * Src: https://github.com/Inseckto/HSV-to-RGB
     */
    float h = (float)hue / 360;
    float s = (float)sat / 100;
    float v = (float)val / 100;

    int i = (int)floorf(h * 6);
    float f = h * 6 - (float)i;
    uint8_t p = (uint8_t)(v * (1.f - s) * 255.f);
    uint8_t q = (uint8_t)(v * (1.f - f * s) * 255.f);
    uint8_t t = (uint8_t)(v * (1.f - (1.f - f) * s) * 255.f);
#endif
    switch (i % 6) {
 	 /* Src: https://stackoverflow.com/questions/3018313 */
//    uint8_t reg = hue / 43;
//    uint8_t rem = (hue - (reg * 43)) * 6;
//    uint8_t p = (val * (255 - sat)) >> 8;
//    uint8_t q = (val * (255 - ((sat * rem) >> 8))) >> 8;
//    uint8_t t = (val * (255 - ((sat * (255 - rem)) >> 8))) >> 8;
//    switch (reg) {
        case 0: *_r = val, *_g = t, *_b = p; break;
        case 1: *_r = q, *_g = val, *_b = p; break;
        case 2: *_r = p, *_g = val, *_b = t; break;
        case 3: *_r = p, *_g = q, *_b = val; break;
        case 4: *_r = t, *_g = p, *_b = val; break;
        default: *_r = val, *_g = p, *_b = q; break;
    }
}

/* Exported definition functions -------------------------------------------- */
void ALED_Init(void)
{
    /* Auto-calculation! */
    uint32_t APBfq; // Clock freq
#ifdef APB1
    APBfq = HAL_RCC_GetPCLK1Freq();
    APBfq *= (RCC->CFGR & RCC_CFGR_PPRE) == 0 ? 1 : 2;
#endif
#ifdef APB2
    APBfq = HAL_RCC_GetPCLK2Freq();
    APBfq *= (RCC->CFGR & RCC_CFGR_PPRE2) == 0 ? 1 : 2;
#endif
    APBfq /= (uint32_t) (800 * 1000);  // 800 KHz - 1.25us
    TIM_HANDLE.Instance->PSC = 0;
    TIM_HANDLE.Instance->ARR = (uint16_t) (APBfq - 1);
    TIM_HANDLE.Instance->EGR = 1;
    PWM_HI = (uint8_t) (APBfq * 0.68) - 1;     // Log.1 - 0.85us
    PWM_LO = (uint8_t) (APBfq * 0.32) - 1;     // Log.0 - 0.40us

    /* Enable the Capture compare channel */
    TIM_CCxChannelCmd(TIM_HANDLE.Instance, TIM_CHANNEL_1, TIM_CCx_ENABLE);
    if (IS_TIM_BREAK_INSTANCE(TIM_HANDLE.Instance) != RESET)
    {
      /* Enable the main output */
      __HAL_TIM_MOE_ENABLE(&TIM_HANDLE);
    }
    HAL_Delay(1); // Make some delay

	  ALED_FillOFF();
	  while (ALED_Status() == ALED_BUSY);
	  ALED_Show();

    HAL_Delay(1); // Make some delay
}

ALDED_Status_t ALED_Status(void)
{
	return (ALDED_Status_t)HAL_TIM_PWM_GetState(&htim17);
}

ALDED_Status_t ALED_Show(void)
{
	ALDED_Status_t err = ALED_READY;
    if (BUF_COUNTER != 0 || (ALED_Status() != ALED_READY)) {
        err = ALED_BUSY;
    }
	for (uint8_t i = 0; i < 8; i++) {
		// set first transfer from first values
		PWM_BUF[i] = (((RGB_BUF[0] << i) & 0x80) > 0) ? PWM_HI : PWM_LO;
		PWM_BUF[i + 8] = (((RGB_BUF[1] << i) & 0x80) > 0) ? PWM_HI : PWM_LO;
		PWM_BUF[i + 16] = (((RGB_BUF[2] << i) & 0x80) > 0) ? PWM_HI : PWM_LO;
		PWM_BUF[i + 24] = (((RGB_BUF[3] << i) & 0x80) > 0) ? PWM_HI : PWM_LO;
		PWM_BUF[i + 32] = (((RGB_BUF[4] << i) & 0x80) > 0) ? PWM_HI : PWM_LO;
		PWM_BUF[i + 40] = (((RGB_BUF[5] << i) & 0x80) > 0) ? PWM_HI : PWM_LO;
	}
	BUF_COUNTER += 2;

	__HAL_TIM_SET_COUNTER(&TIM_HANDLE, 0);
	if (HAL_TIM_PWM_Start_DMA(&TIM_HANDLE, TIM_CH, (uint32_t*)PWM_BUF, (uint16_t)PWM_BUF_LEN)  != HAL_OK) {
		err = ALED_ERROR;
	}
    return err;
}

void ALED_SetBrightness(uint8_t brigtness)
{
    RGB_BR = brigtness;
}

/**
 * @brief Set LED with RGB color by index
 * @param[in] i LED position
 * @param[in] r Red component   [0..255]
 * @param[in] g Green component [0..255]
 * @param[in] b Blue component  [0..255]
 */
void ALED_SetRGB(uint16_t led, uint8_t red, uint8_t green, uint8_t blue)
{
    // overflow protection
    if (led >= NUM_PIXELS) {
        uint16_t _led = led / NUM_PIXELS;
        led -= _led * NUM_PIXELS;
    }
    // set brightness
    red /= 256 / ((uint16_t) RGB_BR + 1);
    green /= 256 / ((uint16_t) RGB_BR + 1);
    blue /= 256 / ((uint16_t) RGB_BR + 1);
#if USE_GAMMA_CORRECTION
    g = scale8(g, 0xB0);
    b = scale8(b, 0xF0);
#endif
    const uint8_t subp1 = green;
    const uint8_t subp2 = red;
    const uint8_t subp3 = blue;
    // RGB
    RGB_BUF[3 * led] = subp1;     // subpixel 1
    RGB_BUF[3 * led + 1] = subp2; // subpixel 2
    RGB_BUF[3 * led + 2] = subp3; // subpixel 3
}

/**
 * @brief Fill ALL LEDs with RGB color
 * @param[in] r Red component   [0..255]
 * @param[in] g Green component [0..255]
 * @param[in] b Blue component  [0..255]
 */
void ALED_FillRGB(uint8_t red, uint8_t green, uint8_t blue)
{
    for (uint16_t i = 0; i < NUM_PIXELS; i++)
        ALED_SetRGB(i, red, green, blue);
}

/**
 * @brief Set LED with HSV color by index
 * @param[in] i LED position
 * @param[in] hue HUE (color) [0..360]
 * @param[in] sat Saturation  [0..100]
 * @param[in] val Value (brightness) [0..100]
 */
void ALED_SetHSV(uint16_t led, uint16_t hue, uint8_t sat, uint8_t val)
{
    uint8_t _r, _g, _b;
    if (hue > 360) {
    	hue = 360;
    }
    if (sat > 100) {
    	sat = 100;
    }
    if (val > 100) {
    	val = 100;
    }
    HSV2RGB(hue, sat, val, &_r, &_g, &_b);
    ALED_SetRGB(led, _r, _g, _b);
}

/**
 * @brief Fill ALL LEDs with HSV color
 * @param[in] hue HUE (color) [0..360]
 * @param[in] sat Saturation  [0..100]
 * @param[in] val Value (brightness) [0..100]
 */
void ALED_FillHSV(uint16_t hue, uint8_t sat, uint8_t val)
{
    uint8_t _r, _g, _b;
    if (hue > 360) {
    	hue = 360;
    }
    if (sat > 100) {
    	sat = 100;
    }
    if (val > 100) {
    	val = 100;
    }
    HSV2RGB(hue, sat, val, &_r, &_g, &_b);
    ALED_FillRGB(_r, _g, _b);
}

/**
 * @brief Set White component in strip by index
 * @param[in] i LED position
 * @param[in] w White component [0..255]
 */
void ALED_SetWhite(uint16_t led, uint8_t white)
{
    white /= 256 / ((uint16_t) RGB_BR + 1);
    RGB_BUF[4 * led + 3] = white;
}

/**
 * @brief Set ALL White components in strip
 * @param[in] w White component [0..255]
 */
void ALED_FillWhite(uint8_t white)
{
    for (uint16_t i = 0; i < NUM_PIXELS; i++)
        ALED_SetWhite(i, white);
}

void ALED_FillOFF(void)
{
	for (uint16_t i = 0; i < NUM_PIXELS; i++)
        ALED_SetRGB(i, 0, 0, 0);
}
