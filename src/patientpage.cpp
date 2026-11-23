#include "patientpage.h"
#include "dbmanager.h"
#include "delegates.h"
#include "patientdialog.h"
#include "statisticsdialog.h"

#include <QDate>
#include <QFile>
#include <QFileDialog>
#include <QFileInfo>
#include <QHBoxLayout>
#include <QHash>
#include <QHeaderView>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QSqlError>
#include <QSqlQuery>
#include <QSqlRecord>
#include <QSqlTableModel>
#include <QShortcut>
#include <QStackedLayout>
#include <QTableView>
#include <QTextStream>
#include <QVBoxLayout>

namespace {
enum Column {
    ColId = 0, ColIdCard, ColName, ColSex, ColDob, ColAge, ColHeight, ColWeight,
    ColPhone, ColAddress, ColDept, ColDoctor, ColDiagnosis, ColVisitDate, ColCreated
};
}

PatientPage::PatientPage(QWidget *parent)
    : QWidget(parent)
{
    buildUi();
    setupModel();
    refresh();
}

void PatientPage::buildUi()
{
    setObjectName(QStringLiteral("pageRoot"));

    auto *title = new QLabel(QStringLiteral("患者信息管理"), this);
    title->setObjectName(QStringLiteral("pageTitle"));
    auto *subtitle = new QLabel(QStringLiteral("支持按姓名、身份证号、手机号模糊查询，双击记录可直接编辑"), this);
    subtitle->setObjectName(QStringLiteral("pageSubtitle"));

    m_searchEdit = new QLineEdit(this);
    m_searchEdit->setPlaceholderText(QStringLiteral("请输入姓名 / 身份证号 / 手机号关键字"));
    m_searchEdit->setClearButtonEnabled(true);
    m_searchEdit->setMinimumWidth(260);

    auto *queryButton = new QPushButton(QStringLiteral("查 询"), this);
    queryButton->setObjectName(QStringLiteral("primaryButton"));
    queryButton->setCursor(Qt::PointingHandCursor);

    auto *resetButton = new QPushButton(QStringLiteral("重 置"), this);
    resetButton->setObjectName(QStringLiteral("ghostButton"));
    resetButton->setCursor(Qt::PointingHandCursor);

    auto *addButton = new QPushButton(QStringLiteral("＋ 新增患者"), this);
    addButton->setObjectName(QStringLiteral("primaryButton"));
    addButton->setCursor(Qt::PointingHandCursor);

    m_editButton = new QPushButton(QStringLiteral("修改"), this);
    m_editButton->setObjectName(QStringLiteral("ghostButton"));
    m_editButton->setCursor(Qt::PointingHandCursor);

    m_deleteButton = new QPushButton(QStringLiteral("删除"), this);
    m_deleteButton->setObjectName(QStringLiteral("dangerButton"));
    m_deleteButton->setCursor(Qt::PointingHandCursor);

    auto *refreshButton = new QPushButton(QStringLiteral("刷新"), this);
    refreshButton->setObjectName(QStringLiteral("ghostButton"));
    refreshButton->setCursor(Qt::PointingHandCursor);

    auto *statButton = new QPushButton(QStringLiteral("统计查询"), this);
    statButton->setObjectName(QStringLiteral("ghostButton"));
    statButton->setCursor(Qt::PointingHandCursor);

    auto *exportButton = new QPushButton(QStringLiteral("导出 CSV"), this);
    exportButton->setObjectName(QStringLiteral("ghostButton"));
    exportButton->setCursor(Qt::PointingHandCursor);

    auto *toolbar = new QHBoxLayout;
    toolbar->setSpacing(8);
    toolbar->addWidget(m_searchEdit);
    toolbar->addWidget(queryButton);
    toolbar->addWidget(resetButton);
    toolbar->addSpacing(16);
    toolbar->addWidget(addButton);
    toolbar->addWidget(m_editButton);
    toolbar->addWidget(m_deleteButton);
    toolbar->addWidget(refreshButton);
    toolbar->addStretch(1);
    toolbar->addWidget(statButton);
    toolbar->addWidget(exportButton);

    m_view = new QTableView(this);
    m_view->setObjectName(QStringLiteral("dataTable"));
    m_view->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_view->setSelectionMode(QAbstractItemView::SingleSelection);
    m_view->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_view->setAlternatingRowColors(true);
    m_view->setShowGrid(false);
    m_view->verticalHeader()->setVisible(false);
    m_view->verticalHeader()->setDefaultSectionSize(34);
    m_view->horizontalHeader()->setHighlightSections(false);
    m_view->horizontalHeader()->setStretchLastSection(true);
    m_view->setSortingEnabled(false);

    m_emptyLabel = new QLabel(QStringLiteral("未找到匹配的患者记录\n请调整查询关键字或点击「重置」"), this);
    m_emptyLabel->setObjectName(QStringLiteral("emptyLabel"));
    m_emptyLabel->setAlignment(Qt::AlignCenter);

    m_stack = new QStackedLayout;
    m_stack->addWidget(m_view);
    m_stack->addWidget(m_emptyLabel);

    auto *tableContainer = new QWidget(this);
    tableContainer->setLayout(m_stack);

    m_summaryLabel = new QLabel(this);
    m_summaryLabel->setObjectName(QStringLiteral("summaryLabel"));

    auto *hintLabel = new QLabel(QStringLiteral("提示：删除操作会二次确认；修改后表格与数据库同步刷新"), this);
    hintLabel->setObjectName(QStringLiteral("hintLabel"));

    auto *footer = new QHBoxLayout;
    footer->addWidget(m_summaryLabel);
    footer->addStretch(1);
    footer->addWidget(hintLabel);

    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(28, 24, 28, 18);
    layout->setSpacing(12);
    layout->addWidget(title);
    layout->addWidget(subtitle);
    layout->addSpacing(4);
    layout->addLayout(toolbar);
    layout->addWidget(tableContainer, 1);
    layout->addLayout(footer);

    connect(queryButton, &QPushButton::clicked, this, &PatientPage::onQuery);
    connect(m_searchEdit, &QLineEdit::returnPressed, this, &PatientPage::onQuery);
    connect(resetButton, &QPushButton::clicked, this, &PatientPage::onReset);
    connect(addButton, &QPushButton::clicked, this, &PatientPage::onAdd);
    connect(m_editButton, &QPushButton::clicked, this, &PatientPage::onEdit);
    connect(m_deleteButton, &QPushButton::clicked, this, &PatientPage::onDelete);
    connect(refreshButton, &QPushButton::clicked, this, &PatientPage::onRefresh);
    connect(statButton, &QPushButton::clicked, this, &PatientPage::onStatistics);
    connect(exportButton, &QPushButton::clicked, this, &PatientPage::onExportCsv);
    connect(m_view, &QTableView::doubleClicked, this, &PatientPage::onTableDoubleClicked);

    // 常用操作快捷方式，便于键盘操作
    new QShortcut(QKeySequence(QStringLiteral("Ctrl+F")), this, [this]() {
        m_searchEdit->setFocus();
        m_searchEdit->selectAll();
    });
    new QShortcut(QKeySequence(QStringLiteral("Ctrl+N")), this, [this]() { onAdd(); });
    new QShortcut(QKeySequence(QStringLiteral("Ctrl+E")), this, [this]() { onEdit(); });
    new QShortcut(QKeySequence(QStringLiteral("Ctrl+T")), this, [this]() { onStatistics(); });
    new QShortcut(QKeySequence(QStringLiteral("Ctrl+Shift+E")), this, [this]() { onExportCsv(); });
    new QShortcut(QKeySequence(QStringLiteral("F5")), this, [this]() { refresh(); });
    new QShortcut(QKeySequence(QStringLiteral("Ctrl+D")), this, [this]() { onDelete(); });
    auto *deleteShortcut = new QShortcut(QKeySequence::Delete, this);
    deleteShortcut->setContext(Qt::WidgetWithChildrenShortcut);
    connect(deleteShortcut, &QShortcut::activated, this, &PatientPage::onDelete);

    m_currentUser = QStringLiteral("admin");
}

