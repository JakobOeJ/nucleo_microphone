#include <stm32f4xx_hal.h>

#include "adc.h"

ADC_HandleTypeDef    AdcHandle;
static int rank = 1;
__IO uint32_t values[16];
int number_of_conversions = 0;

// HAL init function. DO NOT CALL.
void HAL_ADC_MspInit(ADC_HandleTypeDef * hadc) {
	(void) hadc;
	static DMA_HandleTypeDef  hdma_adc;

	ADCx_CLK_ENABLE();
  	DMAx_CLK_ENABLE();

	hdma_adc.Instance = ADCx_DMA_STREAM;

	hdma_adc.Init.Channel  = ADCx_DMA_CHANNEL;
	hdma_adc.Init.Direction = DMA_PERIPH_TO_MEMORY;
	hdma_adc.Init.PeriphInc = DMA_PINC_DISABLE;
	hdma_adc.Init.MemInc = DMA_MINC_ENABLE;
	hdma_adc.Init.PeriphDataAlignment = DMA_PDATAALIGN_WORD;
	hdma_adc.Init.MemDataAlignment = DMA_MDATAALIGN_WORD;
	hdma_adc.Init.Mode = DMA_CIRCULAR;
	hdma_adc.Init.Priority = DMA_PRIORITY_HIGH;
	hdma_adc.Init.FIFOMode = DMA_FIFOMODE_DISABLE;
	hdma_adc.Init.FIFOThreshold = DMA_FIFO_THRESHOLD_HALFFULL;
	hdma_adc.Init.MemBurst = DMA_MBURST_SINGLE;
	hdma_adc.Init.PeriphBurst = DMA_PBURST_SINGLE;

	HAL_DMA_Init(&hdma_adc);

	__HAL_LINKDMA(hadc, DMA_Handle, hdma_adc);

	HAL_NVIC_SetPriority(ADCx_DMA_IRQn, 0, 0);
	HAL_NVIC_EnableIRQ(ADCx_DMA_IRQn);
}

// HAL deinit function. DO NOT CALL.
void HAL_ADC_MspDeInit(ADC_HandleTypeDef * hadc) {
	(void) hadc;
	static DMA_HandleTypeDef  hdma_adc;

	ADCx_FORCE_RESET();
    ADCx_RELEASE_RESET();

	HAL_DMA_DeInit(&hdma_adc);
	HAL_NVIC_DisableIRQ(ADCx_DMA_IRQn);
}

// DMA interrupt service routine. DO NOT CALL.
void ADCx_DMA_IRQHandler(void)
{
	HAL_DMA_IRQHandler(AdcHandle.DMA_Handle);
}

HAL_StatusTypeDef init_adc(int num_conv) {
	number_of_conversions = num_conv;
  	AdcHandle.Instance          = ADCx;

	AdcHandle.Init.ClockPrescaler        = ADC_CLOCKPRESCALER_PCLK_DIV8;
	AdcHandle.Init.Resolution            = ADC_RESOLUTION_12B;
	AdcHandle.Init.ScanConvMode          = ENABLE;
	AdcHandle.Init.ContinuousConvMode    = ENABLE;
	AdcHandle.Init.DiscontinuousConvMode = DISABLE;
	AdcHandle.Init.NbrOfDiscConversion   = 0;
	AdcHandle.Init.ExternalTrigConvEdge  = ADC_EXTERNALTRIGCONVEDGE_NONE;
	AdcHandle.Init.ExternalTrigConv      = ADC_SOFTWARE_START;
	AdcHandle.Init.DataAlign             = ADC_DATAALIGN_RIGHT;
	AdcHandle.Init.NbrOfConversion       = num_conv;
	AdcHandle.Init.DMAContinuousRequests = ENABLE;
	AdcHandle.Init.EOCSelection          = DISABLE;

	return HAL_ADC_Init(&AdcHandle);
}

void init_analog_pins(GPIO_TypeDef *port, uint32_t pin) {
	GPIO_InitTypeDef GPIO_InitStruct = {
		.Pin  = pin,
		.Mode = GPIO_MODE_ANALOG,
		.Pull = GPIO_NOPULL,
	};

	HAL_GPIO_Init(port, &GPIO_InitStruct);
}

HAL_StatusTypeDef init_adc_channel(uint32_t channel, uint8_t *number) {
	*number = rank - 1;

	ADC_ChannelConfTypeDef cConfig = {
		.Channel      	= channel,                		/* Sampled channel number */
		.Rank         	= rank++,          				/* Rank of sampled channel number ADCx1_CHANNEL */
		.SamplingTime 	= ADC_SAMPLETIME_56CYCLES,    	/* Sampling time (number of clock cycles unit) */
		.Offset 		= 0,                            /* Parameter discarded because offset correction is disabled */
	};

	return HAL_ADC_ConfigChannel(&AdcHandle, &cConfig);
}

HAL_StatusTypeDef start_adc() {
	return HAL_ADC_Start_DMA(&AdcHandle, (uint32_t*) values, number_of_conversions);
}

HAL_StatusTypeDef stop_adc() {
	return HAL_ADC_Stop_DMA(&AdcHandle) != HAL_OK;
}

int read_adc_value(uint8_t number) {
	return values[number];
}
