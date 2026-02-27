#include <QApplication>
#ifdef Q_OS_WIN
#include <windows.h>
#endif

#include <widget/capturewindow.h>


int main(int argc, char* argv[])
{
    // 设置Windows控制台编码
#ifdef Q_OS_WIN
    // 设置控制台输出代码页为 UTF-8
    SetConsoleOutputCP(CP_UTF8);
    // 可选：设置控制台输入为 UTF-8
    SetConsoleCP(CP_UTF8);
#endif

    QApplication app(argc, argv);

    // 应用级图标（任务栏/Alt-Tab/托盘等更统一）
    app.setWindowIcon(QIcon(":/icon/img.png"));
    CaptureWindow main;

    main.show();
    app.exec();
    return 0;
}
