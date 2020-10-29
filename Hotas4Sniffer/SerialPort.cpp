#include "stdafx.h"
#include "SerialPort.h"

SerialPort::SerialPort()
{
	QObject::connect(this, &QIODevice::readyRead, [&]()
	{
		QByteArray data = readAll();
		QString command;

		for (auto it = data.cbegin(); it != data.cend(); ++it)
		{
			unsigned char hex = *it;
			QString hex_format = QString("%1 ").arg(hex, 2, 16, QLatin1Char('0'));

			command.append(hex_format);
		}

		cout << "recv: " << command.toStdString() << endl;
	});

	QObject::connect(this, &QSerialPort::errorOccurred, [&](QSerialPort::SerialPortError error)
	{
		if (error == QSerialPort::SerialPortError::ResourceError)
		{
			printf("ERROR: device disconnected.\n");
		}
	});
}

void SerialPort::availablePorts(std::vector<QString>& ports)
{
	auto p = QSerialPortInfo::availablePorts();

	ports.clear();
	ports.resize(p.size());

	std::transform(p.cbegin(), p.cend(), ports.begin(), [](const QSerialPortInfo& info)
	{
		return info.portName();
	});
}

bool SerialPort::connect(QString portName, int baudRate, int mode)
{
	setPortName(portName);
	setBaudRate(baudRate);
	setDataBits(QSerialPort::Data8);
	setFlowControl(QSerialPort::NoFlowControl);
	setParity(QSerialPort::NoParity);
	setStopBits(QSerialPort::OneStop);

	switch (mode)
	{
		case 1:
		{
			setParity(QSerialPort::EvenParity);
			break;
		}
		case 2:
		{
			setParity(QSerialPort::OddParity);
			break;
		}
	}

	return __super::open(QIODevice::ReadWrite);
}

void SerialPort::disconnect()
{
	__super::close();
}

bool SerialPort::isConnected()
{
	return __super::isOpen();
}

QByteArray SerialPort::read(int timeout)
{
	if (waitForReadyRead(timeout))
	{
		return __super::readAll();
	}

	return QByteArray();
}

qint64 SerialPort::write(const QByteArray& data)
{
	qint64 retval = __super::write(data);
	waitForBytesWritten();
	return retval;
}

bool SerialPort::write(char code)
{
	bool re = putChar(code);
	waitForBytesWritten();
	return re;
}

bool SerialPort::read(char& code, int timeout)
{
	if (waitForReadyRead(timeout))
	{
		getChar(&code);
		return true;
	}

	return false;
}
