#define ADCx	                        ADC1
#define ADCx_CLK_ENABLE()               __HAL_RCC_ADC1_CLK_ENABLE()
#define DMAx_CLK_ENABLE()               __HAL_RCC_DMA2_CLK_ENABLE()
#define ADCx_FORCE_RESET()              __HAL_RCC_ADC_FORCE_RESET()
#define ADCx_RELEASE_RESET()            __HAL_RCC_ADC_RELEASE_RESET()

#define ADCx_DMA_CHANNEL                DMA_CHANNEL_0
#define ADCx_DMA_STREAM                 DMA2_Stream0

#define ADCx_DMA_IRQn                   DMA2_Stream0_IRQn
#define ADCx_DMA_IRQHandler             DMA2_Stream0_IRQHandler

#define ADCx_IRQn						ADC_IRQn

HAL_StatusTypeDef init_adc(int num_conv);
void init_analog_pins(GPIO_TypeDef *port, uint32_t pin);
HAL_StatusTypeDef init_adc_channel(uint32_t channel, uint8_t *number);
HAL_StatusTypeDef start_adc();
HAL_StatusTypeDef stop_adc();
int read_adc_value(uint8_t number);
