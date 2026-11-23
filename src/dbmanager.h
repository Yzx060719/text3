#ifndef DBMANAGER_H
#define DBMANAGER_H

#include <QSqlDatabase>
#include <QString>

// 数据库访问单例：集中管理 QODBC 连接、错误信息与操作日志
// 参考：https://stackoverflow.com/questions/1008019/c-singleton-design-pattern
class DatabaseManager
{
public:
    static DatabaseManager &instance();

    bool open(QString *errorMessage = nullptr);
    bool isOpen() const { return m_opened; }
    // 允许通过命令行 --db=xxx 指定数据库名，便于演示连接失败等异常场景
    static void setDatabaseOverride(const QString &databaseName);
    QSqlDatabase database() const { return m_db; }
    QString lastError() const { return m_lastError; }
    QString connectionString() const { return m_connectionString; }

    // 记录一条操作日志（选做功能：操作记录留存与查询）
    bool logEvent(const QString &userName, const QString &event, const QString &detail);

    static QString sha256(const QString &text);

private:
    DatabaseManager() = default;
    Q_DISABLE_COPY(DatabaseManager)

    QSqlDatabase m_db;
    QString m_lastError;
    QString m_connectionString;
    bool m_opened = false;
};

#endif // DBMANAGER_H
