#include "Mis2x265.h"

#include <QtGui>
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    Mis2x265 w;
    w.show();
    return a.exec();
}
