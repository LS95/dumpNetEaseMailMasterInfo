#include "widget.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    Widget w;
    w.setWindowTitle("网易邮箱大师数据恢复工具");
    w.show();
    return a.exec();
}
