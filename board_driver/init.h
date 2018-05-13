#include <stdint.h>

#define init_board() HAL_Init()

void boot(uint32_t address);
void set_system_clock_168mhz(void);
