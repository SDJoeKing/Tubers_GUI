#include "mainwindow.h"
#include <QApplication>
#include <QStyleFactory>
#include "styleConfig.h"

extern QString styles;

int main(int argc, char *argv[])
{
    auto style = QApplication::setStyle("fusion");


    // // QApplication::setDesktopSettingsAware(false);
    QApplication a(argc, argv);

    //     a.setStyleSheet(styles);


    MainWindow w;

    w.show();

    return a.exec();

}
