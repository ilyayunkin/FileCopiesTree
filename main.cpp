#include "mainwindow.h"
#include <QApplication>
#include <QDebug>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    w.show();

    qDebug() << "C++ version: " << __cplusplus;
    qDebug() << "Qt version: " << QT_VERSION_STR;

#if __GNUC__
    qDebug() << "GCC version: " << __MINGW_GCC_VERSION;
#endif

    return a.exec();
}
