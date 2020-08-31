#include "Hotas4Sniffer.h"

#include <iostream>

using namespace std;

int main()
{
	Hotas4Sniffer controller;

	if (controller.findDevice() == false)
	{
		cout << "Could NOT find the contoller." << endl;
		return -1;
	}

	controller.start();

	while (controller.isRunning())
	{
		Sleep(1000);
	}

	return 0;
}
