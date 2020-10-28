#include "stdafx.h"
#include "Dialog.h"

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
			buttonConnect->setCheckable(true);
			buttonConnect->setFixedWidth(100);
			buttonConnect->setFixedHeight(100);

			auto buttonMoveCenter = new QPushButton("Move\nCenter");
			buttonMoveCenter->setFixedWidth(100);
			buttonMoveCenter->setFixedHeight(100);

			auto buttonMoveZero = new QPushButton("Move\nZero");
			buttonMoveZero->setFixedWidth(100);
			buttonMoveZero->setFixedHeight(100);

			connect(buttonConnect, &QPushButton::toggled, [](bool checked)
			{
				if (checked)
				{

				}
				else
				{

				}
			});

			connect(buttonMoveCenter, &QPushButton::clicked, [this]()
			{
				if (timerUpdateUI->isActive())
				{
					return;
				}

				
			});

			connect(buttonMoveZero, &QPushButton::clicked, [this]()
			{
				if (timerUpdateUI->isActive())
				{
					return;
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
			buttonStart->setCheckable(true);
			buttonStart->setFixedWidth(100);
			buttonStart->setFixedHeight(100);

			connect(buttonStart, &QPushButton::toggled, [this, buttonStart](bool checked)
			{
				if (checked)
				{
					if (controller.findDevice() == false || controller.start() == false)
					{
						buttonStart->setChecked(false);
						return;
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
	int n = 1;
	int x = 500;
	int y = 500;
	int w = 600;
	int h = 800;

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
			QJsonObject object = doc.object();

			
		}

		loadFile.close();
	}
	else
	{
		retval = false;
	}

	for (int i = 0; i < n; i++)
	{
		auto serialPort = new QSerialPort;
		serialPorts.emplace_back(serialPort);
	}

	return retval;
}

bool Dialog::saveOption()
{
	QString filepath = QCoreApplication::applicationDirPath();
	QFile loadFile(filepath + "/option.txt");

	if (loadFile.open(QIODevice::ReadOnly) == false)
		return false;

	QJsonDocument doc = QJsonDocument::fromJson(loadFile.readAll());
	if (doc.isNull())
		return false;

	QJsonObject object = doc.object();

	


	loadFile.close();
	if (loadFile.open(QIODevice::WriteOnly | QIODevice::Truncate) == false)
		return false;

	doc.setObject(object);
	loadFile.write(doc.toJson(QJsonDocument::JsonFormat::Indented));
	loadFile.close();

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
	}

	return __super::eventFilter(object, event);
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
