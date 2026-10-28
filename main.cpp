#include "mainwindow.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    QLoggingCategory::setFilterRules("qt.multimedia.*=false");

    MainWindow w;
    w.show();
    return QApplication::exec();
}
