#include <stm32f4xx_hal.h>

#include <stdint.h>
#include <stdbool.h>

#include "init.h"


void set_system_clock_168mhz(void) {
	__HAL_RCC_PWR_CLK_ENABLE();

	__HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

	const uint32_t HSE_mhz = HSE_VALUE/10e5;
	const bool HSE_is_odd  = (HSE_mhz & 1) == 1;

	RCC_OscInitTypeDef RCC_OscInit = {
		.OscillatorType = RCC_OSCILLATORTYPE_HSE,
		.HSEState       = RCC_HSE_ON,
		.PLL = {
			.PLLState   = RCC_PLL_ON,
			.PLLSource  = RCC_PLLSOURCE_HSE,
			.PLLM       = 8, // should match HSE freq in mhz so HSE/PLLM == 1
			.PLLN       = 336, // 336 / 2 == 168 == max core clk
			.PLLP       = 4, // ((HSE/pllm) * plln) / pllp == 168 (max core clk)
			.PLLQ       = 7, // ((HSE/pllm) * plln) / pllq == 48 (USB needs 48 and sdio needs 48 or lower)
		},
	};
	if (HAL_RCC_OscConfig(&RCC_OscInit) != HAL_OK) {
		while (1);
	}

	RCC_ClkInitTypeDef RCC_ClkInit = {
		.ClockType      = (RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2),
		.SYSCLKSource   = RCC_SYSCLKSOURCE_PLLCLK,
		.AHBCLKDivider  = RCC_SYSCLK_DIV1,
		.APB1CLKDivider = RCC_HCLK_DIV1,
		.APB2CLKDivider = RCC_HCLK_DIV2,
	};
	// We set FLASH_LATENCY_5 as we are in vcc range 2.7-3.6 at 168mhz
	// see datasheet table 10 at page 80.
	if (HAL_RCC_ClockConfig(&RCC_ClkInit, FLASH_LATENCY_5) != HAL_OK) {
		while (1);
	}
}


void boot(uint32_t address) {
	uint32_t appStack = (uint32_t) *(uint32_t*)address;
	void (*appEntry)(void) = (void*)*(uint32_t*)(address + 4);

	__disable_irq();
	__set_MSP(appStack);
	appEntry();
}
