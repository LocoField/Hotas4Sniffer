#include "Hotas4Sniffer.h"

#include <stdio.h>

enum ButtonMap
{
	/* for buttons1 */
	SWITCH_HAT_UP = 0,
	SWITCH_HAT_UP_RIGHT = 1,
	SWITCH_HAT_RIGHT = 2,
	SWITCH_HAT_DOWN_RIGHT = 3,
	SWITCH_HAT_DOWN = 4,
	SWITCH_HAT_DOWN_LEFT = 5,
	SWITCH_HAT_LEFT = 6,
	SWITCH_HAT_UP_LEFT = 7,
	SWITCH_HAT_CENTER = 8,

	BUTTON_R1 = 10,
	BUTTON_L1 = 20,
	BUTTON_R3 = 40,
	BUTTON_L3 = 80,

	/* for buttons2 */
	BUTTON_RECTANGLE = 1,
	BUTTON_CROSS = 2,
	BUTTON_CIRCLE = 4,
	BUTTON_TRIANGLE = 8,

	BUTTON_R2 = 10,
	BUTTON_L2 = 20,
	BUTTON_SHARE = 40,
	BUTTON_OPTIONS = 80,
};

#pragma pack(push, 1)
struct Hotas4Data
{
	unsigned char index;     // [1]
	unsigned short handleX;  // [0-1023]
	unsigned short handleY;  // [0-1023]
	unsigned char throttle;  // [ff-00]
	unsigned char twist;     // [00-ff]
	unsigned char pedals[3]; // [ff,ff,80]: left brake, right brake, rudder
	unsigned char rocker;    // [00-ff]
	unsigned char buttons1;  // [1|2|4|8|10|20|40|80]: hat-switch, R1, L1, R3, L3]
	unsigned char buttons2;  // [1|2|4|8|10|20|40|80]: ¡à, ¡¿, ¡Û, ¡â, R2, L2, SHARE, OPTIONS
};
#pragma pack(pop)

Hotas4Sniffer::Hotas4Sniffer()
{
}

bool Hotas4Sniffer::findDevice()
{
	const USHORT idVendor = 1103;
	const USHORT idProduct = 46716;

	return __super::findDevice(idVendor, idProduct);
}

void Hotas4Sniffer::processInterruptData(unsigned char* buffer, DWORD bytes)
{
	Hotas4Data data;
	memcpy(&data, buffer, sizeof(data));

#ifdef _DEBUG
	SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), COORD());

	printf("handle: %4d, %4d\n", data.handleX, data.handleY);
	printf("twist: %3d\n", data.twist);
	printf("throttle: %3d\n", data.throttle);
	printf("rocker: %3d\n", data.rocker);
	printf("button1: %3d\n", data.buttons1);
	printf("button2: %3d\n", data.buttons2);
	printf("pedals: %3d %3d %3d\n", data.pedals[0], data.pedals[1], data.pedals[2]);
#endif

	Axis currentAxis = Axis::fromHotas4Handle(data.handleX, data.handleY, data.twist);

	if (lastAxis != currentAxis)
	{
		onAxisEvent(currentAxis);
	}

	lastAxis = currentAxis;
}

void Hotas4Sniffer::onAxisEvent(const Axis& axis)
{
	printf("\n");
	printf("Axis: %3.2f  %3.2f  %3.2f  \n", axis.roll, axis.pitch, axis.yaw);

	if (callbackAxisEvent)
	{
		callbackAxisEvent(axis);
	}
}

void Hotas4Sniffer::onButtonEvent(int type, int buttonId)
{
	if (callbackButtonEvent)
	{
		callbackButtonEvent(type, buttonId);
	}
}

void Hotas4Sniffer::setAxisEventCallback(std::function<void(const Axis&)> callback)
{
	callbackAxisEvent = callback;
}

void Hotas4Sniffer::setButtonEventCallback(std::function<void(int, int)> callback)
{
	callbackButtonEvent = callback;
}
