#include "patientdialog.h"
#include "dbmanager.h"

#include <QComboBox>
#include <QDateEdit>
#include <QDoubleSpinBox>
#include <QFormLayout>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QRegularExpression>
#include <QShortcut>
#include <QSpinBox>
#include <QSqlError>
#include <QSqlQuery>
#include <QVBoxLayout>

PatientDialog::PatientDialog(Mode mode, int patientId, QWidget *parent)
    : QDialog(parent)
    , m_mode(mode)
    , m_patientId(patientId)
{
    setWindowTitle(mode == AddMode ? QStringLiteral("新增患者") : QStringLiteral("编辑患者信息"));
    setModal(true);
    setMinimumWidth(680);
    buildUi();
    loadDepartments();

    if (mode == EditMode)
        loadPatient();
    else
        m_dobEdit->setDate(QDate(1990, 1, 1));

    syncAgeFromDob();
}

void PatientDialog::buildUi()
{
    m_nameEdit = new QLineEdit(this);
    m_nameEdit->setMaxLength(20);
    m_nameEdit->setPlaceholderText(QStringLiteral("必填，如：张三"));

    m_sexCombo = new QComboBox(this);
    m_sexCombo->addItem(QStringLiteral("男"), 1);
    m_sexCombo->addItem(QStringLiteral("女"), 0);

    m_idCardEdit = new QLineEdit(this);
    m_idCardEdit->setMaxLength(18);
    m_idCardEdit->setPlaceholderText(QStringLiteral("必填，18 位身份证号"));

    m_dobEdit = new QDateEdit(this);
    m_dobEdit->setCalendarPopup(true);
    m_dobEdit->setDisplayFormat(QStringLiteral("yyyy-MM-dd"));
    m_dobEdit->setDateRange(QDate(1900, 1, 1), QDate::currentDate());

    m_ageSpin = new QSpinBox(this);
    m_ageSpin->setRange(0, 150);
    m_ageSpin->setSuffix(QStringLiteral(" 岁"));

    m_phoneEdit = new QLineEdit(this);
    m_phoneEdit->setMaxLength(11);
    m_phoneEdit->setPlaceholderText(QStringLiteral("必填，11 位手机号"));

    m_heightSpin = new QDoubleSpinBox(this);
    m_heightSpin->setRange(30.0, 250.0);
    m_heightSpin->setDecimals(1);
    m_heightSpin->setSuffix(QStringLiteral(" cm"));
    m_heightSpin->setValue(170.0);

    m_weightSpin = new QDoubleSpinBox(this);
    m_weightSpin->setRange(10.0, 300.0);
    m_weightSpin->setDecimals(1);
    m_weightSpin->setSuffix(QStringLiteral(" kg"));
    m_weightSpin->setValue(65.0);

    m_deptCombo = new QComboBox(this);
    m_doctorCombo = new QComboBox(this);

    m_addressEdit = new QLineEdit(this);
    m_addressEdit->setMaxLength(60);
    m_addressEdit->setPlaceholderText(QStringLiteral("选填"));

    m_diagnosisEdit = new QLineEdit(this);
    m_diagnosisEdit->setMaxLength(120);
    m_diagnosisEdit->setPlaceholderText(QStringLiteral("选填，如：社区获得性肺炎"));

    m_visitDateEdit = new QDateEdit(this);
    m_visitDateEdit->setCalendarPopup(true);
    m_visitDateEdit->setDisplayFormat(QStringLiteral("yyyy-MM-dd"));
    m_visitDateEdit->setDate(QDate::currentDate());

    auto *grid = new QGridLayout;
    grid->setHorizontalSpacing(12);
    grid->setVerticalSpacing(12);

    auto addLabel = [this, grid](const QString &text, int row, int col) {
        auto *label = new QLabel(text, this);
        label->setObjectName(QStringLiteral("formLabel"));
        label->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
        grid->addWidget(label, row, col);
    };

    addLabel(QStringLiteral("姓　　名 *"), 0, 0);
    grid->addWidget(m_nameEdit, 0, 1);
    addLabel(QStringLiteral("性　　别"), 0, 2);
    grid->addWidget(m_sexCombo, 0, 3);

    addLabel(QStringLiteral("身份证号 *"), 1, 0);
    grid->addWidget(m_idCardEdit, 1, 1);
    addLabel(QStringLiteral("出生日期"), 1, 2);
    grid->addWidget(m_dobEdit, 1, 3);

    addLabel(QStringLiteral("年　　龄"), 2, 0);
    grid->addWidget(m_ageSpin, 2, 1);
    addLabel(QStringLiteral("手机号码 *"), 2, 2);
    grid->addWidget(m_phoneEdit, 2, 3);

    addLabel(QStringLiteral("身高(cm)"), 3, 0);
    grid->addWidget(m_heightSpin, 3, 1);
    addLabel(QStringLiteral("体重(kg)"), 3, 2);
    grid->addWidget(m_weightSpin, 3, 3);

    addLabel(QStringLiteral("就诊科室 *"), 4, 0);
    grid->addWidget(m_deptCombo, 4, 1);
    addLabel(QStringLiteral("主治医生 *"), 4, 2);
    grid->addWidget(m_doctorCombo, 4, 3);

    addLabel(QStringLiteral("住　　址"), 5, 0);
    grid->addWidget(m_addressEdit, 5, 1, 1, 3);

    addLabel(QStringLiteral("诊　　断"), 6, 0);
    grid->addWidget(m_diagnosisEdit, 6, 1, 1, 3);

    addLabel(QStringLiteral("就诊日期"), 7, 0);
    grid->addWidget(m_visitDateEdit, 7, 1);

    m_tipLabel = new QLabel(QStringLiteral("带 * 的字段为必填项，身份证号与手机号需符合格式要求。"), this);
    m_tipLabel->setObjectName(QStringLiteral("hintLabel"));
    m_tipLabel->setWordWrap(true);

    auto *saveButton = new QPushButton(QStringLiteral("保 存"), this);
    saveButton->setObjectName(QStringLiteral("primaryButton"));
    saveButton->setDefault(true);
    saveButton->setCursor(Qt::PointingHandCursor);
    auto *cancelButton = new QPushButton(QStringLiteral("取 消"), this);
    cancelButton->setObjectName(QStringLiteral("ghostButton"));
    cancelButton->setCursor(Qt::PointingHandCursor);

    auto *buttonRow = new QHBoxLayout;
    buttonRow->addStretch(1);
    buttonRow->addWidget(saveButton, 0);
    buttonRow->addWidget(cancelButton, 0);

    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(26, 22, 26, 18);
    layout->setSpacing(14);
    layout->addLayout(grid);
    layout->addWidget(m_tipLabel);
    layout->addLayout(buttonRow);

    setTabOrder(m_nameEdit, m_sexCombo);
    setTabOrder(m_sexCombo, m_idCardEdit);
    setTabOrder(m_idCardEdit, m_dobEdit);
    setTabOrder(m_dobEdit, m_ageSpin);
    setTabOrder(m_ageSpin, m_phoneEdit);
    setTabOrder(m_phoneEdit, m_heightSpin);
    setTabOrder(m_heightSpin, m_weightSpin);
    setTabOrder(m_weightSpin, m_deptCombo);
    setTabOrder(m_deptCombo, m_doctorCombo);
    setTabOrder(m_doctorCombo, m_addressEdit);
    setTabOrder(m_addressEdit, m_diagnosisEdit);
    setTabOrder(m_diagnosisEdit, m_visitDateEdit);
    setTabOrder(m_visitDateEdit, saveButton);

    connect(saveButton, &QPushButton::clicked, this, &PatientDialog::accept);
    connect(cancelButton, &QPushButton::clicked, this, &QDialog::reject);
    connect(m_deptCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &PatientDialog::reloadDoctors);
    connect(m_dobEdit, &QDateEdit::dateChanged, this, &PatientDialog::syncAgeFromDob);

    // 字段快捷键：Alt+字母 直接定位到对应输入框，方便键盘录入
    const QVector<QPair<QString, QWidget *>> fieldShortcuts = {
        {QStringLiteral("Alt+N"), m_nameEdit},
        {QStringLiteral("Alt+I"), m_idCardEdit},
        {QStringLiteral("Alt+B"), m_dobEdit},
        {QStringLiteral("Alt+A"), m_ageSpin},
        {QStringLiteral("Alt+M"), m_phoneEdit},
        {QStringLiteral("Alt+H"), m_heightSpin},
        {QStringLiteral("Alt+W"), m_weightSpin},
        {QStringLiteral("Alt+D"), m_deptCombo},
        {QStringLiteral("Alt+R"), m_doctorCombo},
        {QStringLiteral("Alt+S"), m_addressEdit},
        {QStringLiteral("Alt+G"), m_diagnosisEdit},
        {QStringLiteral("Alt+V"), m_visitDateEdit}
    };
    for (const auto &item : fieldShortcuts) {
        auto *shortcut = new QShortcut(QKeySequence(item.first), this);
        QWidget *target = item.second;
        connect(shortcut, &QShortcut::activated, this, [target]() {
            target->setFocus();
            if (auto *edit = qobject_cast<QLineEdit *>(target))
                edit->selectAll();
        });
    }

    m_nameEdit->setFocus();
}

