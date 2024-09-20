#include "mainwindow.h"
#include <QApplication>
#include <QStyleFactory>

int main(int argc, char *argv[])
{
    auto style = QApplication::setStyle("fusion");

    if(style){
    // QApplication::setDesktopSettingsAware(false);
    QApplication a(argc, argv);

    MainWindow w;
    w.show();

    return a.exec();
    }
    return 0;
}
