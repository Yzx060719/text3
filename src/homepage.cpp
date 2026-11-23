#include "homepage.h"

#include <QDate>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLocale>
#include <QMouseEvent>
#include <QPushButton>
#include <QVBoxLayout>

NavCard::NavCard(const QString &icon, const QString &title, const QString &caption,
                 QWidget *parent)
    : QFrame(parent)
{
    setObjectName(QStringLiteral("navCard"));
    setFixedSize(224, 168);
    setCursor(Qt::PointingHandCursor);

    m_iconLabel = new QLabel(icon, this);
    m_iconLabel->setObjectName(QStringLiteral("navIcon"));
    m_iconLabel->setAlignment(Qt::AlignCenter);
    m_iconLabel->setFixedSize(56, 56);

    auto *titleLabel = new QLabel(title, this);
    titleLabel->setObjectName(QStringLiteral("navTitle"));
    titleLabel->setAlignment(Qt::AlignCenter);

    auto *captionLabel = new QLabel(caption, this);
    captionLabel->setObjectName(QStringLiteral("navCaption"));
    captionLabel->setAlignment(Qt::AlignCenter);

    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(16, 20, 16, 16);
    layout->setSpacing(8);
    layout->addWidget(m_iconLabel, 0, Qt::AlignHCenter);
    layout->addWidget(titleLabel);
    layout->addWidget(captionLabel);
    layout->addStretch(1);
}

void NavCard::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton && rect().contains(event->pos()))
        emit clicked();
    QFrame::mouseReleaseEvent(event);
}

HomePage::HomePage(QWidget *parent)
    : QWidget(parent)
{
    setObjectName(QStringLiteral("pageRoot"));

    m_welcomeLabel = new QLabel(QStringLiteral("欢迎使用医院诊疗系统"), this);
    m_welcomeLabel->setObjectName(QStringLiteral("pageTitle"));

    m_subLabel = new QLabel(this);
    m_subLabel->setObjectName(QStringLiteral("pageSubtitle"));

    auto *cardDept = new NavCard(QStringLiteral("科"), QStringLiteral("科室管理"),
                                 QStringLiteral("Department"), this);
    auto *cardDoctor = new NavCard(QStringLiteral("医"), QStringLiteral("医生管理"),
                                   QStringLiteral("Doctor"), this);
    auto *cardPatient = new NavCard(QStringLiteral("患"), QStringLiteral("患者管理"),
                                    QStringLiteral("Patient"), this);

    auto *cardRow = new QHBoxLayout;
    cardRow->setSpacing(34);
    cardRow->addStretch(1);
    cardRow->addWidget(cardDept);
    cardRow->addWidget(cardDoctor);
    cardRow->addWidget(cardPatient);
    cardRow->addStretch(1);

    auto *logButton = new QPushButton(QStringLiteral("查看系统操作日志 →"), this);
    logButton->setObjectName(QStringLiteral("linkButton"));
    logButton->setCursor(Qt::PointingHandCursor);

    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(48, 40, 48, 32);
    layout->addWidget(m_welcomeLabel);
    layout->addWidget(m_subLabel);
    layout->addStretch(1);
    layout->addLayout(cardRow);
    layout->addStretch(1);
    layout->addWidget(logButton, 0, Qt::AlignHCenter);

    connect(cardDept, &NavCard::clicked, this, &HomePage::requestDepartmentPage);
    connect(cardDoctor, &NavCard::clicked, this, &HomePage::requestDoctorPage);
    connect(cardPatient, &NavCard::clicked, this, &HomePage::requestPatientPage);
    connect(logButton, &QPushButton::clicked, this, &HomePage::requestLogPage);

    setUserInfo(QStringLiteral("admin"), QStringLiteral("系统管理员"));
}

void HomePage::setUserInfo(const QString &userName, const QString &fullName)
{
    m_welcomeLabel->setText(QStringLiteral("欢迎回来，%1").arg(fullName.isEmpty() ? userName : fullName));
    const QLocale chinese(QLocale::Chinese, QLocale::China);
    m_subLabel->setText(QStringLiteral("当前账号 %1　|　%2　|　请选择需要进入的功能模块")
                            .arg(userName, chinese.toString(QDate::currentDate(),
                                                            QStringLiteral("yyyy年M月d日 dddd"))));
}
