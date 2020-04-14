#include "ui/connection.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    Connection w;
    w.show();

    return a.exec();
}
