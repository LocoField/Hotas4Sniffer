#pragma once

#include "Axis.h"

#include <USBPCapHelper.h>
#include <functional>

#ifdef _DEBUG
#pragma comment(lib, "USBPcapHelperd.lib")
#else
#pragma comment(lib, "USBPcapHelper.lib")
#endif

class Hotas4Sniffer : public USBPcapHelper
{
public:
	Hotas4Sniffer();
	virtual ~Hotas4Sniffer() = default;

public:
	bool findDevice();

protected:
	virtual void processInterruptData(unsigned char* buffer, DWORD bytes) override;

public:
	void onAxisEvent(const Axis& axis);
	void onButtonEvent(int type, int buttonId);

	void setAxisEventCallback(std::function<void(const Axis&)> callback);
	void setButtonEventCallback(std::function<void(int, int)> callback);

private:
	Axis lastAxis;

	std::function<void(const Axis&)> callbackAxisEvent = nullptr;
	std::function<void(int, int)> callbackButtonEvent = nullptr;

};