void PatientDialog::loadDepartments()
{
    m_deptCombo->clear();
    QSqlQuery query(DatabaseManager::instance().database());
    query.exec(QStringLiteral("SELECT DeptID, DeptName FROM dbo.Department ORDER BY DeptID"));
    while (query.next())
        m_deptCombo->addItem(query.value(1).toString(), query.value(0).toInt());
    reloadDoctors();
}

void PatientDialog::reloadDoctors()
{
    const int currentDoctor = m_doctorCombo->currentData().toInt();
    const int deptId = m_deptCombo->currentData().toInt();

    m_doctorCombo->clear();
    QSqlQuery query(DatabaseManager::instance().database());
    query.prepare(QStringLiteral(
        "SELECT DoctorID, DoctorName, Title FROM dbo.Doctor WHERE DeptID = ? ORDER BY DoctorID"));
    query.addBindValue(deptId);
    if (!query.exec())
        return;
    while (query.next()) {
        m_doctorCombo->addItem(QStringLiteral("%1（%2）")
                                   .arg(query.value(1).toString(), query.value(2).toString()),
                               query.value(0).toInt());
    }
    if (currentDoctor > 0) {
        const int pos = m_doctorCombo->findData(currentDoctor);
        if (pos >= 0)
            m_doctorCombo->setCurrentIndex(pos);
    }
}

