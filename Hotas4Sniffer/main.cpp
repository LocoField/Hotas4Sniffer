#include "Dialog.h"

#include <QtWidgets/QApplication>

using namespace std;

int main(int argc, char** argv)
{
	QApplication app(argc, argv);

	Dialog dialog;
	dialog.loadOption();

	return dialog.exec();
}
