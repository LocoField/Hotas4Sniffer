#include "stdafx.h"
#include "Hotas4Sniffer.h"

#include <stdio.h>

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

	int currentLeftButtons = data.buttons2;
	if (lastButtonsLeft != currentLeftButtons)
	{
		onLeftButtonsEvent(currentLeftButtons);
	}

	int currentRightButtons = data.buttons1;
	if (lastButtonsRight != currentRightButtons)
	{
		onRightButtonsEvent(currentRightButtons);
	}

	lastAxis = currentAxis;
	lastButtonsLeft = currentLeftButtons;
	lastButtonsRight = currentRightButtons;
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

void Hotas4Sniffer::onLeftButtonsEvent(int buttons)
{
	if (callbackLeftButtonsEvent)
	{
		callbackLeftButtonsEvent(buttons, 0);
	}
}

void Hotas4Sniffer::onRightButtonsEvent(int buttons)
{
	if (callbackRightButtonsEvent)
	{
		callbackRightButtonsEvent(buttons, 0);
	}
}

void Hotas4Sniffer::setAxisEventCallback(std::function<void(const Axis&)> callback)
{
	callbackAxisEvent = callback;
}

void Hotas4Sniffer::setLeftButtonsEventCallback(std::function<void(int, int)> callback)
{
	callbackLeftButtonsEvent = callback;
}

void Hotas4Sniffer::setRightButtonsEventCallback(std::function<void(int, int)> callback)
{
	callbackRightButtonsEvent = callback;
}
