#include <stm32f4xx_hal.h>
#include <stdbool.h>

#include "can.h"
#include "obdii.h"

#define FILL 0x55
#define MSG_LEN 8
#define DTC_LEN 2
#define DTC_FIRST_FRAME 2
#define DTC_PR_FRAME 3
#define MODE1_MAX_PID 0x7F

typedef enum {
	Single      = 0,
	First       = 1,
	Consecutive = 2,
	Flow        = 3,
	Unknown     = -1,
} MF_CANMode;

OBDII_Mode1_Frame mode1_buffer[MODE1_MAX_PID];

/////////////////////////////////////
// Convert to OBDII frame
////////////////////////////////////
static OBDII_Mode1_Frame can_to_obdii_mode1(const CAN_RxFrame* can_frame) {
	OBDII_Mode1_Frame frame = (OBDII_Mode1_Frame) {
		.Pid    = (OBDII_Mode1_Pid) can_frame->Msg[2],
		.Length = can_frame->Msg[0] - 2,
		.New 	= true,
	};
	for (uint8_t i = 0; i < frame.Length; i++) {
		frame.Msg[i] = can_frame->Msg[3 + i];
	}
	return frame;
}

/////////////////////////////////////////////
// Diagnostics Trouble Codes helper functions
/////////////////////////////////////////////
/*static MF_CANMode MultiFrameMode(const CanRxMsgTypeDef* can_frame) {
	return can_frame->Msg[0] >> 4 & 0xF;
}

static uint8_t SingleFrameMessageAmount(const CanRxMsgTypeDef* can_frame) {
	return (can_frame->Msg[0] & 0xF) / DTC_LEN;
}

static uint16_t MultiFrameMessageAmount(const CAN_Frame* can_frame) {
	return ((can_frame->Msg[0] & 0xF) << 4 | can_frame->Msg[1]) / DTC_LEN;
}

static DTC_Message ByteToDTC(const uint8_t *message) {
	return (DTC_Message) {
		.FirstChar  = message[0] >> 6 & 0x3,
		.SecondChar = message[0] >> 4 & 0x3,
		.ThirdChar  = message[0] & 0xF,
		.FourthChar = message[1] >> 4 & 0xF,
		.FifthChar  = message[1] & 0xF,
	};
}*/

////////////////////////////////////
// Receive callback
////////////////////////////////////
static void obdii_response_handler(CAN_RxFrame *msg) {
	if (msg->Msg[1] == 0x41) {
		OBDII_Mode1_Frame frame = can_to_obdii_mode1(msg);
		mode1_buffer[frame.Pid] = frame;
	}
}

///////////////////////////////////
// Public functions
///////////////////////////////////
HAL_StatusTypeDef OBDII_Init() {
	for (int i = 0; i < MODE1_MAX_PID; i++) {
		mode1_buffer[i] = (OBDII_Mode1_Frame) {
			.Pid = MODE1_MAX_PID + 1,
			.Length = 0,
			.New = false,
		};
	}

	return CAN_Filter(OBDII_RESPONSE_ID, 0x7F8, obdii_response_handler);
}

