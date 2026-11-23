#include "mainwindow.h"
#include "dbmanager.h"
#include "homepage.h"
#include "logpage.h"
#include "loginpage.h"
#include "managepage.h"
#include "patientpage.h"

#include <QAction>
#include <QGuiApplication>
#include <QIcon>
#include <QLabel>
#include <QRect>
#include <QScreen>
#include <QSizePolicy>
#include <QStackedWidget>
#include <QStatusBar>
#include <QStyle>
#include <QToolBar>
#include <QToolButton>
#include <QVBoxLayout>

namespace {
enum PageIndex {
    PageLogin = 0,
    PageHome = 1,
    PagePatient = 2,
    PageDepartment = 3,
    PageDoctor = 4,
    PageLog = 5
};
}

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setWindowTitle(QStringLiteral("医院诊疗系统 · 患者信息管理（Qt SQL + Model/View）"));
    resize(1100, 650);
    setMinimumSize(980, 600);

    // 按屏幕可用区域居中，并保证窗口完整可见（高分屏缩放下尤其重要）
    if (QScreen *screen = QGuiApplication::primaryScreen()) {
        const QRect available = screen->availableGeometry();
        const QSize target(qMin(1100, available.width() - 80), qMin(650, available.height() - 80));
        resize(target);
        move(available.center() - QPoint(target.width() / 2, target.height() / 2));
    }

    buildUi();
    buildToolBar();
    buildStatusBar();
    setNavigationEnabled(false);
    m_toolBar->setVisible(false);
    setWindowIcon(QIcon());
}

void MainWindow::buildUi()
{
    m_stack = new QStackedWidget(this);

    m_loginPage = new LoginPage(this);
    m_homePage = new HomePage(this);
    m_patientPage = new PatientPage(this);
    m_departmentPage = new ManagePage(ManagePage::DepartmentKind, this);
    m_doctorPage = new ManagePage(ManagePage::DoctorKind, this);
    m_logPage = new LogPage(this);

    m_stack->insertWidget(PageLogin, m_loginPage);
    m_stack->insertWidget(PageHome, m_homePage);
    m_stack->insertWidget(PagePatient, m_patientPage);
    m_stack->insertWidget(PageDepartment, m_departmentPage);
    m_stack->insertWidget(PageDoctor, m_doctorPage);
    m_stack->insertWidget(PageLog, m_logPage);
    m_stack->setCurrentIndex(PageLogin);

    auto *central = new QWidget(this);
    auto *layout = new QVBoxLayout(central);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);
    m_toolBar = new QToolBar(central);
    layout->addWidget(m_toolBar);
    layout->addWidget(m_stack, 1);
    setCentralWidget(central);

    connect(m_loginPage, &LoginPage::loginSucceeded, this, &MainWindow::onLoginSucceeded);
    connect(m_loginPage, &LoginPage::quitRequested, this, &QWidget::close);

    connect(m_homePage, &HomePage::requestPatientPage, this, [this]() { showPage(PagePatient); });
    connect(m_homePage, &HomePage::requestDepartmentPage, this, [this]() { showPage(PageDepartment); });
    connect(m_homePage, &HomePage::requestDoctorPage, this, [this]() { showPage(PageDoctor); });
    connect(m_homePage, &HomePage::requestLogPage, this, [this]() { showPage(PageLog); });

    connect(m_patientPage, &PatientPage::statusMessage, this, &MainWindow::updateStatus);
    connect(m_departmentPage, &ManagePage::statusMessage, this, &MainWindow::updateStatus);
    connect(m_doctorPage, &ManagePage::statusMessage, this, &MainWindow::updateStatus);
    connect(m_logPage, &LogPage::statusMessage, this, &MainWindow::updateStatus);
}

