#include "stdafx.h"
#include "Dialog.h"
#include "ACServoMotorHelper.h"

#define DIALOG_TITLE "Motion Simulator by Hotas 4"

Dialog::Dialog()
{
	initialize();
}

Dialog::~Dialog()
{
}

void Dialog::initialize()
{
	installEventFilter(this);

	controller.setAxisEventCallback([this](const Axis& axis)
	{
		

		needUpdateUI = true;
	});

	controller.setLeftButtonsEventCallback([this](int buttons, int type)
	{


		needUpdateUI = true;
	});

	controller.setRightButtonsEventCallback([this](int buttons, int type)
	{
		// HAT buttons
		switch (buttons)
		{
			case (int)RightButtons::SWITCH_HAT_CENTER:
			{
				printf("  C  %d  \n", rand());
				break;
			}
			case (int)RightButtons::SWITCH_HAT_LEFT:
			{
				printf("  LEFT  \n");
				break;
			}
			case (int)RightButtons::SWITCH_HAT_RIGHT:
			{
				printf("  RIGHT  \n");
				break;
			}
			case (int)RightButtons::SWITCH_HAT_UP:
			{
				printf("  UP  \n");
				break;
			}
			case (int)RightButtons::SWITCH_HAT_DOWN:
			{
				printf("  DOWN  \n");
				break;
			}
		}

		//Sleep(1000);

		needUpdateUI = true;
	});

	timerUpdateUI = new QTimer;
	connect(timerUpdateUI, &QTimer::timeout, [this]()
	{
		if (needUpdateUI)
			updateUI();

		

		needUpdateUI = false;
	});

	{
		auto groupBox = new QGroupBox("Motor");

		motorLayout = new QVBoxLayout;
		motorLayout->addWidget(groupBox);

		{
			auto buttonConnect = new QPushButton("Connect");
			buttonConnect->setFocusPolicy(Qt::FocusPolicy::NoFocus);
			buttonConnect->setCheckable(true);
			buttonConnect->setFixedWidth(100);
			buttonConnect->setFixedHeight(100);

			auto buttonMoveCenter = new QPushButton("Move\nCenter");
			buttonMoveCenter->setFocusPolicy(Qt::FocusPolicy::NoFocus);
			buttonMoveCenter->setFixedWidth(100);
			buttonMoveCenter->setFixedHeight(100);

			auto buttonMoveZero = new QPushButton("Move\nZero");
			buttonMoveZero->setFocusPolicy(Qt::FocusPolicy::NoFocus);
			buttonMoveZero->setFixedWidth(100);
			buttonMoveZero->setFixedHeight(100);

			connect(buttonConnect, &QPushButton::toggled, [this](bool checked)
			{
				if (checked)
				{
					int i = 0;
					for (auto& serialPort : serialPorts)
					{
						if (serialPort->connect(portNames[i], 115200, QSerialPort::OddParity) == false)
						{
							printf("ERROR: motor connect failed: %d\n", i);
							break;
						}

						i++;
					}

					if (i == serialPorts.size())
					{
						return;
					}
				}

				for (auto& serialPort : serialPorts)
				{
					serialPort->disconnect();
				}
			});

			connect(buttonMoveCenter, &QPushButton::clicked, [this]()
			{
				if (timerUpdateUI->isActive())
				{
					return;
				}

				for (size_t i = 0; i < serialPorts.size(); i++)
				{
					serialPorts[i]->writeAndRead(ACServoMotorHelper::setPosition(0, centerPositions[i]));
					serialPorts[i]->writeAndRead(ACServoMotorHelper::trigger(0));
					serialPorts[i]->writeAndRead(ACServoMotorHelper::normal());
				}
			});

			connect(buttonMoveZero, &QPushButton::clicked, [this]()
			{
				if (timerUpdateUI->isActive())
				{
					return;
				}

				for (size_t i = 0; i < serialPorts.size(); i++)
				{
					serialPorts[i]->writeAndRead(ACServoMotorHelper::setPosition(0, -centerPositions[i]));
					serialPorts[i]->writeAndRead(ACServoMotorHelper::trigger(0));
					serialPorts[i]->writeAndRead(ACServoMotorHelper::normal());
				}
			});

			auto layout = new QHBoxLayout;
			layout->setAlignment(Qt::AlignLeft);
			layout->addWidget(buttonConnect);
			layout->addWidget(buttonMoveCenter);
			layout->addWidget(buttonMoveZero);

			groupBox->setLayout(layout);
		}
	}

	{
		auto groupBox = new QGroupBox("Controller");

		controllerLayout = new QVBoxLayout;
		controllerLayout->addWidget(groupBox);

		{
			auto buttonStart = new QPushButton("Start");
			buttonStart->setFocusPolicy(Qt::FocusPolicy::NoFocus);
			buttonStart->setCheckable(true);
			buttonStart->setFixedWidth(100);
			buttonStart->setFixedHeight(100);

			connect(buttonStart, &QPushButton::toggled, [this, buttonStart](bool checked)
			{
				if (checked)
				{
					if (controller.findDevice() == false || controller.start() == false)
					{
						printf("Controller start failed. Use keyboard instead.\n");
					}

					timerUpdateUI->start();
				}
				else
				{
					controller.stop();
					timerUpdateUI->stop();
				}
			});

			auto layout = new QHBoxLayout;
			layout->setAlignment(Qt::AlignLeft);
			layout->addWidget(buttonStart);

			groupBox->setLayout(layout);
		}
	}

	mainLayout = new QVBoxLayout(this);
	mainLayout->addLayout(motorLayout);
	mainLayout->addLayout(controllerLayout);

	setMinimumWidth(600);

	setWindowTitle(DIALOG_TITLE);
	setWindowFlag(Qt::WindowMinimizeButtonHint);
}

