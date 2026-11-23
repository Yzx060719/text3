#ifndef PATIENTPAGE_H
#define PATIENTPAGE_H

#include <QWidget>

class QLabel;
class QLineEdit;
class QPushButton;
class QSqlTableModel;
class QStackedLayout;
class QTableView;
class RelationDelegate;

// 患者管理页：列表显示、模糊查询、新增、修改、删除、导出、统计
class PatientPage : public QWidget
{
    Q_OBJECT
public:
    explicit PatientPage(QWidget *parent = nullptr);

    void setCurrentUser(const QString &userName);
    void refresh();

signals:
    void statusMessage(const QString &text);

private slots:
    void onQuery();
    void onReset();
    void onAdd();
    void onEdit();
    void onDelete();
    void onRefresh();
    void onExportCsv();
    void onStatistics();
    void onTableDoubleClicked(const QModelIndex &index);
    void updateActionState();

private:
    void buildUi();
    void setupModel();
    void reloadReferenceData();
    void updateSummary();
    int selectedPatientId() const;
    QString currentKeyword() const;

    QSqlTableModel *m_model = nullptr;
    QTableView *m_view = nullptr;
    QStackedLayout *m_stack = nullptr;
    QLabel *m_emptyLabel = nullptr;
    QLineEdit *m_searchEdit = nullptr;
    QLabel *m_summaryLabel = nullptr;
    QPushButton *m_editButton = nullptr;
    QPushButton *m_deleteButton = nullptr;

    RelationDelegate *m_deptDelegate = nullptr;
    RelationDelegate *m_doctorDelegate = nullptr;

    QString m_currentUser;
};

#endif // PATIENTPAGE_H
