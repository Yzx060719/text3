#include "managepage.h"
#include "dbmanager.h"
#include "delegates.h"

#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QMessageBox>
#include <QPushButton>
#include <QSqlError>
#include <QSqlQuery>
#include <QSqlTableModel>
#include <QTableView>
#include <QVBoxLayout>

ManagePage::ManagePage(Kind kind, QWidget *parent)
    : QWidget(parent)
    , m_kind(kind)
{
    buildUi();
    setupModel();
    refresh();
}

void ManagePage::buildUi()
{
    setObjectName(QStringLiteral("pageRoot"));

    const bool isDepartment = (m_kind == DepartmentKind);

    auto *title = new QLabel(isDepartment ? QStringLiteral("科室信息管理") : QStringLiteral("医生信息管理"), this);
    title->setObjectName(QStringLiteral("pageTitle"));
    auto *subtitle = new QLabel(
        isDepartment ? QStringLiteral("双击单元格可直接修改科室信息，修改后点击「保存」提交到数据库")
                     : QStringLiteral("双击单元格可直接修改医生信息，科室列使用自定义代理下拉选择"),
        this);
    subtitle->setObjectName(QStringLiteral("pageSubtitle"));

    auto *addButton = new QPushButton(QStringLiteral("＋ 新增行"), this);
    addButton->setObjectName(QStringLiteral("primaryButton"));
    addButton->setCursor(Qt::PointingHandCursor);
    auto *deleteButton = new QPushButton(QStringLiteral("删除行"), this);
    deleteButton->setObjectName(QStringLiteral("dangerButton"));
    deleteButton->setCursor(Qt::PointingHandCursor);
    auto *saveButton = new QPushButton(QStringLiteral("保 存"), this);
    saveButton->setObjectName(QStringLiteral("primaryButton"));
    saveButton->setCursor(Qt::PointingHandCursor);
    auto *revertButton = new QPushButton(QStringLiteral("撤 销"), this);
    revertButton->setObjectName(QStringLiteral("ghostButton"));
    revertButton->setCursor(Qt::PointingHandCursor);
    auto *refreshButton = new QPushButton(QStringLiteral("刷 新"), this);
    refreshButton->setObjectName(QStringLiteral("ghostButton"));
    refreshButton->setCursor(Qt::PointingHandCursor);

    auto *toolbar = new QHBoxLayout;
    toolbar->setSpacing(8);
    toolbar->addWidget(addButton);
    toolbar->addWidget(deleteButton);
    toolbar->addSpacing(12);
    toolbar->addWidget(saveButton);
    toolbar->addWidget(revertButton);
    toolbar->addStretch(1);
    toolbar->addWidget(refreshButton);

    m_view = new QTableView(this);
    m_view->setObjectName(QStringLiteral("dataTable"));
    m_view->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_view->setSelectionMode(QAbstractItemView::SingleSelection);
    m_view->setAlternatingRowColors(true);
    m_view->setShowGrid(false);
    m_view->verticalHeader()->setVisible(false);
    m_view->verticalHeader()->setDefaultSectionSize(34);
    m_view->horizontalHeader()->setHighlightSections(false);
    m_view->horizontalHeader()->setStretchLastSection(true);

    m_summaryLabel = new QLabel(this);
    m_summaryLabel->setObjectName(QStringLiteral("summaryLabel"));
    m_stateLabel = new QLabel(QStringLiteral("提示：编辑后需点击「保存」才会写入数据库"), this);
    m_stateLabel->setObjectName(QStringLiteral("hintLabel"));

    auto *footer = new QHBoxLayout;
    footer->addWidget(m_summaryLabel);
    footer->addStretch(1);
    footer->addWidget(m_stateLabel);

    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(28, 24, 28, 18);
    layout->setSpacing(12);
    layout->addWidget(title);
    layout->addWidget(subtitle);
    layout->addSpacing(4);
    layout->addLayout(toolbar);
    layout->addWidget(m_view, 1);
    layout->addLayout(footer);

    connect(addButton, &QPushButton::clicked, this, &ManagePage::onAddRow);
    connect(deleteButton, &QPushButton::clicked, this, &ManagePage::onDeleteRow);
    connect(saveButton, &QPushButton::clicked, this, &ManagePage::onSave);
    connect(revertButton, &QPushButton::clicked, this, &ManagePage::onRevert);
    connect(refreshButton, &QPushButton::clicked, this, &ManagePage::onRefresh);
}

