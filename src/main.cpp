#include "Widget\Widget.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    Widget w;
    w.show();

    return a.exec();
}
/*
    因为在写AI
    所以在Widget1.cpp中的
        138-148注释了，是关于定时器时间到了自动pass的事件
*/
