#pragma once

#include "Axis.h"

#include <USBPCapHelper.h>
#include <functional>

#ifdef _DEBUG
#pragma comment(lib, "USBPcapHelperd.lib")
#else
#pragma comment(lib, "USBPcapHelper.lib")
#endif

/* for buttons2 */
enum class LeftButtons
{
	BUTTON_NONE = 0,
	BUTTON_RECTANGLE = 1,
	BUTTON_CROSS = 2,
	BUTTON_CIRCLE = 4,
	BUTTON_TRIANGLE = 8,

	BUTTON_R2 = 10,
	BUTTON_L2 = 20,
	BUTTON_SHARE = 40,
	BUTTON_OPTIONS = 80,
};

/* for buttons1 */
enum class RightButtons
{
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
};

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
	void onLeftButtonsEvent(int buttons); // TODO: multi buttons handling
	void onRightButtonsEvent(int buttons);

	void setAxisEventCallback(std::function<void(const Axis&)> callback);
	void setLeftButtonsEventCallback(std::function<void(int, int)> callback);
	void setRightButtonsEventCallback(std::function<void(int, int)> callback);

private:
	Axis lastAxis;
	int lastButtonsLeft = (int)LeftButtons::BUTTON_NONE;
	int lastButtonsRight = (int)RightButtons::SWITCH_HAT_CENTER;

	std::function<void(const Axis&)> callbackAxisEvent = nullptr;
	std::function<void(int, int)> callbackLeftButtonsEvent = nullptr;
	std::function<void(int, int)> callbackRightButtonsEvent = nullptr;

};