void MainWindow::buildToolBar()
{
    m_toolBar->setObjectName(QStringLiteral("mainToolBar"));
    m_toolBar->setMovable(false);
    m_toolBar->setFloatable(false);
    m_toolBar->setToolButtonStyle(Qt::ToolButtonTextOnly);
    m_toolBar->setIconSize(QSize(16, 16));

    auto *brand = new QLabel(QStringLiteral("  ✚ 医院诊疗系统 "), m_toolBar);
    brand->setObjectName(QStringLiteral("toolBrand"));

    m_homeAction = m_toolBar->addAction(QStringLiteral("首页"));
    m_patientAction = m_toolBar->addAction(QStringLiteral("患者管理"));
    m_departmentAction = m_toolBar->addAction(QStringLiteral("科室管理"));
    m_doctorAction = m_toolBar->addAction(QStringLiteral("医生管理"));
    m_logAction = m_toolBar->addAction(QStringLiteral("操作日志"));
    m_toolBar->insertWidget(m_homeAction, brand);

    for (auto *action : {m_homeAction, m_patientAction, m_departmentAction,
                         m_doctorAction, m_logAction})
        action->setCheckable(true);

    auto *spacer = new QWidget(m_toolBar);
    spacer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    m_toolBar->addWidget(spacer);

    m_userLabel = new QLabel(m_toolBar);
    m_userLabel->setObjectName(QStringLiteral("toolUser"));
    m_toolBar->addWidget(m_userLabel);
    m_logoutAction = m_toolBar->addAction(QStringLiteral("退出登录"));

    connect(m_homeAction, &QAction::triggered, this, [this]() { showPage(PageHome); });
    connect(m_patientAction, &QAction::triggered, this, [this]() { showPage(PagePatient); });
    connect(m_departmentAction, &QAction::triggered, this, [this]() { showPage(PageDepartment); });
    connect(m_doctorAction, &QAction::triggered, this, [this]() { showPage(PageDoctor); });
    connect(m_logAction, &QAction::triggered, this, [this]() { showPage(PageLog); });
    connect(m_logoutAction, &QAction::triggered, this, &MainWindow::onLogout);

    // 页面切换快捷键：Ctrl+1 首页，Ctrl+2 患者，Ctrl+3 科室，Ctrl+4 医生，Ctrl+5 日志
    m_homeAction->setShortcut(QKeySequence(QStringLiteral("Ctrl+1")));
    m_patientAction->setShortcut(QKeySequence(QStringLiteral("Ctrl+2")));
    m_departmentAction->setShortcut(QKeySequence(QStringLiteral("Ctrl+3")));
    m_doctorAction->setShortcut(QKeySequence(QStringLiteral("Ctrl+4")));
    m_logAction->setShortcut(QKeySequence(QStringLiteral("Ctrl+5")));
}

void MainWindow::buildStatusBar()
{
    m_dbLabel = new QLabel(this);
    m_dbLabel->setObjectName(QStringLiteral("statusDb"));
    m_dbLabel->setText(QStringLiteral("● 数据库未连接"));

    m_devLabel = new QLabel(QStringLiteral("开发者：杨振曦 · 2024414290439 · 24级软件4班"), this);
    m_devLabel->setObjectName(QStringLiteral("statusDev"));

    statusBar()->addPermanentWidget(m_dbLabel);
    statusBar()->addPermanentWidget(m_devLabel);
    statusBar()->showMessage(QStringLiteral("就绪"));
}

void MainWindow::setDatabaseStatus(bool available, const QString &error)
{
    m_loginPage->setDatabaseAvailable(available, error);
    if (available) {
        m_dbLabel->setText(QStringLiteral("● 已连接：SQL Server / HospitalDB"));
        m_dbLabel->setProperty("state", QStringLiteral("ok"));
    } else {
        m_dbLabel->setText(QStringLiteral("● 数据库连接失败"));
        m_dbLabel->setProperty("state", QStringLiteral("error"));
    }
    style()->unpolish(m_dbLabel);
    style()->polish(m_dbLabel);
}

void MainWindow::setNavigationEnabled(bool enabled)
{
    m_userLabel->setVisible(enabled);
    for (auto *action : {m_homeAction, m_patientAction, m_departmentAction,
                         m_doctorAction, m_logAction, m_logoutAction}) {
        action->setEnabled(enabled);
        if (QWidget *widget = m_toolBar->widgetForAction(action))
            widget->setVisible(enabled);
    }
}

void MainWindow::onLoginSucceeded(const QString &userName, const QString &fullName)
{
    m_currentUser = userName;
    m_patientPage->setCurrentUser(userName);
    m_homePage->setUserInfo(userName, fullName);
    m_userLabel->setText(QStringLiteral("当前用户：%1　").arg(fullName.isEmpty() ? userName : fullName));

    setNavigationEnabled(true);
    m_toolBar->setVisible(true);
    showPage(PageHome);
    statusBar()->showMessage(QStringLiteral("登录成功：%1，欢迎使用医院诊疗系统").arg(userName), 6000);
}

void MainWindow::onLogout()
{
    DatabaseManager::instance().logEvent(m_currentUser, QStringLiteral("退出登录"),
                                         QStringLiteral("用户主动退出"));
    m_currentUser.clear();
    m_loginPage->reset();
    setNavigationEnabled(false);
    m_toolBar->setVisible(false);
    m_stack->setCurrentIndex(PageLogin);
    statusBar()->showMessage(QStringLiteral("已退出登录"));
}

void MainWindow::showPage(int index)
{
    if (index < 0 || index >= m_stack->count())
        return;

    switch (index) {
    case PagePatient:
        m_patientPage->refresh();
        break;
    case PageDepartment:
        m_departmentPage->refresh();
        break;
    case PageDoctor:
        m_doctorPage->refresh();
        break;
    case PageLog:
        m_logPage->refresh();
        break;
    default:
        break;
    }
    m_stack->setCurrentIndex(index);

    m_homeAction->setChecked(index == PageHome);
    m_patientAction->setChecked(index == PagePatient);
    m_departmentAction->setChecked(index == PageDepartment);
    m_doctorAction->setChecked(index == PageDoctor);
    m_logAction->setChecked(index == PageLog);
}

void MainWindow::updateStatus(const QString &text)
{
    statusBar()->showMessage(text, 5000);
}
