#include "statisticsdialog.h"
#include "dbmanager.h"

#include <QDate>
#include <QFrame>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QPushButton>
#include <QSqlError>
#include <QSqlQuery>
#include <QSqlQueryModel>
#include <QTabWidget>
#include <QTableView>
#include <QVBoxLayout>

namespace {

QFrame *makeStatCard(const QString &caption, QLabel **valueLabel, QWidget *parent)
{
    auto *card = new QFrame(parent);
    card->setObjectName(QStringLiteral("statCard"));

    auto *value = new QLabel(QStringLiteral("0"), card);
    value->setObjectName(QStringLiteral("statValue"));
    auto *label = new QLabel(caption, card);
    label->setObjectName(QStringLiteral("statCaption"));

    auto *layout = new QVBoxLayout(card);
    layout->setContentsMargins(18, 12, 18, 12);
    layout->setSpacing(2);
    layout->addWidget(value);
    layout->addWidget(label);

    *valueLabel = value;
    return card;
}

QTableView *makeTable(QWidget *parent)
{
    auto *view = new QTableView(parent);
    view->setObjectName(QStringLiteral("dataTable"));
    view->setEditTriggers(QAbstractItemView::NoEditTriggers);
    view->setSelectionBehavior(QAbstractItemView::SelectRows);
    view->setAlternatingRowColors(true);
    view->setShowGrid(false);
    view->verticalHeader()->setVisible(false);
    view->verticalHeader()->setDefaultSectionSize(32);
    view->horizontalHeader()->setStretchLastSection(false);
    view->horizontalHeader()->setHighlightSections(false);
    return view;
}

void applyColumnWidths(QTableView *view, const QVector<int> &widths)
{
    for (int i = 0; i < widths.size(); ++i)
        view->setColumnWidth(i, widths.at(i));
}

} // namespace

StatisticsDialog::StatisticsDialog(QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle(QStringLiteral("统计查询（QSqlQuery 聚合查询）"));
    setModal(true);
    resize(900, 580);

    auto *title = new QLabel(QStringLiteral("诊疗数据统计"), this);
    title->setObjectName(QStringLiteral("pageTitle"));
    auto *subtitle = new QLabel(QStringLiteral("由 QSqlQuery 执行聚合 SQL 生成，用于核对界面显示与数据库实际数据是否一致"), this);
    subtitle->setObjectName(QStringLiteral("pageSubtitle"));

    auto *cardRow = new QHBoxLayout;
    cardRow->setSpacing(12);
    cardRow->addWidget(makeStatCard(QStringLiteral("患者总数"), &m_totalLabel, this), 1);
    cardRow->addWidget(makeStatCard(QStringLiteral("科室数量"), &m_deptLabel, this), 1);
    cardRow->addWidget(makeStatCard(QStringLiteral("医生数量"), &m_doctorLabel, this), 1);
    cardRow->addWidget(makeStatCard(QStringLiteral("本月新增"), &m_monthLabel, this), 1);

    auto *deptView = makeTable(this);
    m_deptModel = new QSqlQueryModel(this);
    deptView->setModel(m_deptModel);
    m_deptView = deptView;

    auto *doctorView = makeTable(this);
    m_doctorModel = new QSqlQueryModel(this);
    doctorView->setModel(m_doctorModel);
    m_doctorView = doctorView;

    auto *tabs = new QTabWidget(this);
    tabs->addTab(deptView, QStringLiteral("科室患者分布"));
    tabs->addTab(doctorView, QStringLiteral("医生接诊量"));

    auto *closeButton = new QPushButton(QStringLiteral("关 闭"), this);
    closeButton->setObjectName(QStringLiteral("primaryButton"));
    closeButton->setCursor(Qt::PointingHandCursor);

    auto *footer = new QHBoxLayout;
    footer->addStretch(1);
    footer->addWidget(closeButton);

    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(24, 20, 24, 16);
    layout->setSpacing(10);
    layout->addWidget(title);
    layout->addWidget(subtitle);
    layout->addSpacing(4);
    layout->addLayout(cardRow);
    layout->addWidget(tabs, 1);
    layout->addLayout(footer);

    connect(closeButton, &QPushButton::clicked, this, &QDialog::accept);

    loadSummary();
    loadDepartmentStats();
    loadDoctorStats();
}

