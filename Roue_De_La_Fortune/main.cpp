#include "ui/connection.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    Connection* w = new Connection();
    w->show();

    return a.exec();
}
