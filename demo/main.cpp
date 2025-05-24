#include <QApplication>

#include <iostream>

#include "Editor.h"


int main(int argc, char**argv){

    QApplication app(argc, argv);

    Editor editor("editor");

    return app.exec();
}
