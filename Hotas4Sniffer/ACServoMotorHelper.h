#pragma once

#include <vector>

typedef std::vector<unsigned char> Command;

class ACServoMotorHelper
{
public:
	ACServoMotorHelper() = default;
	~ACServoMotorHelper() = default;

protected:
	static void calculateCRC(Command& data);

public:
	static int getDataLength(const Command& data);
	static bool checkWriteRegisters(const Command& data);

	static bool getCycleValue(const Command& data, int& cycle); // high * 10000 + low
	static bool getParamValue(const Command& data, int& value);
	static bool getEncoderValue(const Command& data, int& position, bool& complete); // high + low * 2500

	static Command readCycles(int index);
	static Command readGear(); // TODO: refactoring
	static Command readEncoder();

	static Command setPosition(int index, int cycle);

	static Command stop(int index);
	static Command trigger(int index);
	static Command normal();

	static Command emergency(bool on);

};
