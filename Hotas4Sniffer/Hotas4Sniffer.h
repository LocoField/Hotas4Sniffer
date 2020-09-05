#pragma once

#include "Axis.h"

#include <Windows.h>
#include <functional>

#define DEFAULT_SNAPSHOT_LENGTH             (65535)
#define DEFAULT_INTERNAL_KERNEL_BUFFER_SIZE (1024*1024)

class Hotas4Sniffer
{
public:
	Hotas4Sniffer();
	virtual ~Hotas4Sniffer() = default;

public:
	bool findDevice();
	bool start();
	void stop();
	bool isRunning();

protected:
	void readDataFromDevice();
	void processData(unsigned char* data, DWORD bytes);

public:
	void onAxisEvent(const Axis& axis);
	void onButtonEvent(int type, int buttonId);

	void setAxisEventCallback(std::function<void(const Axis&)> callback);
	void setButtonEventCallback(std::function<void(int, int)> callback);

private:
	unsigned int snaplen = DEFAULT_SNAPSHOT_LENGTH;
	unsigned int bufferlen = DEFAULT_INTERNAL_KERNEL_BUFFER_SIZE;

	char* deviceAddr = nullptr;
	HANDLE deviceHandle;

	bool running = false;

	Axis lastAxis;

	std::function<void(const Axis&)> callbackAxisEvent = nullptr;
	std::function<void(int, int)> callbackButtonEvent = nullptr;

};
