#ifndef PATIENTDIALOG_H
#define PATIENTDIALOG_H

#include <QDialog>

class QComboBox;
class QDateEdit;
class QDoubleSpinBox;
class QLabel;
class QLineEdit;
class QSpinBox;

// 患者信息编辑对话框：新增/修改共用，保存前做必填与格式校验
class PatientDialog : public QDialog
{
    Q_OBJECT
public:
    enum Mode { AddMode, EditMode };

    PatientDialog(Mode mode, int patientId, QWidget *parent = nullptr);

    int savedId() const { return m_savedId; }

protected:
    void accept() override;

private slots:
    void reloadDoctors();
    void syncAgeFromDob();

private:
    void buildUi();
    void loadDepartments();
    void loadPatient();
    bool validateInput(QString *message, QWidget **focusWidget);

    Mode m_mode;
    int m_patientId = -1;
    int m_savedId = -1;

    QLineEdit *m_nameEdit = nullptr;
    QComboBox *m_sexCombo = nullptr;
    QLineEdit *m_idCardEdit = nullptr;
    QDateEdit *m_dobEdit = nullptr;
    QSpinBox *m_ageSpin = nullptr;
    QLineEdit *m_phoneEdit = nullptr;
    QDoubleSpinBox *m_heightSpin = nullptr;
    QDoubleSpinBox *m_weightSpin = nullptr;
    QComboBox *m_deptCombo = nullptr;
    QComboBox *m_doctorCombo = nullptr;
    QLineEdit *m_addressEdit = nullptr;
    QLineEdit *m_diagnosisEdit = nullptr;
    QDateEdit *m_visitDateEdit = nullptr;
    QLabel *m_tipLabel = nullptr;
};

#endif // PATIENTDIALOG_H
