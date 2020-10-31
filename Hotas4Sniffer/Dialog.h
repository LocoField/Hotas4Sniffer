#pragma once

#include "Hotas4Sniffer.h"
#include "SerialPort.h"

#include <QtWidgets/QDialog>

class QVBoxLayout;
class QTimer;

class Dialog : public QDialog
{
public:
	Dialog();
	virtual ~Dialog();

protected:
	void initialize();
	void updateUI();

public:
	bool loadOption();
	bool saveOption();

private:
	bool eventFilter(QObject* object, QEvent* event) override;
	void keyPressEvent(QKeyEvent *) override;
	void closeEvent(QCloseEvent* event) override;

private:
	QVBoxLayout* mainLayout;
	QVBoxLayout* motorLayout;
	QVBoxLayout* controllerLayout;

	Hotas4Sniffer controller;

	// instead of using Q_OBJECT
	QTimer* timerUpdateUI;
	bool needUpdateUI = false;

	std::vector<QString> portNames;
	std::vector<SerialPort*> serialPorts;
	std::vector<int> centerPositions;

	int angle = 2000; // difference
	int speed = 1000; // rpm

};