void PatientDialog::syncAgeFromDob()
{
    const QDate dob = m_dobEdit->date();
    if (!dob.isValid())
        return;
    const int age = static_cast<int>(dob.daysTo(QDate::currentDate()) / 365.25);
    if (age >= 0 && age <= 150)
        m_ageSpin->setValue(age);
}

void PatientDialog::loadPatient()
{
    QSqlQuery query(DatabaseManager::instance().database());
    query.prepare(QStringLiteral(
        "SELECT PatientName, Sex, IdCard, DOB, Age, Height, Weight, MobilePhone, Address,"
        " DeptID, DoctorID, Diagnosis, VisitDate FROM dbo.Patient WHERE PatientID = ?"));
    query.addBindValue(m_patientId);
    if (!query.exec() || !query.next()) {
        QMessageBox::warning(this, QStringLiteral("提示"), QStringLiteral("未能读取该患者记录。"));
        return;
    }

    m_nameEdit->setText(query.value(0).toString());
    m_sexCombo->setCurrentIndex(query.value(1).toInt() == 1 ? 0 : 1);
    m_idCardEdit->setText(query.value(2).toString());
    const QDate dob = query.value(3).toDate();
    if (dob.isValid())
        m_dobEdit->setDate(dob);
    m_ageSpin->setValue(query.value(4).toInt());
    m_heightSpin->setValue(query.value(5).toDouble());
    m_weightSpin->setValue(query.value(6).toDouble());
    m_phoneEdit->setText(query.value(7).toString());
    m_addressEdit->setText(query.value(8).toString());

    const int deptPos = m_deptCombo->findData(query.value(9).toInt());
    if (deptPos >= 0)
        m_deptCombo->setCurrentIndex(deptPos);
    reloadDoctors();
    const int doctorPos = m_doctorCombo->findData(query.value(10).toInt());
    if (doctorPos >= 0)
        m_doctorCombo->setCurrentIndex(doctorPos);

    m_diagnosisEdit->setText(query.value(11).toString());
    const QDate visit = query.value(12).toDate();
    if (visit.isValid())
        m_visitDateEdit->setDate(visit);
}