void PatientPage::setupModel()
{
    m_model = new QSqlTableModel(this, DatabaseManager::instance().database());
    m_model->setTable(QStringLiteral("Patient"));
    m_model->setEditStrategy(QSqlTableModel::OnManualSubmit);
    m_model->setSort(ColId, Qt::AscendingOrder);

    const QStringList headers = {
        QStringLiteral("编号"), QStringLiteral("身份证号"), QStringLiteral("姓名"),
        QStringLiteral("性别"), QStringLiteral("出生日期"), QStringLiteral("年龄"),
        QStringLiteral("身高"), QStringLiteral("体重"), QStringLiteral("手机号码"),
        QStringLiteral("住址"), QStringLiteral("科室"), QStringLiteral("主治医生"),
        QStringLiteral("诊断"), QStringLiteral("就诊日期"), QStringLiteral("建档时间")
    };
    for (int i = 0; i < headers.size(); ++i)
        m_model->setHeaderData(i, Qt::Horizontal, headers.at(i));

    m_view->setModel(m_model);
    m_view->setColumnHidden(ColCreated, true);
    m_view->setColumnHidden(ColDob, true);
    m_view->setColumnHidden(ColAddress, true);
    m_view->setItemDelegateForColumn(ColSex, new SexDelegate(this));
    m_view->setItemDelegateForColumn(ColDob, new DateDelegate(this));
    m_view->setItemDelegateForColumn(ColVisitDate, new DateDelegate(this));

    m_deptDelegate = new RelationDelegate(ColDept, this);
    m_doctorDelegate = new RelationDelegate(ColDoctor, this);
    m_view->setItemDelegateForColumn(ColDept, m_deptDelegate);
    m_view->setItemDelegateForColumn(ColDoctor, m_doctorDelegate);

    QHeaderView *header = m_view->horizontalHeader();
    const QVector<int> widths = {44, 146, 74, 46, 0, 46, 60, 60, 104, 0, 68, 94, 118, 86, 0};
    for (int i = 0; i < widths.size(); ++i) {
        if (widths.at(i) > 0)
            m_view->setColumnWidth(i, widths.at(i));
    }
    header->setSectionResizeMode(QHeaderView::Interactive);

    connect(m_view->selectionModel(), &QItemSelectionModel::selectionChanged,
            this, &PatientPage::updateActionState);
}