/*bool GetTroubleCodes(DTC_Message message[], size_t* len) {
	static bool running = false;
	static MF_CANMode frameType = Unknown;
	static size_t length = 0;
	static size_t messageCount = 0;
	static uint8_t frameIndex = 1;
	if (!running) {
		// Diagnostics trouble code request
		CAN_Send(OBDII_REQUEST_ID, (uint8_t[])  {1, DTC, FILL, FILL, FILL, FILL, FILL, FILL}, MSG_LEN);
		running = true;
	}

	CAN_Frame received;
	if (frameType == Unknown) {
		if (CAN_Receive(OBDII_FilterNum, &received)) {
			frameType = MultiFrameMode(&received);
		}
		else {
			return false;
		}
	}

	if (frameType == Single) {
		*len = SingleFrameMessageAmount(&received);

		// Get DTC in single frame
		for (uint8_t i = 0; i < *len; i++) {
			message[i] = ByteToDTC(received.Msg + 1 + i * 2);
		}

		running = false;
		frameType = Unknown;
		return true;
	}
	else if (frameType == First) {
		// Send flow control
		CAN_Send(OBDII_REQUEST_ID, (uint8_t[]) {Flow << 4, 0, 0, FILL, FILL, FILL, FILL, FILL}, MSG_LEN);

		// Handle DTC in first frame
		message[0] = ByteToDTC(received.Msg + 1);
		message[1] = ByteToDTC(received.Msg + 3);

		messageCount += 2;
		length = MultiFrameMessageAmount(&received);
	}

	// While there are more messages
	while (messageCount < length - 1) {
		if (CAN_Receive(OBDII_FilterNum, &received)) {
			if ((received.Msg[0] & 0xFF) != frameIndex) // Check that index order is correct
			{   } // Error
			frameIndex = (frameIndex + 1) % 0xF;

			// For all DTC in frame
			for (uint8_t j = 0; j < DTC_PR_FRAME; j++) {
				if (messageCount < length - 1) {
					message[messageCount++] = ByteToDTC(received.Msg + (j + 1) * DTC_LEN);
				}
			}
		}
		else {
			return false;
		}
	}

	*len = length;

	running = false;
	frameType = Unknown;
	length = 0;
	messageCount = 0;
	frameIndex = 1;
	return true;
}

void ClearTroubleCodes() {
	CAN_Send(OBDII_REQUEST_ID, (uint8_t[]) {1, ClearDTC, FILL, FILL, FILL, FILL, FILL, FILL}, MSG_LEN);
}*/

HAL_StatusTypeDef OBDII_Mode1_Request(OBDII_Mode1_Pid pid) {
	assert_param(pid != FreezeDTC);
	return CAN_Send(OBDII_REQUEST_ID, (uint8_t[]) {2, ShowCurrent, pid, FILL, FILL, FILL, FILL, FILL}, MSG_LEN);
}

/*void OBDII_Mode2_Request(OBDII_Pid pid) {
	assert_param(pid != MonitorStatus);
	CAN_Send(OBDII_REQUEST_ID, (uint8_t[]) {2, ShowFreeze, pid, FILL, FILL, FILL, FILL, FILL}, MSG_LEN);
}*/

OBDII_Mode1_Frame OBDII_Mode1_Response(OBDII_Mode1_Pid pid) {
	OBDII_Mode1_Frame frame = mode1_buffer[pid];

	if (frame.New) {
		mode1_buffer[pid].New = false;
	}

	return frame;
}

uint32_t OBDII_Mode1_UID(OBDII_Mode1_Pid pid) {
	return (CAN_OBD_ID_START << 8) | pid;
}

void OBDII_Burst(void) {
	OBDII_Mode1_Request(MonitorStatus);
	OBDII_Mode1_Request(FuelSystemStatus);
	OBDII_Mode1_Request(CalculatedEngineLoad);
	OBDII_Mode1_Request(EngineCoolantTemperature);
	OBDII_Mode1_Request(ShortTermFuelTrim);
	OBDII_Mode1_Request(LongTermFuelTrim);
	OBDII_Mode1_Request(IntakeManifoldPressure);
	OBDII_Mode1_Request(EngineRPM);
	OBDII_Mode1_Request(VehicleSpeed);
	OBDII_Mode1_Request(TimingAdvance);
	OBDII_Mode1_Request(IntakeAirTemperature);
	OBDII_Mode1_Request(ThrottlePosition);
	OBDII_Mode1_Request(OxygenSensorsPresent);
	OBDII_Mode1_Request(RuntimeEngineStart);
	OBDII_Mode1_Request(DistanceWithMIL);
	OBDII_Mode1_Request(DistanceSinceClear);
	OBDII_Mode1_Request(OxygenSensorFARatio);
	OBDII_Mode1_Request(MonitorStatusDriveCycle);
	OBDII_Mode1_Request(ControlModuleVoltage);
	OBDII_Mode1_Request(FARatioCommanded);
	OBDII_Mode1_Request(RelativeThrottlePosition);
	OBDII_Mode1_Request(AbsoluteThrottlePosition);
	OBDII_Mode1_Request(EngineFuelRate);
	OBDII_Mode1_Request(DriverDemandTorque);
	OBDII_Mode1_Request(EngineReferenceTorque);
	OBDII_Mode1_Request(EnginePercentTorque);
}
