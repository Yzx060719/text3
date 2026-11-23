#include "loginpage.h"
#include "dbmanager.h"

#include <QFormLayout>
#include <QFrame>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QSqlError>
#include <QSqlQuery>
#include <QStyle>
#include <QVBoxLayout>
#include <QHBoxLayout>

LoginPage::LoginPage(QWidget *parent)
    : QWidget(parent)
{
    buildUi();
}

void LoginPage::buildUi()
{
    setObjectName(QStringLiteral("pageRoot"));

    auto *outer = new QVBoxLayout(this);
    outer->setContentsMargins(0, 0, 0, 0);

    auto *card = new QFrame(this);
    card->setObjectName(QStringLiteral("loginCard"));
    card->setFixedWidth(430);

    auto *logo = new QLabel(QStringLiteral("✚"), card);
    logo->setObjectName(QStringLiteral("loginLogo"));
    logo->setAlignment(Qt::AlignCenter);

    auto *title = new QLabel(QStringLiteral("医院诊疗系统"), card);
    title->setObjectName(QStringLiteral("loginTitle"));
    title->setAlignment(Qt::AlignCenter);

    auto *subtitle = new QLabel(QStringLiteral("Hospital Diagnosis & Treatment System"), card);
    subtitle->setObjectName(QStringLiteral("loginSubtitle"));
    subtitle->setAlignment(Qt::AlignCenter);

    m_userEdit = new QLineEdit(card);
    m_userEdit->setPlaceholderText(QStringLiteral("请输入用户名"));
    m_userEdit->setClearButtonEnabled(true);

    m_pwdEdit = new QLineEdit(card);
    m_pwdEdit->setPlaceholderText(QStringLiteral("请输入密码"));
    m_pwdEdit->setEchoMode(QLineEdit::Password);

    auto *form = new QFormLayout;
    form->setLabelAlignment(Qt::AlignRight | Qt::AlignVCenter);
    form->setFormAlignment(Qt::AlignHCenter);
    form->setHorizontalSpacing(12);
    form->setVerticalSpacing(14);
    form->addRow(QStringLiteral("用户名"), m_userEdit);
    form->addRow(QStringLiteral("密　码"), m_pwdEdit);

    m_loginButton = new QPushButton(QStringLiteral("登 录"), card);
    m_loginButton->setObjectName(QStringLiteral("primaryButton"));
    m_loginButton->setDefault(true);
    m_loginButton->setCursor(Qt::PointingHandCursor);

    m_quitButton = new QPushButton(QStringLiteral("退 出"), card);
    m_quitButton->setObjectName(QStringLiteral("ghostButton"));
    m_quitButton->setCursor(Qt::PointingHandCursor);

    auto *buttons = new QHBoxLayout;
    buttons->setSpacing(12);
    buttons->addWidget(m_loginButton, 3);
    buttons->addWidget(m_quitButton, 2);

    m_tipLabel = new QLabel(card);
    m_tipLabel->setObjectName(QStringLiteral("tipLabel"));
    m_tipLabel->setWordWrap(true);
    m_tipLabel->setMinimumHeight(36);
    m_tipLabel->setAlignment(Qt::AlignLeft | Qt::AlignTop);

    m_dbLabel = new QLabel(card);
    m_dbLabel->setObjectName(QStringLiteral("hintLabel"));
    m_dbLabel->setAlignment(Qt::AlignCenter);

    auto *devLabel = new QLabel(QStringLiteral("开发者：杨振曦　学号：2024414290439　班级：24级软件4班"), card);
    devLabel->setObjectName(QStringLiteral("devLabel"));
    devLabel->setAlignment(Qt::AlignCenter);

    auto *cardLayout = new QVBoxLayout(card);
    cardLayout->setContentsMargins(38, 30, 38, 24);
    cardLayout->setSpacing(6);
    cardLayout->addWidget(logo, 0, Qt::AlignHCenter);
    cardLayout->addSpacing(6);
    cardLayout->addWidget(title);
    cardLayout->addWidget(subtitle);
    cardLayout->addSpacing(18);
    cardLayout->addLayout(form);
    cardLayout->addSpacing(14);
    cardLayout->addLayout(buttons);
    cardLayout->addWidget(m_tipLabel);
    cardLayout->addWidget(m_dbLabel);
    cardLayout->addSpacing(4);
    cardLayout->addWidget(devLabel);

    outer->addStretch(1);
    outer->addWidget(card, 0, Qt::AlignHCenter);
    outer->addStretch(1);

    setTabOrder(m_userEdit, m_pwdEdit);
    setTabOrder(m_pwdEdit, m_loginButton);
    setTabOrder(m_loginButton, m_quitButton);

    connect(m_loginButton, &QPushButton::clicked, this, &LoginPage::attemptLogin);
    connect(m_pwdEdit, &QLineEdit::returnPressed, this, &LoginPage::attemptLogin);
    connect(m_userEdit, &QLineEdit::returnPressed, this, [this]() { m_pwdEdit->setFocus(); });
    connect(m_quitButton, &QPushButton::clicked, this, &LoginPage::quitRequested);

    m_dbLabel->setText(QStringLiteral("数据库：已连接 HospitalDB"));
    m_userEdit->setFocus();
}

