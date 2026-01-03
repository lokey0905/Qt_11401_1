#include "ControlPanel.h"
#include <QApplication>
#include <QIcon>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    ControlPanel w;
    a.setWindowIcon(QIcon(":/config/system_call.png"));
    w.show();

    return a.exec();
}
