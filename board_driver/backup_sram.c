/*
 * Backup SRAM
 * See reference manual page 119-120
 */

#include <stm32f407xx.h>
#include "backup_sram.h"


PWR_TypeDef *pwr = (void*)PWR_BASE;


void disable_backup_sram() {
	pwr->CSR |= ~PWR_CSR_BRE;
}


void enable_backup_sram() {
	pwr->CSR |= PWR_CSR_BRE;

	/* Wait for backup regulator to get ready. */
	while (pwr->CSR & PWR_CSR_BRR);
}
