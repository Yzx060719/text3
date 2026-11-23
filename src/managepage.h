#ifndef MANAGEPAGE_H
#define MANAGEPAGE_H

#include <QWidget>

class QLabel;
class QSqlTableModel;
class QTableView;

// 科室/医生管理页：同一套 View/Model 代码复用于两张表，支持单元格内编辑与批量提交
class ManagePage : public QWidget
{
    Q_OBJECT
public:
    enum Kind { DepartmentKind, DoctorKind };

    explicit ManagePage(Kind kind, QWidget *parent = nullptr);

    void refresh();

signals:
    void statusMessage(const QString &text);

private slots:
    void onAddRow();
    void onDeleteRow();
    void onSave();
    void onRevert();
    void onRefresh();

private:
    void buildUi();
    void setupModel();

    Kind m_kind;
    QSqlTableModel *m_model = nullptr;
    QTableView *m_view = nullptr;
    QLabel *m_summaryLabel = nullptr;
    QLabel *m_stateLabel = nullptr;
};

#endif // MANAGEPAGE_H