void LoginPage::setDatabaseAvailable(bool available, const QString &error)
{
    m_dbAvailable = available;
    m_dbError = error;
    if (available) {
        m_dbLabel->setText(QStringLiteral("● 数据库连接正常：HospitalDB"));
        m_dbLabel->setProperty("state", QStringLiteral("ok"));
        m_loginButton->setEnabled(true);
    } else {
        m_dbLabel->setText(QStringLiteral("● 数据库连接失败，请检查 SQL Server 服务或连接配置"));
        m_dbLabel->setProperty("state", QStringLiteral("error"));
        m_loginButton->setEnabled(false);
        m_tipLabel->setText(error);
        m_tipLabel->setProperty("state", QStringLiteral("error"));
    }
    style()->unpolish(m_dbLabel);
    style()->polish(m_dbLabel);
    style()->unpolish(m_tipLabel);
    style()->polish(m_tipLabel);
}

void LoginPage::reset()
{
    m_pwdEdit->clear();
    m_tipLabel->clear();
    m_tipLabel->setProperty("state", QString());
    style()->unpolish(m_tipLabel);
    style()->polish(m_tipLabel);
    m_userEdit->setFocus();
    m_userEdit->selectAll();
}

void LoginPage::attemptLogin()
{
    const QString userName = m_userEdit->text().trimmed();
    const QString password = m_pwdEdit->text();

    auto showTip = [this](const QString &text) {
        m_tipLabel->setText(text);
        m_tipLabel->setProperty("state", QStringLiteral("error"));
        style()->unpolish(m_tipLabel);
        style()->polish(m_tipLabel);
    };

    if (!m_dbAvailable) {
        showTip(QStringLiteral("数据库不可用，无法登录。"));
        return;
    }
    if (userName.isEmpty()) {
        showTip(QStringLiteral("请输入用户名。"));
        m_userEdit->setFocus();
        return;
    }
    if (password.isEmpty()) {
        showTip(QStringLiteral("请输入密码。"));
        m_pwdEdit->setFocus();
        return;
    }

    // 先判断账号是否存在，再判断口令是否正确，从而区分两类错误凭据
    QSqlQuery existsQuery(DatabaseManager::instance().database());
    existsQuery.prepare(QStringLiteral("SELECT FullName FROM dbo.Users WHERE UserName = ?"));
    existsQuery.addBindValue(userName);
    if (!existsQuery.exec()) {
        showTip(QStringLiteral("查询用户失败：%1").arg(existsQuery.lastError().text().trimmed()));
        return;
    }
    if (!existsQuery.next()) {
        showTip(QStringLiteral("用户 %1 不存在，请核对用户名。").arg(userName));
        DatabaseManager::instance().logEvent(userName, QStringLiteral("登录失败"), QStringLiteral("用户不存在"));
        return;
    }
    const QString fullName = existsQuery.value(0).toString();
    existsQuery.finish();

    QSqlQuery authQuery(DatabaseManager::instance().database());
    authQuery.prepare(QStringLiteral(
        "SELECT FullName, Role FROM dbo.Users WHERE UserName = ? AND Password = ?"));
    authQuery.addBindValue(userName);
    authQuery.addBindValue(DatabaseManager::sha256(password));
    if (!authQuery.exec()) {
        showTip(QStringLiteral("登录校验失败：%1").arg(authQuery.lastError().text().trimmed()));
        return;
    }
    if (!authQuery.next()) {
        showTip(QStringLiteral("密码错误，请重新输入。"));
        DatabaseManager::instance().logEvent(userName, QStringLiteral("登录失败"), QStringLiteral("密码错误"));
        m_pwdEdit->selectAll();
        m_pwdEdit->setFocus();
        return;
    }

    const QString name = authQuery.value(0).toString();
    const QString role = authQuery.value(1).toString();
    DatabaseManager::instance().logEvent(userName, QStringLiteral("登录成功"),
                                         QStringLiteral("%1（%2）").arg(name, role));

    m_tipLabel->setText(QStringLiteral("登录成功，正在进入系统…"));
    m_tipLabel->setProperty("state", QStringLiteral("ok"));
    style()->unpolish(m_tipLabel);
    style()->polish(m_tipLabel);

    emit loginSucceeded(userName, name);
}
