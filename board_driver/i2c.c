#include <stm32f4xx_hal.h>
#include <stdbool.h>
#include <string.h>

#include "can.h"
#include "i2c.h"

#define BUFFER_SIZE 256

typedef struct {
	uint16_t addr;
	uint8_t buf[32];
	size_t n;
} i2c_msg;

static I2C_HandleTypeDef i2cHandle;
// static volatile uint16_t head = 0;
// static volatile uint16_t tail = 0;
// static volatile uint16_t size = 0;
// static i2c_msg transmit_buffer[BUFFER_SIZE];

static void set_SDA_as_input_pin(void) {
	GPIO_InitTypeDef  GPIO_InitStruct;

	GPIO_InitStruct.Mode      = GPIO_MODE_INPUT;
	GPIO_InitStruct.Pull      = GPIO_PULLUP;
	GPIO_InitStruct.Speed     = GPIO_SPEED_FAST;
	GPIO_InitStruct.Pin       = DASHBOARD_I2C_SDA_PIN;
	// GPIO_InitStruct.Alternate = 0; // Disablw, how?

	HAL_GPIO_Init(DASHBOARD_I2C_SDA_GPIO_PORT, &GPIO_InitStruct);
}

static void set_SCL_as_output_pin(void) {
	GPIO_InitTypeDef  GPIO_InitStruct;
	GPIO_InitStruct.Pin       = DASHBOARD_I2C_SCL_PIN;
	GPIO_InitStruct.Mode      = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull      = GPIO_PULLUP;
	GPIO_InitStruct.Speed     = GPIO_SPEED_FAST;

	HAL_GPIO_Init(DASHBOARD_I2C_SCL_GPIO_PORT, &GPIO_InitStruct);

}

static bool SDA_is_low(void) {
	return HAL_GPIO_ReadPin(DASHBOARD_I2C_SDA_GPIO_PORT, DASHBOARD_I2C_SDA_PIN) == GPIO_PIN_RESET;
}

static void set_SCL_high(void) {
	HAL_GPIO_WritePin(DASHBOARD_I2C_SCL_GPIO_PORT, DASHBOARD_I2C_SCL_PIN, GPIO_PIN_SET);
}

static void toggle_SCL(void) {
	HAL_GPIO_TogglePin(DASHBOARD_I2C_SCL_GPIO_PORT, DASHBOARD_I2C_SCL_PIN);
}

static int bus_recovering(void) {
	set_SDA_as_input_pin();
	if (SDA_is_low()) {
		set_SCL_as_output_pin();
		set_SCL_high();

		int retry_limit = 100;
		while (SDA_is_low()) {
			toggle_SCL();
			if (!retry_limit--) return -1; // Should not take more than 8 cycles or there is another problem
		}
	}
	return 0;
}

int i2c_init(void) {
	bus_recovering();

	i2cHandle.Instance             = DASHBOARD_I2C;

	i2cHandle.Init.AddressingMode  = I2C_ADDRESSINGMODE_7BIT,//I2C_ADDRESSINGMODE_10BIT;
	i2cHandle.Init.ClockSpeed      = 400000;
	i2cHandle.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
	i2cHandle.Init.DutyCycle       = I2C_DUTYCYCLE_16_9, //I2C_DUTYCYCLE_2;
	i2cHandle.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
	i2cHandle.Init.NoStretchMode   = I2C_NOSTRETCH_DISABLE;
	// i2cHandle.Init.OwnAddress1     = 0x00;/*I2C_ADDRESS*/;
	// i2cHandle.Init.OwnAddress2     = 0xFE;

	if(HAL_I2C_Init(&i2cHandle) != HAL_OK) {
		return -1;
	}

	return 0;
}

int i2c_is_ready(uint16_t addr) {
	return HAL_I2C_IsDeviceReady(&i2cHandle, addr, 1000, 1000);
}

int i2c_master_transmit_DMA(uint16_t addr, void *buf, size_t n) {
	while (HAL_I2C_GetState(&i2cHandle) != HAL_I2C_STATE_READY) {}

	// retry:
	if (HAL_I2C_Master_Transmit_DMA(&i2cHandle, addr, (uint8_t*)buf, n) != HAL_OK) {
		/*if (size < BUFFER_SIZE) {
			transmit_buffer[head].addr = addr;
			transmit_buffer[head].n = n;
			memcpy(transmit_buffer[head].buf, buf, n);

			head = (head + 1) % BUFFER_SIZE;
			size++;
		}*/

		return 1;
		// if (HAL_I2C_GetError(&i2cHandle) == HAL_I2C_ERROR_AF) {
		// 	goto retry;
		// }
	}
	return 0;

	// return HAL_I2C_Master_Transmit(&i2cHandle, addr, (uint8_t*)buf, n, 1000) == HAL_OK ? 0 : -1;
	// while (HAL_I2C_GetState(&i2cHandle) != HAL_I2C_STATE_READY) {;}
	// return HAL_I2C_Master_Transmit_DMA(&i2cHandle, addr, (uint8_t*)buf, n) == HAL_OK ?
	// 0 : -1;
}

