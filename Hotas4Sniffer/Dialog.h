#pragma once

#include "Hotas4Sniffer.h"

#include <QtWidgets/QDialog>
#include <QtWidgets/QBoxLayout>

class QSerialPort;

class Dialog : public QDialog
{
public:
	Dialog();
	virtual ~Dialog();

protected:
	void initialize();

public:
	bool loadOption();
	bool saveOption();

private:
	bool eventFilter(QObject* object, QEvent* event) override;
	void closeEvent(QCloseEvent* event) override;

private:
	QVBoxLayout* mainLayout;
	QVBoxLayout* settingLayout;

	Hotas4Sniffer controller;

	std::vector<QSerialPort*> serialPorts;

};