bool PatientDialog::validateInput(QString *message, QWidget **focusWidget)
{
    const QString name = m_nameEdit->text().trimmed();
    if (name.isEmpty()) {
        *message = QStringLiteral("患者姓名不能为空，请填写必填项。");
        *focusWidget = m_nameEdit;
        return false;
    }

    const QString idCard = m_idCardEdit->text().trimmed();
    static const QRegularExpression idPattern(QStringLiteral("^\\d{17}[0-9Xx]$"));
    if (!idPattern.match(idCard).hasMatch()) {
        *message = QStringLiteral("身份证号格式不正确，应为 18 位（末位可为 X）。");
        *focusWidget = m_idCardEdit;
        return false;
    }

    const QString phone = m_phoneEdit->text().trimmed();
    static const QRegularExpression phonePattern(QStringLiteral("^1[3-9]\\d{9}$"));
    if (!phonePattern.match(phone).hasMatch()) {
        *message = QStringLiteral("手机号格式不正确，应为 1 开头的 11 位数字。");
        *focusWidget = m_phoneEdit;
        return false;
    }

    if (m_deptCombo->currentData().toInt() <= 0) {
        *message = QStringLiteral("请选择就诊科室。");
        *focusWidget = m_deptCombo;
        return false;
    }
    if (m_doctorCombo->currentData().toInt() <= 0) {
        *message = QStringLiteral("请选择主治医生。");
        *focusWidget = m_doctorCombo;
        return false;
    }

    // 身份证唯一性校验（修改时排除自身）
    QSqlQuery dup(DatabaseManager::instance().database());
    dup.prepare(QStringLiteral(
        "SELECT COUNT(*) FROM dbo.Patient WHERE IdCard = ? AND PatientID <> ?"));
    dup.addBindValue(idCard);
    dup.addBindValue(m_mode == EditMode ? m_patientId : -1);
    if (dup.exec() && dup.next() && dup.value(0).toInt() > 0) {
        *message = QStringLiteral("身份证号 %1 已存在，请勿重复建档。").arg(idCard);
        *focusWidget = m_idCardEdit;
        return false;
    }

    return true;
}

void PatientDialog::accept()
{
    QString message;
    QWidget *focusWidget = nullptr;
    if (!validateInput(&message, &focusWidget)) {
        QMessageBox::warning(this, QStringLiteral("数据校验未通过"), message);
        if (focusWidget)
            focusWidget->setFocus();
        return;
    }

    QSqlQuery query(DatabaseManager::instance().database());
    if (m_mode == AddMode) {
        query.prepare(QStringLiteral(
            "INSERT INTO dbo.Patient (PatientName, Sex, IdCard, DOB, Age, Height, Weight,"
            " MobilePhone, Address, DeptID, DoctorID, Diagnosis, VisitDate)"
            " OUTPUT INSERTED.PatientID VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)"));
    } else {
        query.prepare(QStringLiteral(
            "UPDATE dbo.Patient SET PatientName = ?, Sex = ?, IdCard = ?, DOB = ?, Age = ?,"
            " Height = ?, Weight = ?, MobilePhone = ?, Address = ?, DeptID = ?, DoctorID = ?,"
            " Diagnosis = ?, VisitDate = ? WHERE PatientID = ?"));
    }

    query.addBindValue(m_nameEdit->text().trimmed());
    query.addBindValue(m_sexCombo->currentData().toInt());
    query.addBindValue(m_idCardEdit->text().trimmed());
    query.addBindValue(m_dobEdit->date());
    query.addBindValue(m_ageSpin->value());
    query.addBindValue(m_heightSpin->value());
    query.addBindValue(m_weightSpin->value());
    query.addBindValue(m_phoneEdit->text().trimmed());
    query.addBindValue(m_addressEdit->text().trimmed());
    query.addBindValue(m_deptCombo->currentData().toInt());
    query.addBindValue(m_doctorCombo->currentData().toInt());
    query.addBindValue(m_diagnosisEdit->text().trimmed());
    query.addBindValue(m_visitDateEdit->date());
    if (m_mode == EditMode)
        query.addBindValue(m_patientId);

    if (!query.exec()) {
        QMessageBox::critical(this, QStringLiteral("保存失败"),
                              QStringLiteral("数据库写入失败：\n%1").arg(query.lastError().text().trimmed()));
        return;
    }

    if (m_mode == AddMode && query.next())
        m_savedId = query.value(0).toInt();
    else
        m_savedId = m_patientId;

    QDialog::accept();
}
