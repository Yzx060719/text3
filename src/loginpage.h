#ifndef LOGINPAGE_H
#define LOGINPAGE_H

#include <QWidget>

class QLineEdit;
class QPushButton;
class QLabel;

// 登录页：校验用户凭据，并将结果区分为"用户不存在/密码错误/数据库不可用"
class LoginPage : public QWidget
{
    Q_OBJECT
public:
    explicit LoginPage(QWidget *parent = nullptr);

    void setDatabaseAvailable(bool available, const QString &error);
    void reset();

signals:
    void loginSucceeded(const QString &userName, const QString &fullName);
    void quitRequested();

private slots:
    void attemptLogin();

private:
    void buildUi();

    QLineEdit *m_userEdit = nullptr;
    QLineEdit *m_pwdEdit = nullptr;
    QPushButton *m_loginButton = nullptr;
    QPushButton *m_quitButton = nullptr;
    QLabel *m_tipLabel = nullptr;
    QLabel *m_dbLabel = nullptr;

    bool m_dbAvailable = true;
    QString m_dbError;
};

#endif // LOGINPAGE_H
