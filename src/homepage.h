#ifndef HOMEPAGE_H
#define HOMEPAGE_H

#include <QFrame>
#include <QWidget>

class QLabel;
class QMouseEvent;

// 首页功能卡片（点击后进入对应模块）
class NavCard : public QFrame
{
    Q_OBJECT
public:
    NavCard(const QString &icon, const QString &title, const QString &caption,
            QWidget *parent = nullptr);

signals:
    void clicked();

protected:
    void mouseReleaseEvent(QMouseEvent *event) override;

private:
    QLabel *m_iconLabel = nullptr;
};

// 欢迎页：以卡片方式进入各功能模块（QStackedWidget 的切换入口）
class HomePage : public QWidget
{
    Q_OBJECT
public:
    explicit HomePage(QWidget *parent = nullptr);

    void setUserInfo(const QString &userName, const QString &fullName);

signals:
    void requestPatientPage();
    void requestDepartmentPage();
    void requestDoctorPage();
    void requestLogPage();

private:
    QLabel *m_welcomeLabel = nullptr;
    QLabel *m_subLabel = nullptr;
};

#endif // HOMEPAGE_H
