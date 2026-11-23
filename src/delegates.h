#ifndef DELEGATES_H
#define DELEGATES_H

#include <QStyledItemDelegate>
#include <QVector>
#include <QPair>

// 性别代理：编辑时使用下拉框，显示时把 0/1 映射为"女/男"
class SexDelegate : public QStyledItemDelegate
{
    Q_OBJECT
public:
    explicit SexDelegate(QObject *parent = nullptr);

    void paint(QPainter *painter, const QStyleOptionViewItem &option,
               const QModelIndex &index) const override;
    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option,
                          const QModelIndex &index) const override;
    void setEditorData(QWidget *editor, const QModelIndex &index) const override;
    void setModelData(QWidget *editor, QAbstractItemModel *model,
                      const QModelIndex &index) const override;
    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const override;
};

// 外键代理：把科室/医生编号显示为名称，编辑时提供下拉选择
class RelationDelegate : public QStyledItemDelegate
{
    Q_OBJECT
public:
    explicit RelationDelegate(int idColumn, QObject *parent = nullptr);

    void setItems(const QVector<QPair<int, QString>> &items);
    int idColumn() const { return m_idColumn; }

    void paint(QPainter *painter, const QStyleOptionViewItem &option,
               const QModelIndex &index) const override;
    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option,
                          const QModelIndex &index) const override;
    void setEditorData(QWidget *editor, const QModelIndex &index) const override;
    void setModelData(QWidget *editor, QAbstractItemModel *model,
                      const QModelIndex &index) const override;
    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const override;

private:
    int m_idColumn;
    QVector<QPair<int, QString>> m_items;
};

// 日期显示代理：把数据库返回的日期统一格式化为 yyyy-MM-dd
class DateDelegate : public QStyledItemDelegate
{
    Q_OBJECT
public:
    explicit DateDelegate(QObject *parent = nullptr);

    void paint(QPainter *painter, const QStyleOptionViewItem &option,
               const QModelIndex &index) const override;
};

#endif // DELEGATES_H
