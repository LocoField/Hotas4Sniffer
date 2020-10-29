#pragma once

#include <QtSerialPort/QSerialPort>

class QWidget;
class QLayout;

class CommandSet;

class SerialPort final : protected QSerialPort
{
public:
	SerialPort();
	~SerialPort() = default;

public:
	static void availablePorts(std::vector<QString>& ports);

public:
	bool connect(QString portName, int baudRate, int mode = 0);
	void disconnect();
	bool isConnected();

	QByteArray read(int timeout = 2000);
	qint64 write(const QByteArray& data);

	bool write(char code);
	bool read(char& code, int timeout = 2000);

};

