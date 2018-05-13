#include <stm32f4xx.h>
#include <stm32f4xx_hal.h>
#include <stdio.h>
#include <board_driver/usb/usb.h>

#define ENABLE_WRITE_ACCESS 0x5555
#define SET_PRESCALER32 0x03
#define SET_RELOAD_TIME 0xFFF
#define START_IDWG 0xCCCC
#define RESET_IWDG 0xAAAA

IWDG_TypeDef *IwdgHandle = IWDG;
RCC_TypeDef *RCCHandle = RCC;

//Sets up the IWDG, by writing to the registers: Key, Prescale and reaload.
//If not key register is set to 0x5555 it is not allowed to set the proscale and
//reload register.
void setup_IWDG()
{
    IwdgHandle->KR = ENABLE_WRITE_ACCESS;
    IwdgHandle->PR = SET_PRESCALER32;
    IwdgHandle->RLR = SET_RELOAD_TIME;

    //Busy loop: Should not continue if the Reload Value Update (RVU) or the
    //Prescare Value Update (PVU) bit is set.
    while((IwdgHandle->SR & IWDG_SR_PVU_Msk) && (IwdgHandle->SR & IWDG_SR_RVU_Msk));
}

//At startup check if through IWDGRSTF if reset was caused by IWDG.
int was_reset_by_IWDG()
{
    return !!(RCCHandle->CSR & RCC_CSR_IWDGRSTF_Msk);
}

//Initialises the IWDG by writing 0xCCCC to the key register.
void init_IWDG()
{
    IwdgHandle->KR = START_IDWG;
}

//Refreshes the IWDG by writing 0xAAAA to the key register.
void reset_IWDG()
{
    IwdgHandle->KR = RESET_IWDG;
}