void HAL_I2C_MspInit(I2C_HandleTypeDef *hi2c) {
	static DMA_HandleTypeDef hdma_tx;
	static DMA_HandleTypeDef hdma_rx;

	GPIO_InitTypeDef  GPIO_InitStruct;

	/*##-1- Enable peripherals and GPIO Clocks #################################*/
	/* Enable GPIO TX/RX clock */
	DASHBOARD_I2C_SCL_GPIO_CLK_ENABLE();
	DASHBOARD_I2C_SDA_GPIO_CLK_ENABLE();
	/* Enable I2C1 clock */
	DASHBOARD_I2C_CLK_ENABLE();
	/* Enable DMA2 clock */
	DASHBOARD_I2C_DMA_CLK_ENABLE();

	/*##-2- Configure peripheral GPIO ##########################################*/
	/* I2C TX GPIO pin configuration  */
	GPIO_InitStruct.Pin       = DASHBOARD_I2C_SCL_PIN;
	GPIO_InitStruct.Mode      = GPIO_MODE_AF_OD;
	GPIO_InitStruct.Pull      = GPIO_PULLUP;
	GPIO_InitStruct.Speed     = GPIO_SPEED_FAST;
	GPIO_InitStruct.Alternate = DASHBOARD_I2C_SCL_AF;

	HAL_GPIO_Init(DASHBOARD_I2C_SCL_GPIO_PORT, &GPIO_InitStruct);

	/* I2C RX GPIO pin configuration  */
	GPIO_InitStruct.Pin = DASHBOARD_I2C_SDA_PIN;
	GPIO_InitStruct.Alternate = DASHBOARD_I2C_SDA_AF;

	HAL_GPIO_Init(DASHBOARD_I2C_SDA_GPIO_PORT, &GPIO_InitStruct);

	/*##-3- Configure the DMA streams ##########################################*/
	/* Configure the DMA handler for Transmission process */
	hdma_tx.Instance                 = DASHBOARD_I2C_TX_DMA_STREAM;

	hdma_tx.Init.Channel             = DASHBOARD_I2C_TX_DMA_CHANNEL;
	hdma_tx.Init.Direction           = DMA_MEMORY_TO_PERIPH;
	hdma_tx.Init.PeriphInc           = DMA_PINC_DISABLE;
	hdma_tx.Init.MemInc              = DMA_MINC_ENABLE;
	hdma_tx.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
	hdma_tx.Init.MemDataAlignment    = DMA_MDATAALIGN_BYTE;
	hdma_tx.Init.Mode                = DMA_NORMAL;
	hdma_tx.Init.Priority            = DMA_PRIORITY_LOW;
	hdma_tx.Init.FIFOMode            = DMA_FIFOMODE_DISABLE;
	hdma_tx.Init.FIFOThreshold       = DMA_FIFO_THRESHOLD_FULL;
	hdma_tx.Init.MemBurst            = DMA_MBURST_INC4;
	hdma_tx.Init.PeriphBurst         = DMA_PBURST_INC4;

	HAL_DMA_Init(&hdma_tx);

	/* Associate the initialized DMA handle to the the I2C handle */
	__HAL_LINKDMA(hi2c, hdmatx, hdma_tx);

	/* Configure the DMA handler for Transmission process */
	hdma_rx.Instance                 = DASHBOARD_I2C_RX_DMA_STREAM;

	hdma_rx.Init.Channel             = DASHBOARD_I2C_RX_DMA_CHANNEL;
	hdma_rx.Init.Direction           = DMA_PERIPH_TO_MEMORY;
	hdma_rx.Init.PeriphInc           = DMA_PINC_DISABLE;
	hdma_rx.Init.MemInc              = DMA_MINC_ENABLE;
	hdma_rx.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
	hdma_rx.Init.MemDataAlignment    = DMA_MDATAALIGN_BYTE;
	hdma_rx.Init.Mode                = DMA_NORMAL;
	hdma_rx.Init.Priority            = DMA_PRIORITY_HIGH;
	hdma_rx.Init.FIFOMode            = DMA_FIFOMODE_DISABLE;
	hdma_rx.Init.FIFOThreshold       = DMA_FIFO_THRESHOLD_FULL;
	hdma_rx.Init.MemBurst            = DMA_MBURST_INC4;
	hdma_rx.Init.PeriphBurst         = DMA_PBURST_INC4;

	HAL_DMA_Init(&hdma_rx);

	/* Associate the initialized DMA handle to the the I2C handle */
	__HAL_LINKDMA(hi2c, hdmarx, hdma_rx);

	/*##-4- Configure the NVIC for DMA #########################################*/
	/* NVIC configuration for DMA transfer complete interrupt (I2C1_TX) */
	HAL_NVIC_SetPriority(DASHBOARD_I2C_DMA_TX_IRQn, 0, 1);
	HAL_NVIC_EnableIRQ(DASHBOARD_I2C_DMA_TX_IRQn);

	/* NVIC configuration for DMA transfer complete interrupt (I2C1_RX) */
	HAL_NVIC_SetPriority(DASHBOARD_I2C_DMA_RX_IRQn, 0, 0);
	HAL_NVIC_EnableIRQ(DASHBOARD_I2C_DMA_RX_IRQn);

	/*##-5- Configure the NVIC for I2C #########################################*/
	/* NVIC for I2C1 */

	HAL_NVIC_SetPriority(DASHBOARD_I2C_ER_IRQn, 0, 1);
	HAL_NVIC_EnableIRQ(DASHBOARD_I2C_ER_IRQn);
	HAL_NVIC_SetPriority(DASHBOARD_I2C_EV_IRQn, 0, 2);
	HAL_NVIC_EnableIRQ(DASHBOARD_I2C_EV_IRQn);
}

