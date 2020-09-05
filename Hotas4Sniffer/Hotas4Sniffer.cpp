#include "Hotas4Sniffer.h"

#include <stdio.h>
#include <thread>
#include <functional>

#include <initguid.h>
#include <usbiodef.h>

#include "filters.h"
#include "enum.h"
#include "iocontrol.h"

struct DataHeader
{
	pcaprec_hdr_s recordHeader;
	USBPCAP_BUFFER_PACKET_HEADER packetHeader;
};

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
	filters_initialize();

	if (usbpcapFilters[0] == NULL)
	{
		printf("No filter control devices are available.\n");

		if (is_usbpcap_upper_filter_installed() == FALSE)
		{
			printf("Please reinstall USBPcap Driver.\n");
			return false;
		}
	}


	int i = 0;
	int indexMatch = 0; // 1 based, not 0

	auto findHotas4Device = [](HANDLE hub, ULONG port, USHORT deviceAddress, PUSB_DEVICE_DESCRIPTOR desc, void *ctx)
	{
		const USHORT idVendor = 1103;
		const USHORT idProduct = 46716;

		if (desc->idVendor == idVendor &&
			desc->idProduct == idProduct)
		{
			int* index = reinterpret_cast<int*>(ctx);
			if (index)
			{
				*index = port;
			}
		}
	};

	while (usbpcapFilters[i] != NULL)
	{
		enumerate_all_connected_devices(usbpcapFilters[i]->device, findHotas4Device, &indexMatch);

		if (indexMatch > 0)
		{
			deviceAddr = usbpcapFilters[i]->device;
			return true;
		}

		i++;
	}

	return false;
}

bool Hotas4Sniffer::start()
{
	USBPCAP_ADDRESS_FILTER filter;
	DWORD bytes_ret = 0;

	if (USBPcapInitAddressFilter(&filter, NULL, TRUE) == FALSE)
	{
		return false;
	}

	deviceHandle = CreateFileA(deviceAddr, GENERIC_READ | GENERIC_WRITE, 0, 0, OPEN_EXISTING, FILE_FLAG_OVERLAPPED, 0);
	if (deviceHandle == INVALID_HANDLE_VALUE)
	{
		printf("Couldn't open device: %d\n", GetLastError());
		return false;
	}


	if (DeviceIoControl(deviceHandle, IOCTL_USBPCAP_SET_SNAPLEN_SIZE, &snaplen, sizeof(snaplen), NULL, 0, &bytes_ret, 0) == false)
	{
		printf("DeviceIoControl failed with %d status (supplimentary code %d)\n", GetLastError(), bytes_ret);
		goto FINISH;
	}

	if (DeviceIoControl(deviceHandle, IOCTL_USBPCAP_SETUP_BUFFER, &bufferlen, sizeof(bufferlen), NULL, 0, &bytes_ret, 0) == false)
	{
		printf("DeviceIoControl failed with %d status (supplimentary code %d)\n", GetLastError(), bytes_ret);
		goto FINISH;
	}

	if (DeviceIoControl(deviceHandle, IOCTL_USBPCAP_START_FILTERING, &filter, sizeof(filter), NULL, 0, &bytes_ret, 0) == false)
	{
		printf("DeviceIoControl failed with %d status (supplimentary code %d)\n", GetLastError(), bytes_ret);
		goto FINISH;
	}


	running = true;

	std::thread(std::bind(&Hotas4Sniffer::readDataFromDevice, this)).detach();
	return true;

FINISH:
	if (deviceHandle != INVALID_HANDLE_VALUE)
	{
		CloseHandle(deviceHandle);
	}

	return false;
}

void Hotas4Sniffer::stop()
{
	running = false;
}

bool Hotas4Sniffer::isRunning()
{
	return running;
}

void Hotas4Sniffer::readDataFromDevice()
{
	OVERLAPPED readOverlapped;
	HANDLE readHandle;

	unsigned char* buffer = new unsigned char[bufferlen];
	memset(buffer, 0, bufferlen);

	memset(&readOverlapped, 0, sizeof(readOverlapped));
	readOverlapped.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
	readHandle = readOverlapped.hEvent;

	ReadFile(deviceHandle, buffer, bufferlen, NULL, &readOverlapped);

	while (running)
	{
		DWORD dw = WaitForSingleObject(readHandle, INFINITE);
		DWORD read;

		if (dw == WAIT_OBJECT_0)
		{
			GetOverlappedResult(deviceHandle, &readOverlapped, &read, TRUE);
			ResetEvent(readOverlapped.hEvent);

			processData(buffer, read);

			ReadFile(deviceHandle, buffer, bufferlen, &read, &readOverlapped);
		}
		else if (dw == WAIT_FAILED)
		{
			fprintf(stderr, "WaitForMultipleObjects failed in read_thread(): %d", GetLastError());
			break;
		}
	}

	CancelIo(deviceHandle);
	CloseHandle(deviceHandle);
	CloseHandle(readOverlapped.hEvent);

	if (buffer != NULL)
	{
		delete[] buffer;
	}
}

void Hotas4Sniffer::processData(unsigned char* buffer, DWORD bytes)
{
	// beginning with a USBPcap header
	if (bytes == sizeof(pcap_hdr_s))
	{
		// TODO: version handling
		return;
	}

	DataHeader header;
	memcpy(&header, buffer, sizeof(header));

	buffer += sizeof(header);
	bytes -= sizeof(header);

	if (header.packetHeader.function != URB_FUNCTION_BULK_OR_INTERRUPT_TRANSFER ||
		header.packetHeader.dataLength == 0)
	{
		return;
	}

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
