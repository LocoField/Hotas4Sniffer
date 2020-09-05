#pragma once

class Axis
{
public:
	Axis()
	{
		roll = 0;
		pitch = 0;
		yaw = 0;
	}

	Axis(float x, float y, float z)
	{
		roll = x;
		pitch = y;
		yaw = z;
	}

public:
	bool operator==(const Axis& other) const
	{
		return roll == other.roll && pitch == other.pitch && yaw == other.yaw;
	}

	bool operator!=(const Axis& other) const
	{
		return !(*this == other);
	}

public:
	static Axis fromHotas4Handle(int x, int y, int z)
	{
		if (x == 0x3FF) x = 1024;
		if (y == 0x3FF) y = 1024;
		if (z == 0xFF) z = 256;

		x -= 512;
		y -= 512;
		z -= 128;

		return Axis(x / (float)512, y / (float)512, z / (float)128);
	}

public:
	float roll;
	float pitch;
	float yaw;

};
