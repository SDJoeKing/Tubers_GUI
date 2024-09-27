#include "mainwindow.h"
#include <QApplication>
#include <QStyleFactory>
#include "styleConfig.h"

extern QString styles;

int main(int argc, char *argv[])
{
    // auto style = QApplication::setStyle("fusion");

    if(true){
    // QApplication::setDesktopSettingsAware(false);
    QApplication a(argc, argv);
    // try
    // {
    //     a.setStyleSheet(styles);
    // }catch(...)
    // {
        QApplication::setStyle("fusion");
    // }

    MainWindow w;
    w.show();

    return a.exec();
    }
    return 0;
}