void HAL_I2C_MspDeInit(I2C_HandleTypeDef *hi2c) {
	(void) hi2c;

	static DMA_HandleTypeDef hdma_tx;
	static DMA_HandleTypeDef hdma_rx;

	/*##-1- Reset peripherals ##################################################*/
	DASHBOARD_I2C_FORCE_RESET();
	DASHBOARD_I2C_RELEASE_RESET();

	/*##-2- Disable peripherals and GPIO Clocks ################################*/
	/* Configure I2C Tx as alternate function  */
	HAL_GPIO_DeInit(DASHBOARD_I2C_SCL_GPIO_PORT, DASHBOARD_I2C_SCL_PIN);
	/* Configure I2C Rx as alternate function  */
	HAL_GPIO_DeInit(DASHBOARD_I2C_SDA_GPIO_PORT, DASHBOARD_I2C_SDA_PIN);

	/*##-3- Disable the DMA Streams ############################################*/
	/* De-Initialize the DMA Stream associate to transmission process */
	HAL_DMA_DeInit(&hdma_tx);
	/* De-Initialize the DMA Stream associate to reception process */
	HAL_DMA_DeInit(&hdma_rx);

	/*##-4- Disable the NVIC for DMA ###########################################*/
	HAL_NVIC_DisableIRQ(DASHBOARD_I2C_DMA_TX_IRQn);
	HAL_NVIC_DisableIRQ(DASHBOARD_I2C_DMA_RX_IRQn);

	/*##-5- Disable the NVIC for I2C ###########################################*/
	HAL_NVIC_DisableIRQ(DASHBOARD_I2C_ER_IRQn);
	HAL_NVIC_DisableIRQ(DASHBOARD_I2C_EV_IRQn);
}

/*void HAL_I2C_MasterTxCpltCallback(I2C_HandleTypeDef *I2cHandle) {

		CAN_Send(12, (uint8_t[]) { 1 }, 1);
	if (size > 0) {
		static i2c_msg transmit;
		transmit = transmit_buffer[tail];
		tail = (tail + 1) % BUFFER_SIZE;
		size--;
		HAL_I2C_Master_Transmit_DMA(I2cHandle, transmit.addr, transmit.buf, transmit.n);
	}
}*/

// interrupts
void DASHBOARD_I2C_EV_IRQHandler(void) {
	HAL_I2C_EV_IRQHandler(&i2cHandle);
}

void DASHBOARD_I2C_ER_IRQHandler(void) {
	HAL_I2C_ER_IRQHandler(&i2cHandle);
}


void DASHBOARD_I2C_DMA_RX_IRQHandler(void) {
	HAL_DMA_IRQHandler(i2cHandle.hdmarx);
}


void DASHBOARD_I2C_DMA_TX_IRQHandler(void) {
	HAL_DMA_IRQHandler(i2cHandle.hdmatx);
}
