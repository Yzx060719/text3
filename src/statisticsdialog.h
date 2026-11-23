#ifndef STATISTICSDIALOG_H
#define STATISTICSDIALOG_H

#include <QDialog>

class QSqlQueryModel;
class QLabel;
class QTableView;

// 统计查询：使用 QSqlQuery 聚合查询生成科室分布与医生工作量报表
class StatisticsDialog : public QDialog
{
    Q_OBJECT
public:
    explicit StatisticsDialog(QWidget *parent = nullptr);

private:
    void loadSummary();
    void loadDepartmentStats();
    void loadDoctorStats();

    QLabel *m_totalLabel = nullptr;
    QLabel *m_deptLabel = nullptr;
    QLabel *m_doctorLabel = nullptr;
    QLabel *m_monthLabel = nullptr;
    QSqlQueryModel *m_deptModel = nullptr;
    QSqlQueryModel *m_doctorModel = nullptr;
    QTableView *m_deptView = nullptr;
    QTableView *m_doctorView = nullptr;
};

#endif // STATISTICSDIALOG_H
