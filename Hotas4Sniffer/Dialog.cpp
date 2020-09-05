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

	{
		auto groupBox = new QGroupBox;

		settingLayout = new QVBoxLayout;
		settingLayout->addWidget(groupBox);

		{
			auto layout = new QHBoxLayout;

			auto button1 = new QPushButton("Controller");
			button1->setCheckable(true);
			button1->setFixedWidth(100);
			button1->setFixedHeight(100);

			auto button2 = new QPushButton("Motor");
			button2->setFixedWidth(100);
			button2->setFixedHeight(100);

			connect(button1, &QPushButton::toggled, [this, button1](bool checked)
			{
				if (checked)
				{
					if (controller.findDevice() == false || controller.start() == false)
					{
						button1->setChecked(false);
					}
				}
				else
				{
					controller.stop();
				}
			});

			layout->addWidget(button1);
			layout->addWidget(button2);

			groupBox->setLayout(layout);
		}
	}

	mainLayout = new QVBoxLayout(this);
	mainLayout->addLayout(settingLayout);

	setWindowTitle(DIALOG_TITLE);
	setWindowFlag(Qt::WindowMinimizeButtonHint);
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
