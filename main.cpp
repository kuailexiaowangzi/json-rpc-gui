#include "widget.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    //High resolution screen.
    QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QGuiApplication::setHighDpiScaleFactorRoundingPolicy(Qt::HighDpiScaleFactorRoundingPolicy::PassThrough);
    QApplication a(argc, argv);
    Widget w;
    w.show();
    return a.exec();
}
