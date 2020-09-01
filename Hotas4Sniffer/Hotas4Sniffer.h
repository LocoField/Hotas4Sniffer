#pragma once

#include <Windows.h>

#define DEFAULT_SNAPSHOT_LENGTH             (65535)
#define DEFAULT_INTERNAL_KERNEL_BUFFER_SIZE (1024*1024)

class Hotas4Sniffer
{
public:
	Hotas4Sniffer();
	virtual ~Hotas4Sniffer() = default;

public:
	bool findDevice();

protected:
	void readDataFromDevice();
	void processData(unsigned char* data, DWORD bytes);

public:
	bool start();
	void stop();
	bool isRunning();

private:
	unsigned int snaplen = DEFAULT_SNAPSHOT_LENGTH;
	unsigned int bufferlen = DEFAULT_INTERNAL_KERNEL_BUFFER_SIZE;

	char* deviceAddr = nullptr;
	HANDLE deviceHandle;

	bool running = false;

};
