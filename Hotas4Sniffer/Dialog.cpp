#include "stdafx.h"
#include "Dialog.h"
#include "ACServoMotorHelper.h"

#define DIALOG_TITLE "Motion Simulator by Hotas 4"

Dialog::Dialog()
{
	initialize();
	loadOption();
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

			auto buttonMoveCenter = new QPushButton("Motor\n\nInitialize");
			buttonMoveCenter->setFocusPolicy(Qt::FocusPolicy::NoFocus);
			buttonMoveCenter->setCheckable(true);
			buttonMoveCenter->setFixedWidth(100);
			buttonMoveCenter->setFixedHeight(100);

			connect(buttonConnect, &QPushButton::toggled, [this, buttonConnect](bool checked)
			{
				if (checked)
				{
					bool connect = motor.connect(portName);

					if (connect == false)
					{
						printf("ERROR: motor connect failed: %s\n", portName.toStdString().c_str());

						buttonConnect->setChecked(false);
						return;
					}

					for (int i = 0; i < numMotors; i++)
					{
						auto data = motor.writeAndRead(ACServoMotorHelper::setSpeed(speed, i + 1));

						if (data.empty())
						{
							printf("ERROR: motor command failed: %d\n", i);

							buttonConnect->setChecked(false);
							return;
						}
					}
				}
				else
				{
					motor.disconnect();

					numMotors = 0;
				}
			});

			connect(buttonMoveCenter, &QPushButton::clicked, [this](bool checked)
			{
				if (checked)
				{
					for (int i = 0; i < numMotors; i++)
					{
						motor.writeAndRead(ACServoMotorHelper::setPosition(centerPositions[i], i + 1));
						motor.writeAndRead(ACServoMotorHelper::trigger(i + 1));
						motor.writeAndRead(ACServoMotorHelper::normal(i + 1));
					}
				}
				else
				{
					for (int i = 0; i < numMotors; i++)
					{
						auto received = motor.writeAndRead(ACServoMotorHelper::readEncoder(i + 1));

						int position = 0;
						bool complete = false;
						if (ACServoMotorHelper::getEncoderValue(received, position, complete) == false)
						{
							printf("ERROR: getEncoderValue()\n");

							for (auto& c : received)
								printf("%x ", c);
							printf("\n\n");

							return;
						}

						motor.writeAndRead(ACServoMotorHelper::setPosition(-position, i + 1));
						motor.writeAndRead(ACServoMotorHelper::trigger(i + 1));
						motor.writeAndRead(ACServoMotorHelper::normal(i + 1));
					}
				}
			});

			auto layout = new QHBoxLayout;
			layout->setAlignment(Qt::AlignLeft);
			layout->addWidget(buttonConnect);
			layout->addWidget(buttonMoveCenter);

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

			angle = optionObject["angle"].toInt();

			QJsonArray optionArray = optionObject["motors"].toArray();
			{
				numMotors = optionArray.size();
				centerPositions.resize(numMotors);
				limitPositions.resize(numMotors);

				int index = 0;
				for (auto it = optionArray.begin(); it != optionArray.end(); ++it)
				{
					QJsonObject object = it->toObject();
					centerPositions[index] = object["offset"].toInt();
					limitPositions[index] = object["limit"].toInt();

					index++;
				}
			}

			portName = optionObject["port"].toString();
			speed = optionObject["speed"].toInt();
		}

		loadFile.close();
	}
	else
	{
		retval = false;
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

	for (int i = 0; i < numMotors; i++)
	{
		QJsonObject object;
		object["offset"] = centerPositions[i];
		object["limit"] = limitPositions[i];

		optionArray.insert(i, object);
	}

	optionObject["angle"] = angle;
	optionObject["motors"] = optionArray;
	optionObject["port"] = portName;
	optionObject["speed"] = speed;

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
	if (timerUpdateUI->isActive() == false)
	{
		return;
	}

	if (event->isAutoRepeat())
	{
		return;
	}

	int heave = 0;

	if (event->type() == QEvent::KeyRelease)
	{
		switch (event->key())
		{
			case Qt::Key_Left:
			{
				if (rollMoved != -1)
					return;

				rollMoved = 0;

				break;
			}
			case Qt::Key_Right:
			{
				if (rollMoved != 1)
					return;

				rollMoved = 0;

				break;
			}
			case Qt::Key_Up:
			{
				if (pitchMoved != -1)
					return;

				pitchMoved = 0;

				break;
			}
			case Qt::Key_Down:
			{
				if (pitchMoved != 1)
					return;

				pitchMoved = 0;

				break;
			}
		}
	}
	else
	{
		switch (event->key())
		{
			case Qt::Key_W:
			{
				heave = 1;

				printf("heave up\n");

				break;
			}
			case Qt::Key_S:
			{
				heave = -1;

				printf("heave down\n");

				break;
			}
			case Qt::Key_Left:
			{
				if (rollMoved != 0)
					return;

				rollMoved = -1;

				printf("left\n");
				break;
			}
			case Qt::Key_Right:
			{
				if (rollMoved != 0)
					return;

				rollMoved = 1;

				printf("right\n");
				break;
			}
			case Qt::Key_Up:
			{
				if (pitchMoved != 0)
					return;

				pitchMoved = -1;

				printf("up\n");
				break;
			}
			case Qt::Key_Down:
			{
				if (pitchMoved != 0)
					return;

				pitchMoved = 1;

				printf("down\n");
				break;
			}
		}
	}


	if (motor.isConnected() == false)
		return;

	std::vector<int> currentPositions(numMotors);
	int completeCount = 0;

	for (int i = 0; i < numMotors; i++)
	{
		auto received = motor.writeAndRead(ACServoMotorHelper::readEncoder(i + 1));

		bool complete = false;
		if (ACServoMotorHelper::getEncoderValue(received, currentPositions[i], complete) == false)
		{
			printf("ERROR: getEncoderValue()\n");

			for (auto& c : received)
				printf("%x ", c);
			printf("\n\n");

			return;
		}

		printf("%6d    ", currentPositions[i]);

		if (complete)
			completeCount++;
	}

	printf("\n");

	if (completeCount != numMotors)
	{
		printf("Motors are moving.\n");

		return;
	}


	// TODO: 모터가 2개인 경우 heave 값은 무시한다.

	std::vector<int> cycleValues(numMotors);

	for (int i = 0; i < numMotors; i++)
	{
		cycleValues[i] += heave * 10000;
	}

	for (int i = 0; i < numMotors; i++)
	{
		int cycle = cycleValues[i] * reverseOption;

		if (currentPositions[i] + cycle < 0 ||
			currentPositions[i] + cycle >= limitPositions[i])
			cycle = 0;

		motor.writeAndRead(ACServoMotorHelper::setPosition(cycle, i + 1));
	}

	for (int i = 0; i < numMotors; i++)
	{
		motor.writeAndRead(ACServoMotorHelper::trigger(i + 1));
		motor.writeAndRead(ACServoMotorHelper::normal(i + 1));
	}
}

void Dialog::closeEvent(QCloseEvent* event)
{
	saveOption();

	__super::closeEvent(event);
}
