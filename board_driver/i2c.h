#ifndef I2C_H
#define I2C_H

#include <stm32f4xx_hal.h>

#define DASHBOARD_I2C                             I2C2
#define DASHBOARD_I2C_CLK_ENABLE()                __HAL_RCC_I2C2_CLK_ENABLE()
#define DASHBOARD_I2C_DMA_CLK_ENABLE()            __HAL_RCC_DMA1_CLK_ENABLE()
#define DASHBOARD_I2C_SDA_GPIO_CLK_ENABLE()       __HAL_RCC_GPIOB_CLK_ENABLE()
#define DASHBOARD_I2C_SCL_GPIO_CLK_ENABLE()       __HAL_RCC_GPIOB_CLK_ENABLE()

#define DASHBOARD_I2C_FORCE_RESET()               __HAL_RCC_I2C2_FORCE_RESET()
#define DASHBOARD_I2C_RELEASE_RESET()             __HAL_RCC_I2C2_RELEASE_RESET()

/* Definition for DASHBOARD_I2C Pins */
#define DASHBOARD_I2C_SCL_PIN                    GPIO_PIN_10
#define DASHBOARD_I2C_SCL_GPIO_PORT              GPIOB
#define DASHBOARD_I2C_SCL_AF                     GPIO_AF4_I2C2
#define DASHBOARD_I2C_SDA_PIN                    GPIO_PIN_11
#define DASHBOARD_I2C_SDA_GPIO_PORT              GPIOB
#define DASHBOARD_I2C_SDA_AF                     GPIO_AF4_I2C2

/* Definition for DASHBOARD_I2C's DMA */
#define DASHBOARD_I2C_TX_DMA_CHANNEL             DMA_CHANNEL_7
#define DASHBOARD_I2C_TX_DMA_STREAM              DMA1_Stream7
#define DASHBOARD_I2C_RX_DMA_CHANNEL             DMA_CHANNEL_7
#define DASHBOARD_I2C_RX_DMA_STREAM              DMA1_Stream2

/* Definition for DASHBOARD_I2C's DMA NVIC */
#define DASHBOARD_I2C_DMA_TX_IRQn                DMA1_Stream7_IRQn
#define DASHBOARD_I2C_DMA_RX_IRQn                DMA1_Stream2_IRQn
#define DASHBOARD_I2C_DMA_TX_IRQHandler          DMA1_Stream7_IRQHandler
#define DASHBOARD_I2C_DMA_RX_IRQHandler          DMA1_Stream2_IRQHandler

/* Definition for DASHBOARD_I2C's NVIC */
#define DASHBOARD_I2C_EV_IRQn                    I2C2_EV_IRQn
#define DASHBOARD_I2C_EV_IRQHandler              I2C2_EV_IRQHandler
#define DASHBOARD_I2C_ER_IRQn                    I2C2_ER_IRQn
#define DASHBOARD_I2C_ER_IRQHandler              I2C2_ER_IRQHandler

int i2c_init(void);
int i2c_master_transmit_DMA(uint16_t addr, void *buf, size_t n);
int i2c_is_ready(uint16_t addr);


#endif /* I2C_H */