void PatientPage::updateActionState()
{
    const bool hasSelection = m_view->currentIndex().isValid() && m_model->rowCount() > 0;
    m_editButton->setEnabled(hasSelection);
    m_deleteButton->setEnabled(hasSelection);
}

void PatientPage::reloadReferenceData()
{
    QVector<QPair<int, QString>> departments;
    QVector<QPair<int, QString>> doctors;

    QSqlQuery query(DatabaseManager::instance().database());
    if (query.exec(QStringLiteral("SELECT DeptID, DeptName FROM dbo.Department ORDER BY DeptID"))) {
        while (query.next())
            departments.append({query.value(0).toInt(), query.value(1).toString()});
    }
    if (query.exec(QStringLiteral("SELECT DoctorID, DoctorName FROM dbo.Doctor ORDER BY DoctorID"))) {
        while (query.next())
            doctors.append({query.value(0).toInt(), query.value(1).toString()});
    }

    m_deptDelegate->setItems(departments);
    m_doctorDelegate->setItems(doctors);
}

void PatientPage::setCurrentUser(const QString &userName)
{
    m_currentUser = userName;
}

QString PatientPage::currentKeyword() const
{
    return m_searchEdit ? m_searchEdit->text().trimmed() : QString();
}

void PatientPage::refresh()
{
    // 数据库不可用时由主窗口统一提示，这里避免逐个页面弹错误框
    if (!DatabaseManager::instance().isOpen()) {
        m_summaryLabel->setText(QStringLiteral("数据库未连接，暂无数据"));
        m_stack->setCurrentWidget(m_emptyLabel);
        return;
    }

    reloadReferenceData();

    const QString keyword = currentKeyword();
    if (keyword.isEmpty()) {
        m_model->setFilter(QString());
    } else {
        QString escaped = keyword;
        escaped.replace(QLatin1Char('\''), QLatin1String("''"));
        m_model->setFilter(QStringLiteral(
            "PatientName LIKE N'%%1%' OR IdCard LIKE '%%1%' OR MobilePhone LIKE '%%1%'")
            .arg(escaped));
    }

    const bool ok = m_model->select();
    if (!ok) {
        QMessageBox::critical(this, QStringLiteral("查询失败"),
                              QStringLiteral("读取患者数据失败：\n%1").arg(m_model->lastError().text()));
        return;
    }

    updateSummary();
}