void ManagePage::setupModel()
{
    m_model = new QSqlTableModel(this, DatabaseManager::instance().database());
    m_model->setEditStrategy(QSqlTableModel::OnManualSubmit);
    m_model->setSort(0, Qt::AscendingOrder);
    m_view->horizontalHeader()->setStretchLastSection(false);

    if (m_kind == DepartmentKind) {
        m_model->setTable(QStringLiteral("Department"));
        m_model->setHeaderData(0, Qt::Horizontal, QStringLiteral("编号"));
        m_model->setHeaderData(1, Qt::Horizontal, QStringLiteral("科室名称"));
        m_model->setHeaderData(2, Qt::Horizontal, QStringLiteral("所在位置"));
        m_model->setHeaderData(3, Qt::Horizontal, QStringLiteral("联系电话"));
        m_view->setModel(m_model);
        m_view->setColumnWidth(0, 70);
        m_view->setColumnWidth(1, 240);
        m_view->setColumnWidth(2, 220);
        m_view->setColumnWidth(3, 200);
        m_view->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
    } else {
        m_model->setTable(QStringLiteral("Doctor"));
        m_model->setHeaderData(0, Qt::Horizontal, QStringLiteral("编号"));
        m_model->setHeaderData(1, Qt::Horizontal, QStringLiteral("工号"));
        m_model->setHeaderData(2, Qt::Horizontal, QStringLiteral("姓名"));
        m_model->setHeaderData(3, Qt::Horizontal, QStringLiteral("所属科室"));
        m_model->setHeaderData(4, Qt::Horizontal, QStringLiteral("职称"));
        m_model->setHeaderData(5, Qt::Horizontal, QStringLiteral("联系电话"));
        m_model->setHeaderData(6, Qt::Horizontal, QStringLiteral("擅长方向"));
        m_view->setModel(m_model);

        // 外键列用自定义代理：显示科室名称，编辑时为下拉框
        auto *deptDelegate = new RelationDelegate(3, this);
        QVector<QPair<int, QString>> departments;
        QSqlQuery query(DatabaseManager::instance().database());
        if (query.exec(QStringLiteral("SELECT DeptID, DeptName FROM dbo.Department ORDER BY DeptID"))) {
            while (query.next())
                departments.append({query.value(0).toInt(), query.value(1).toString()});
        }
        deptDelegate->setItems(departments);
        m_view->setItemDelegateForColumn(3, deptDelegate);

        m_view->setColumnWidth(0, 60);
        m_view->setColumnWidth(1, 90);
        m_view->setColumnWidth(2, 110);
        m_view->setColumnWidth(3, 120);
        m_view->setColumnWidth(4, 130);
        m_view->setColumnWidth(5, 140);
        m_view->setColumnWidth(6, 220);
        m_view->horizontalHeader()->setSectionResizeMode(6, QHeaderView::Stretch);
    }
}

void ManagePage::refresh()
{
    if (!DatabaseManager::instance().isOpen()) {
        m_summaryLabel->setText(QStringLiteral("数据库未连接"));
        return;
    }
    m_model->setSort(0, Qt::AscendingOrder);
    if (!m_model->select()) {
        QMessageBox::critical(this, QStringLiteral("查询失败"),
                              QStringLiteral("读取数据失败：\n%1").arg(m_model->lastError().text()));
        return;
    }
    m_summaryLabel->setText(QStringLiteral("共 %1 条记录").arg(m_model->rowCount()));
    emit statusMessage(QStringLiteral("记录已刷新"));
}

void ManagePage::onAddRow()
{
    const int row = m_model->rowCount();
    m_model->insertRow(row);
    m_view->selectRow(row);
    m_view->edit(m_model->index(row, 1));
    m_stateLabel->setText(QStringLiteral("已插入空白行，请填写内容后点击「保存」"));
}

void ManagePage::onDeleteRow()
{
    const QModelIndex current = m_view->currentIndex();
    if (!current.isValid()) {
        QMessageBox::information(this, QStringLiteral("提示"), QStringLiteral("请先选择要删除的行。"));
        return;
    }

    const auto answer = QMessageBox::question(this, QStringLiteral("确认删除"),
                                              QStringLiteral("确定删除选中的这一行吗？"),
                                              QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
    if (answer != QMessageBox::Yes)
        return;

    m_model->removeRow(current.row());
    m_stateLabel->setText(QStringLiteral("已从模型移除该行，点击「保存」后同步到数据库"));
}

void ManagePage::onSave()
{
    if (!m_model->submitAll()) {
        const QString error = m_model->lastError().text().trimmed();
        QMessageBox::critical(this, QStringLiteral("保存失败"),
                              QStringLiteral("数据库拒绝本次修改：\n%1\n\n"
                                             "常见原因：工号/科室名重复、外键被引用、字段为空。")
                                  .arg(error));
        m_model->revertAll();
        m_stateLabel->setText(QStringLiteral("保存失败，已回滚到数据库中的原始数据"));
        return;
    }
    m_stateLabel->setText(QStringLiteral("修改已提交并写入数据库"));
    DatabaseManager::instance().logEvent(QStringLiteral("admin"),
                                         m_kind == DepartmentKind ? QStringLiteral("修改科室")
                                                                  : QStringLiteral("修改医生"),
                                         QStringLiteral("Model/View 批量提交"));
    refresh();
}

void ManagePage::onRevert()
{
    m_model->revertAll();
    m_stateLabel->setText(QStringLiteral("已撤销未提交的修改"));
    refresh();
}

void ManagePage::onRefresh()
{
    refresh();
}
