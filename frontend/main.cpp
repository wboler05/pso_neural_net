#include "mainwindow.h"
#include <QApplication>

// Entry Point
int main(int argc, char *argv[])
{
    QApplication a(argc, argv); //does all Qt stuff
    MainWindow w;
    w.show();

    return a.exec();
}
