#ifndef LOGPAGE_H
#define LOGPAGE_H

#include <QWidget>

class QLabel;
class QLineEdit;
class QSqlQueryModel;
class QTableView;

// 操作日志页（选做功能）：查看登录、增删改、导出等操作记录
class LogPage : public QWidget
{
    Q_OBJECT
public:
    explicit LogPage(QWidget *parent = nullptr);

    void refresh();

signals:
    void statusMessage(const QString &text);

private slots:
    void onQuery();
    void onReset();

private:
    void buildUi();
    void loadLogs(const QString &keyword);

    QSqlQueryModel *m_model = nullptr;
    QTableView *m_view = nullptr;
    QLineEdit *m_searchEdit = nullptr;
    QLabel *m_summaryLabel = nullptr;
};

#endif // LOGPAGE_H
