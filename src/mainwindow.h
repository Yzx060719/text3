#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

class QAction;
class QLabel;
class QStackedWidget;
class QToolBar;
class HomePage;
class LoginPage;
class LogPage;
class ManagePage;
class PatientPage;

// 主窗口：用 QStackedWidget 组织登录页与各功能页，工具栏由代码动态构建
class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = nullptr);

    void setDatabaseStatus(bool available, const QString &error);

private slots:
    void onLoginSucceeded(const QString &userName, const QString &fullName);
    void onLogout();
    void showPage(int index);
    void updateStatus(const QString &text);

private:
    void buildUi();
    void buildToolBar();
    void buildStatusBar();
    void setNavigationEnabled(bool enabled);

    QStackedWidget *m_stack = nullptr;
    QToolBar *m_toolBar = nullptr;

    LoginPage *m_loginPage = nullptr;
    HomePage *m_homePage = nullptr;
    PatientPage *m_patientPage = nullptr;
    ManagePage *m_departmentPage = nullptr;
    ManagePage *m_doctorPage = nullptr;
    LogPage *m_logPage = nullptr;

    QAction *m_homeAction = nullptr;
    QAction *m_patientAction = nullptr;
    QAction *m_departmentAction = nullptr;
    QAction *m_doctorAction = nullptr;
    QAction *m_logAction = nullptr;
    QAction *m_logoutAction = nullptr;

    QLabel *m_userLabel = nullptr;
    QLabel *m_dbLabel = nullptr;
    QLabel *m_devLabel = nullptr;

    QString m_currentUser;
};

#endif // MAINWINDOW_H
