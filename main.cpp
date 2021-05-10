#include "ui.h"
#include <QApplication>

int main(int argc, char** argv)
{
    QApplication app(argc, argv);

    Ui ui;
    ui.show();

    return app.exec();
}