void PatientPage::updateSummary()
{
    const int shown = m_model->rowCount();

    int total = 0;
    QSqlQuery countQuery(DatabaseManager::instance().database());
    if (countQuery.exec(QStringLiteral("SELECT COUNT(*) FROM dbo.Patient")) && countQuery.next())
        total = countQuery.value(0).toInt();

    const QString keyword = currentKeyword();
    if (shown == 0) {
        m_stack->setCurrentWidget(m_emptyLabel);
    } else {
        m_stack->setCurrentWidget(m_view);
    }

    m_summaryLabel->setText(
        QStringLiteral("当前显示 %1 条 / 全库 %2 条%3")
            .arg(shown)
            .arg(total)
            .arg(keyword.isEmpty() ? QString()
                                   : QStringLiteral("　·　查询条件：%1").arg(keyword)));

    if (shown > 0 && !m_view->currentIndex().isValid())
        m_view->selectRow(0);

    const bool hasSelection = m_view->currentIndex().isValid() && shown > 0;
    m_editButton->setEnabled(hasSelection);
    m_deleteButton->setEnabled(hasSelection);

    emit statusMessage(QStringLiteral("患者数据已刷新，共 %1 条记录").arg(shown));}

void PatientPage::onQuery()
{
    if (currentKeyword().isEmpty()) {
        QMessageBox::information(this, QStringLiteral("查询"), QStringLiteral("请输入查询关键字。"));
        m_searchEdit->setFocus();
        return;
    }
    refresh();
}

void PatientPage::onReset()
{
    m_searchEdit->clear();
    refresh();
    m_searchEdit->setFocus();
}

void PatientPage::onAdd()
{
    PatientDialog dialog(PatientDialog::AddMode, -1, this);
    if (dialog.exec() != QDialog::Accepted)
        return;

    DatabaseManager::instance().logEvent(m_currentUser, QStringLiteral("新增患者"),
                                         QStringLiteral("患者编号 %1").arg(dialog.savedId()));
    refresh();
    emit statusMessage(QStringLiteral("新增患者成功，编号 %1").arg(dialog.savedId()));
}

void PatientPage::onEdit()
{
    const int patientId = selectedPatientId();
    if (patientId <= 0) {
        QMessageBox::information(this, QStringLiteral("提示"), QStringLiteral("请先在表格中选择一条患者记录。"));
        return;
    }

    PatientDialog dialog(PatientDialog::EditMode, patientId, this);
    if (dialog.exec() != QDialog::Accepted)
        return;

    DatabaseManager::instance().logEvent(m_currentUser, QStringLiteral("修改患者"),
                                         QStringLiteral("患者编号 %1").arg(patientId));
    refresh();
    emit statusMessage(QStringLiteral("患者信息已更新并同步到数据库"));
}

void PatientPage::onDelete()
{
    const int patientId = selectedPatientId();
    if (patientId <= 0) {
        QMessageBox::information(this, QStringLiteral("提示"), QStringLiteral("请先在表格中选择一条患者记录。"));
        return;
    }

    const QModelIndex current = m_view->currentIndex();
    const QString name = m_model->record(current.row()).value(ColName).toString();
    const QString idCard = m_model->record(current.row()).value(ColIdCard).toString();

    QMessageBox confirm(this);
    confirm.setWindowTitle(QStringLiteral("确认删除"));
    confirm.setIcon(QMessageBox::Question);
    confirm.setText(QStringLiteral("确定要删除以下患者记录吗？此操作不可撤销。"));
    confirm.setInformativeText(QStringLiteral("姓名：%1\n身份证号：%2\n记录编号：%3")
                                   .arg(name, idCard).arg(patientId));
    QPushButton *yesButton = confirm.addButton(QStringLiteral("确认删除(&Y)"), QMessageBox::AcceptRole);
    QPushButton *noButton = confirm.addButton(QStringLiteral("取消(&N)"), QMessageBox::RejectRole);
    confirm.setDefaultButton(noButton);
    confirm.exec();
    if (confirm.clickedButton() != yesButton)
        return;

    QSqlQuery query(DatabaseManager::instance().database());
    query.prepare(QStringLiteral("DELETE FROM dbo.Patient WHERE PatientID = ?"));
    query.addBindValue(patientId);
    if (!query.exec()) {
        QMessageBox::critical(this, QStringLiteral("删除失败"),
                              QStringLiteral("数据库删除失败：\n%1").arg(query.lastError().text().trimmed()));
        return;
    }

    DatabaseManager::instance().logEvent(m_currentUser, QStringLiteral("删除患者"),
                                         QStringLiteral("%1（%2）").arg(name, idCard));
    refresh();
    emit statusMessage(QStringLiteral("已删除患者 %1，列表已刷新").arg(name));
}

