#include "dbmanager.h"
#include "mainwindow.h"

#include <QApplication>
#include <QFile>
#include <QFont>
#include <QLibraryInfo>
#include <QMessageBox>
#include <QTextStream>
#include <QTranslator>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    QApplication::setApplicationName(QStringLiteral("HospitalMS"));
    QApplication::setOrganizationName(QStringLiteral("DGUT"));

    // 全局字体与样式表
    QFont font = app.font();
    font.setFamily(QStringLiteral("Microsoft YaHei"));
    font.setPointSize(10);
    app.setFont(font);

    // 加载 Qt 中文翻译，使标准对话框按钮（确定/取消）显示为中文
    QTranslator translator;
    const QStringList translationDirs = {
        QCoreApplication::applicationDirPath() + QStringLiteral("/translations"),
        QLibraryInfo::path(QLibraryInfo::TranslationsPath)
    };
    for (const QString &dir : translationDirs) {
        if (translator.load(QStringLiteral("qt_zh_CN"), dir)) {
            app.installTranslator(&translator);
            break;
        }
    }

    QFile styleFile(QStringLiteral(":/style.qss"));
    if (styleFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream stream(&styleFile);
        stream.setEncoding(QStringConverter::Utf8);
        app.setStyleSheet(stream.readAll());
    }

    // 命令行参数：--db=<数据库名> 可覆盖默认库名（用于演示连接失败等异常场景）
    for (const QString &arg : QCoreApplication::arguments()) {
        if (arg.startsWith(QStringLiteral("--db=")))
            DatabaseManager::setDatabaseOverride(arg.mid(5));
    }

    // 先连接数据库，再构建界面：各页面的 Model 在构造时即需要有效连接
    QString error;
    const bool dbReady = DatabaseManager::instance().open(&error);

    MainWindow window;
    window.setDatabaseStatus(dbReady, error);
    window.show();

    if (!dbReady) {
        QMessageBox::critical(&window, QStringLiteral("数据库连接失败"),
                              QStringLiteral("程序已启动，但无法连接数据库，登录功能不可用。\n\n%1\n\n"
                                             "请确认：\n1. SQL Server 服务是否启动；\n"
                                             "2. ODBC Driver 17 for SQL Server 是否安装；\n"
                                             "3. 数据库 HospitalDB 是否存在。")
                                  .arg(error));
    }

    return app.exec();
}