void StatisticsDialog::loadSummary()
{
    QSqlQuery query(DatabaseManager::instance().database());

    if (query.exec(QStringLiteral("SELECT COUNT(*) FROM dbo.Patient")) && query.next())
        m_totalLabel->setText(query.value(0).toString());
    if (query.exec(QStringLiteral("SELECT COUNT(*) FROM dbo.Department")) && query.next())
        m_deptLabel->setText(query.value(0).toString());
    if (query.exec(QStringLiteral("SELECT COUNT(*) FROM dbo.Doctor")) && query.next())
        m_doctorLabel->setText(query.value(0).toString());

    query.prepare(QStringLiteral(
        "SELECT COUNT(*) FROM dbo.Patient WHERE CreatedAt >= ? AND CreatedAt < ?"));
    const QDate firstOfMonth(QDate::currentDate().year(), QDate::currentDate().month(), 1);
    query.addBindValue(firstOfMonth);
    query.addBindValue(firstOfMonth.addMonths(1));
    if (query.exec() && query.next())
        m_monthLabel->setText(query.value(0).toString());
}

void StatisticsDialog::loadDepartmentStats()
{
    m_deptModel->setQuery(QStringLiteral(
        "SELECT d.DeptName AS 科室, COUNT(p.PatientID) AS 患者数,"
        " ISNULL(CAST(AVG(p.Age * 1.0) AS DECIMAL(5,1)), 0) AS 平均年龄,"
        " SUM(CASE WHEN p.Sex = 1 THEN 1 ELSE 0 END) AS 男,"
        " SUM(CASE WHEN p.Sex = 0 THEN 1 ELSE 0 END) AS 女"
        " FROM dbo.Department d LEFT JOIN dbo.Patient p ON p.DeptID = d.DeptID"
        " GROUP BY d.DeptName ORDER BY 患者数 DESC"), DatabaseManager::instance().database());

    m_deptModel->setHeaderData(0, Qt::Horizontal, QStringLiteral("科室"));
    m_deptModel->setHeaderData(1, Qt::Horizontal, QStringLiteral("患者数"));
    m_deptModel->setHeaderData(2, Qt::Horizontal, QStringLiteral("平均年龄"));
    m_deptModel->setHeaderData(3, Qt::Horizontal, QStringLiteral("男"));
    m_deptModel->setHeaderData(4, Qt::Horizontal, QStringLiteral("女"));

    applyColumnWidths(m_deptView, {140, 110, 120, 90, 90});
    for (int i = 0; i < m_deptModel->columnCount(); ++i)
        m_deptView->horizontalHeader()->setSectionResizeMode(i, QHeaderView::Stretch);
}

void StatisticsDialog::loadDoctorStats()
{
    m_doctorModel->setQuery(QStringLiteral(
        "SELECT doc.DoctorName AS 医生, d.DeptName AS 科室, doc.Title AS 职称,"
        " COUNT(p.PatientID) AS 接诊人数"
        " FROM dbo.Doctor doc"
        " JOIN dbo.Department d ON d.DeptID = doc.DeptID"
        " LEFT JOIN dbo.Patient p ON p.DoctorID = doc.DoctorID"
        " GROUP BY doc.DoctorName, d.DeptName, doc.Title"
        " ORDER BY 接诊人数 DESC"), DatabaseManager::instance().database());

    m_doctorModel->setHeaderData(0, Qt::Horizontal, QStringLiteral("医生"));
    m_doctorModel->setHeaderData(1, Qt::Horizontal, QStringLiteral("科室"));
    m_doctorModel->setHeaderData(2, Qt::Horizontal, QStringLiteral("职称"));
    m_doctorModel->setHeaderData(3, Qt::Horizontal, QStringLiteral("接诊人数"));

    applyColumnWidths(m_doctorView, {120, 140, 130, 110});
    for (int i = 0; i < m_doctorModel->columnCount(); ++i)
        m_doctorView->horizontalHeader()->setSectionResizeMode(i, QHeaderView::Stretch);
}
