#include "logpage.h"
#include "dbmanager.h"

#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QSqlError>
#include <QSqlQuery>
#include <QSqlQueryModel>
#include <QTableView>
#include <QVBoxLayout>

LogPage::LogPage(QWidget *parent)
    : QWidget(parent)
{
    buildUi();
    refresh();
}

void LogPage::buildUi()
{
    setObjectName(QStringLiteral("pageRoot"));

    auto *title = new QLabel(QStringLiteral("系统操作日志"), this);
    title->setObjectName(QStringLiteral("pageTitle"));
    auto *subtitle = new QLabel(QStringLiteral("记录登录、增删改与导出等关键操作，用于核对操作闭环"), this);
    subtitle->setObjectName(QStringLiteral("pageSubtitle"));

    m_searchEdit = new QLineEdit(this);
    m_searchEdit->setPlaceholderText(QStringLiteral("按用户 / 操作类型 / 明细关键字检索，留空显示全部"));
    m_searchEdit->setClearButtonEnabled(true);
    m_searchEdit->setMinimumWidth(320);

    auto *queryButton = new QPushButton(QStringLiteral("检 索"), this);
    queryButton->setObjectName(QStringLiteral("primaryButton"));
    queryButton->setCursor(Qt::PointingHandCursor);
    auto *resetButton = new QPushButton(QStringLiteral("重 置"), this);
    resetButton->setObjectName(QStringLiteral("ghostButton"));
    resetButton->setCursor(Qt::PointingHandCursor);

    auto *toolbar = new QHBoxLayout;
    toolbar->setSpacing(8);
    toolbar->addWidget(m_searchEdit);
    toolbar->addWidget(queryButton);
    toolbar->addWidget(resetButton);
    toolbar->addStretch(1);

    m_view = new QTableView(this);
    m_view->setObjectName(QStringLiteral("dataTable"));
    m_view->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_view->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_view->setAlternatingRowColors(true);
    m_view->setShowGrid(false);
    m_view->verticalHeader()->setVisible(false);
    m_view->verticalHeader()->setDefaultSectionSize(32);
    m_view->horizontalHeader()->setStretchLastSection(true);
    m_view->horizontalHeader()->setHighlightSections(false);

    m_model = new QSqlQueryModel(this);
    m_view->setModel(m_model);

    m_summaryLabel = new QLabel(this);
    m_summaryLabel->setObjectName(QStringLiteral("summaryLabel"));

    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(28, 24, 28, 18);
    layout->setSpacing(12);
    layout->addWidget(title);
    layout->addWidget(subtitle);
    layout->addSpacing(4);
    layout->addLayout(toolbar);
    layout->addWidget(m_view, 1);
    layout->addWidget(m_summaryLabel);

    connect(queryButton, &QPushButton::clicked, this, &LogPage::onQuery);
    connect(m_searchEdit, &QLineEdit::returnPressed, this, &LogPage::onQuery);
    connect(resetButton, &QPushButton::clicked, this, &LogPage::onReset);
}

void LogPage::loadLogs(const QString &keyword)
{
    if (!DatabaseManager::instance().isOpen()) {
        m_summaryLabel->setText(QStringLiteral("数据库未连接，无法读取日志"));
        return;
    }
    QSqlQuery query(DatabaseManager::instance().database());
    if (keyword.isEmpty()) {
        query.prepare(QStringLiteral(
            "SELECT TOP 200 LogTime AS 时间, UserName AS 用户, Event AS 操作类型, Detail AS 明细"
            " FROM dbo.OperationHistory ORDER BY HistoryID DESC"));
    } else {
        query.prepare(QStringLiteral(
            "SELECT TOP 200 LogTime AS 时间, UserName AS 用户, Event AS 操作类型, Detail AS 明细"
            " FROM dbo.OperationHistory"
            " WHERE UserName LIKE ? OR Event LIKE ? OR Detail LIKE ?"
            " ORDER BY HistoryID DESC"));
        const QString like = QStringLiteral("%") + keyword + QStringLiteral("%");
        query.addBindValue(like);
        query.addBindValue(like);
        query.addBindValue(like);
    }

    if (!query.exec()) {
        m_summaryLabel->setText(QStringLiteral("日志查询失败：%1").arg(query.lastError().text().trimmed()));
        return;
    }

    m_model->setQuery(query);
    m_model->setHeaderData(0, Qt::Horizontal, QStringLiteral("时间"));
    m_model->setHeaderData(1, Qt::Horizontal, QStringLiteral("用户"));
    m_model->setHeaderData(2, Qt::Horizontal, QStringLiteral("操作类型"));
    m_model->setHeaderData(3, Qt::Horizontal, QStringLiteral("明细"));
    m_view->setColumnWidth(0, 165);
    m_view->setColumnWidth(1, 110);
    m_view->setColumnWidth(2, 130);

    m_summaryLabel->setText(QStringLiteral("共 %1 条日志").arg(m_model->rowCount()));
    emit statusMessage(QStringLiteral("操作日志已刷新"));
}

void LogPage::refresh()
{
    loadLogs(m_searchEdit ? m_searchEdit->text().trimmed() : QString());
}

void LogPage::onQuery()
{
    loadLogs(m_searchEdit->text().trimmed());
}

void LogPage::onReset()
{
    m_searchEdit->clear();
    refresh();
}