void PatientPage::onRefresh()
{
    refresh();
}

void PatientPage::onStatistics()
{
    StatisticsDialog dialog(this);
    dialog.exec();
}

void PatientPage::onExportCsv()
{
    if (m_model->rowCount() == 0) {
        QMessageBox::information(this, QStringLiteral("导出"), QStringLiteral("当前没有可导出的记录。"));
        return;
    }

    const QString fileName = QFileDialog::getSaveFileName(
        this, QStringLiteral("导出患者列表"),
        QStringLiteral("患者列表_%1.csv").arg(QDate::currentDate().toString(QStringLiteral("yyyyMMdd"))),
        QStringLiteral("CSV 文件 (*.csv)"));
    if (fileName.isEmpty())
        return;

    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::critical(this, QStringLiteral("导出失败"), QStringLiteral("无法写入文件：%1").arg(fileName));
        return;
    }

    QTextStream stream(&file);
    stream.setEncoding(QStringConverter::Utf8);
    stream << QChar(0xFEFF); // BOM，保证 Excel 打开中文不乱码

    // 导出前取一份科室/医生名称映射，避免 CSV 中只剩编号
    QHash<int, QString> deptNames;
    QHash<int, QString> doctorNames;
    {
        QSqlQuery lookup(DatabaseManager::instance().database());
        if (lookup.exec(QStringLiteral("SELECT DeptID, DeptName FROM dbo.Department"))) {
            while (lookup.next())
                deptNames.insert(lookup.value(0).toInt(), lookup.value(1).toString());
        }
        if (lookup.exec(QStringLiteral("SELECT DoctorID, DoctorName FROM dbo.Doctor"))) {
            while (lookup.next())
                doctorNames.insert(lookup.value(0).toInt(), lookup.value(1).toString());
        }
    }

    QStringList headers;
    for (int col = 0; col < m_model->columnCount(); ++col) {
        if (col == ColCreated)
            continue;
        headers << m_model->headerData(col, Qt::Horizontal).toString();
    }
    stream << headers.join(QLatin1Char(',')) << '\n';

    for (int row = 0; row < m_model->rowCount(); ++row) {
        const QSqlRecord record = m_model->record(row);
        QStringList fields;
        for (int col = 0; col < m_model->columnCount(); ++col) {
            if (col == ColCreated)
                continue;
            QString text;
            if (col == ColSex) {
                text = record.value(col).toInt() == 1 ? QStringLiteral("男") : QStringLiteral("女");
            } else if (col == ColDept) {
                text = deptNames.value(record.value(col).toInt());
            } else if (col == ColDoctor) {
                text = doctorNames.value(record.value(col).toInt());
            } else if (col == ColDob || col == ColVisitDate) {
                const QDate date = record.value(col).toDate();
                text = date.isValid() ? date.toString(QStringLiteral("yyyy-MM-dd"))
                                      : record.value(col).toString();
            } else {
                text = record.value(col).toString();
            }
            text.replace(QLatin1Char('"'), QLatin1String("\"\""));
            fields << QStringLiteral("\"%1\"").arg(text);
        }
        stream << fields.join(QLatin1Char(',')) << '\n';
    }
    file.close();

    DatabaseManager::instance().logEvent(m_currentUser, QStringLiteral("导出数据"),
                                         QFileInfo(fileName).fileName());
    QMessageBox::information(this, QStringLiteral("导出完成"),
                             QStringLiteral("已导出 %1 条记录到：\n%2")
                                 .arg(m_model->rowCount()).arg(fileName));
}

int PatientPage::selectedPatientId() const
{
    const QModelIndex current = m_view->currentIndex();
    if (!current.isValid())
        return -1;
    return m_model->record(current.row()).value(ColId).toInt();
}

void PatientPage::onTableDoubleClicked(const QModelIndex &index)
{
    if (index.isValid())
        onEdit();
}
