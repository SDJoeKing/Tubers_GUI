#ifndef STYLECONFIG_H
#define STYLECONFIG_H

#include <QString>
int r = 236;
int g = 240;
int b = 241;
int alpha = 255;
QString styles =
    QString("QWidget\
        {\
        background-color:rgba(%1, %2, %3, %4);\
        }\
    QFrame \
        { \
        background-color:rgba(%1, %2, %3, %4); \
        border: 0.5px solid black; \
        border-radius: 4px;\
        padding: 2px;\
        } \
    QLineEdit,QTextEdit\
        {\
        background-color:white;\
        color: black;\
        }\
    QLabel \
        { \
        border: 0px;\
        color: black;\
        }\
    QSpinBox, QDoubleSpinBox\
        {\
        background-color:white;\
        color:black;\
        }\
    QPushButton\
        {\
        margin:0px;\
        border: 2px solid black;\
        border-radius: 2px;\
        padding:3px;\
        background-color:white;\
        color:black;\
        }\
    QPushButton:checked\
        {\
        border-bottom:4px solid blue;\
        border-style:groove;\
        }\
     QPushButton:disabled\
        {\
        border-style:groove;\
        background-color:lightgrey;\
        }\
    QToolBar\
        {\
        background-color:rgba(%1, %2, %3, %4); \
        color:black;\
        }\
    QToolButton\
    {\
        background-color: white;\
        margin:5px;\
        border: 2px solid black;\
        border-radius: 2px;\
        padding:3px;\
        color:black;\
    }\
    QToolButton:checked\
    {\
        border-bottom:4px solid blue;\
        border-style:groove;\
    }\
    QCheckBox\
    {\
        color:black;\
    }\
    QCheckBox::indicator\
    {\
        border:2px solid black;\
        background-color:white;\
    }\
     QCheckBox::indicator:checked\
     {\
        background-color:blue;\
     }\
     QCheckBox::indicator:unchecked\
     {\
        background-color:transparent;\
     }\
    QStatusBar\
    {\
        background-color:rgba(%1, %2, %3, %4);\
    }\
    QMenuBar\
    {\
        color:black;\
    }\
    QMenu\
    {\
        border:2px solid black;\
        color:black;\
    }"
).arg(r).arg(g).arg(b).arg(alpha);

#endif // STYLECONFIG_H
