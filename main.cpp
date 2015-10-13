#include <QApplication>
#include "mainwindow.h"

/**
 * \file main.cpp
 * \brief Fichier main
 */

int main (int argc, char* argv[]){
    QApplication app(argc, argv);
    MainWindow mw;
    mw.show();
    return app.exec();
}
