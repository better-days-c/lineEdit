#include <QApplication>
#include <QStyleFactory>
#include <QDir>
#include <QLocale>
#include <QTranslator>
#include "mainwindow.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    // 设置应用程序信息
    app.setApplicationName("测线编辑器");

    // 设置样式
    app.setStyle(QStyleFactory::create("Fusion"));

    // 创建并显示主窗口
    MainWindow window;
    window.show();

    return app.exec();
}
