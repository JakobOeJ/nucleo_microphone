#include <stm32f4xx_hal.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include <board_driver/uart.h>
#include <board_driver/can.h>

static CAN_RxFrame lastMsg;
static bool received = false;

void AllMsg(CAN_RxFrame *msg);

int main(void) {
	uart_init();

	printf("UART init complete\n");
	if (CAN_Init(CAN_PD0) != CAN_OK) {
		printf("CAN init error\n");
	}

	if (CAN_Filter(0, 0, AllMsg) != CAN_OK) {
		printf("CAN filter error\n");
	}

	HAL_Delay(100);

	if (CAN_Start() != CAN_OK){
		printf("CAN start error\n");
	}
	else {
		printf("CAN started\n");
	}

	uint8_t i = 'a';

	while(true) {
		if (CAN_Send(0x7ff, (uint8_t[]) { i }, 1) == CAN_OK) {
			printf("Transmitting\n");
		}
		else {
			printf("Send error\n");
		}

		i++;
		if (i == 'z' + 1) {
			i = 'a';
        }

		printf("Received %u\n", (unsigned) CAN_GetStats().receive);

        if (received) {
			lastMsg.Msg[lastMsg.Length] = '\0';
			printf("Message: %s\n", lastMsg.Msg);

			received = false;
		}

		HAL_Delay(1000);
	}
}

void AllMsg(CAN_RxFrame *msg) {
	lastMsg = *msg;
	received = true;
}
