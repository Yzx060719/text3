#include "dbmanager.h"

#include <QCryptographicHash>
#include <QSqlError>
#include <QSqlQuery>
#include <QVariant>
#include <qlogging.h>

namespace {
const char *kConnectionName = "hospital_main";
QString s_databaseOverride;
}

void DatabaseManager::setDatabaseOverride(const QString &databaseName)
{
    s_databaseOverride = databaseName;
}

DatabaseManager &DatabaseManager::instance()
{
    static DatabaseManager manager;
    return manager;
}

bool DatabaseManager::open(QString *errorMessage)
{
    m_lastError.clear();

    // 允许通过环境变量 / 命令行覆盖，便于演示"数据库连接失败"的异常分支
    const QString server   = qEnvironmentVariable("HOSPITALMS_SERVER", QStringLiteral("localhost"));
    const QString database = s_databaseOverride.isEmpty()
                                 ? qEnvironmentVariable("HOSPITALMS_DB", QStringLiteral("HospitalDB"))
                                 : s_databaseOverride;

    m_connectionString = QStringLiteral(
        "DRIVER={ODBC Driver 17 for SQL Server};SERVER=%1;DATABASE=%2;Trusted_Connection=yes;")
        .arg(server, database);

    if (QSqlDatabase::contains(QLatin1String(kConnectionName))) {
        m_db = QSqlDatabase::database(QLatin1String(kConnectionName), false);
    } else {
        m_db = QSqlDatabase::addDatabase(QStringLiteral("QODBC"), QLatin1String(kConnectionName));
    }
    m_db.setDatabaseName(m_connectionString);
    m_db.setConnectOptions(QStringLiteral("SQL_ATTR_LOGIN_TIMEOUT=5"));

    if (!m_db.open()) {
        m_opened = false;
        m_lastError = QStringLiteral("无法连接数据库 HospitalDB：\n%1\n\n连接字符串：\n%2")
                          .arg(m_db.lastError().text().trimmed(), m_connectionString);
        qWarning() << "DB open failed:" << m_lastError;
        if (errorMessage)
            *errorMessage = m_lastError;
        return false;
    }

    // 连接可用性探测，避免"驱动加载成功但库不可用"
    QSqlQuery probe(m_db);
    if (!probe.exec(QStringLiteral("SELECT 1")) || !probe.next()) {
        m_opened = false;
        m_lastError = QStringLiteral("数据库探测语句执行失败：\n%1")
                          .arg(probe.lastError().text().trimmed());
        if (errorMessage)
            *errorMessage = m_lastError;
        return false;
    }

    m_opened = true;
    if (errorMessage)
        errorMessage->clear();
    return true;
}

bool DatabaseManager::logEvent(const QString &userName, const QString &event, const QString &detail)
{
    if (!m_opened)
        return false;
    QSqlQuery query(m_db);
    query.prepare(QStringLiteral(
        "INSERT INTO dbo.OperationHistory (UserName, Event, Detail) VALUES (?, ?, ?)"));
    query.addBindValue(userName.isEmpty() ? QStringLiteral("system") : userName);
    query.addBindValue(event);
    query.addBindValue(detail);
    if (!query.exec()) {
        qWarning() << "logEvent failed:" << query.lastError().text();
        return false;
    }
    return true;
}

QString DatabaseManager::sha256(const QString &text)
{
    return QString::fromLatin1(
        QCryptographicHash::hash(text.toUtf8(), QCryptographicHash::Sha256).toHex());
}