void Dialog::updateUI()
{
}

bool Dialog::loadOption()
{
	bool retval = true;

	QString filepath = QCoreApplication::applicationDirPath();
	QFile loadFile(filepath + "/option.txt");

	if (loadFile.open(QIODevice::ReadOnly))
	{
		QJsonDocument doc = QJsonDocument::fromJson(loadFile.readAll());
		if (doc.isNull())
		{
			retval = false;
		}
		else
		{
			QJsonObject optionObject = doc.object();
			QJsonArray optionArray = optionObject["motors"].toArray();

			int numMotors = optionArray.size();
			portNames.reserve(numMotors);
			serialPorts.reserve(numMotors);
			centerPositions.reserve(numMotors);

			for (auto it = optionArray.begin(); it != optionArray.end(); ++it)
			{
				QJsonObject object = it->toObject();
				portNames.push_back(object["port"].toString());
				centerPositions.push_back(object["offset"].toInt());
			}
		}

		loadFile.close();
	}
	else
	{
		retval = false;
	}

	for (size_t i = 0; i < portNames.size(); i++)
	{
		auto serialPort = new SerialPort;
		serialPorts.emplace_back(serialPort);
	}

	return retval;
}

bool Dialog::saveOption()
{
	QString filepath = QCoreApplication::applicationDirPath();
	QFile saveFile(filepath + "/option.txt");

	QJsonDocument doc;
	QJsonObject optionObject;
	QJsonArray optionArray;

	int i = 0;
	for (auto& portName : portNames)
	{
		QJsonObject object;
		object["port"] = portName;
		object["offset"] = 10000;

		optionArray.insert(i++, object);
	}

	optionObject["motors"] = optionArray;
	optionObject["angle"] = 2000;
	optionObject["speed"] = 1000;

	if (saveFile.open(QIODevice::WriteOnly | QIODevice::Truncate) == false)
		return false;

	doc.setObject(optionObject);
	saveFile.write(doc.toJson(QJsonDocument::JsonFormat::Indented));
	saveFile.close();

	return true;
}

bool Dialog::eventFilter(QObject* object, QEvent* event)
{
	QKeyEvent* keyEvent = dynamic_cast<QKeyEvent*>(event);
	if (keyEvent)
	{
		if (keyEvent->key() == Qt::Key_Escape)
		{
			return true;
		}

		if (event->type() == QEvent::KeyRelease)
		{
			keyPressEvent(keyEvent);
			return true;
		}
	}

	return __super::eventFilter(object, event);
}

void Dialog::keyPressEvent(QKeyEvent* event)
{
	if (event->isAutoRepeat() || timerUpdateUI->isActive() == false)
	{
		return;
	}

	std::vector<int> currentPositions(serialPorts.size());

	while (1)
	{
		for (size_t i = 0; i < serialPorts.size();)
		{
			auto received = serialPorts[i]->writeAndRead(ACServoMotorHelper::readEncoder());

			bool complete = false;
			if (ACServoMotorHelper::getEncoderValue(received, currentPositions[i], complete) == false)
			{
				printf("ERROR: getEncoderValue()\n");

				for (auto& c : received)
					printf("%x ", c);
				printf("\n\n");

				return;
			}

			if (complete) i++;
		}
	}

	if (event->type() == QEvent::KeyRelease)
	{
		printf("center\n");
	}
	else
	{
		switch (event->key())
		{
			case Qt::Key_Left:
			{
				printf("left\n");
				break;
			}
			case Qt::Key_Right:
			{
				printf("right\n");
				break;
			}
			case Qt::Key_Up:
			{
				printf("up\n");
				break;
			}
			case Qt::Key_Down:
			{
				printf("down\n");
				break;
			}
		}
	}
}

void Dialog::closeEvent(QCloseEvent* event)
{
	saveOption();

	for (auto& serialPort : serialPorts)
	{
		delete serialPort;
	}

	serialPorts.clear();

	__super::closeEvent(event);
}
