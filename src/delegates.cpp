#include "delegates.h"

#include <QComboBox>
#include <QDate>
#include <QDateTime>
#include <QPainter>
#include <QStyle>
#include <QApplication>

namespace {

// 在当前 option 的基础上，替换显示文本后交给样式绘制，保证与主题一致
void drawText(QPainter *painter, const QStyleOptionViewItem &option,
              const QString &text)
{
    QStyleOptionViewItem opt(option);
    opt.text = text;
    const QWidget *widget = opt.widget;
    QStyle *style = widget ? widget->style() : QApplication::style();
    style->drawControl(QStyle::CE_ItemViewItem, &opt, painter, widget);
}

} // namespace

SexDelegate::SexDelegate(QObject *parent)
    : QStyledItemDelegate(parent)
{
}

void SexDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option,
                        const QModelIndex &index) const
{
    const int value = index.data(Qt::DisplayRole).toInt();
    drawText(painter, option, value == 1 ? QStringLiteral("男") : QStringLiteral("女"));
}

QWidget *SexDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &,
                                   const QModelIndex &) const
{
    auto *editor = new QComboBox(parent);
    editor->addItem(QStringLiteral("男"), 1);
    editor->addItem(QStringLiteral("女"), 0);
    return editor;
}

void SexDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
    auto *combo = qobject_cast<QComboBox *>(editor);
    if (!combo)
        return;
    const int value = index.data(Qt::DisplayRole).toInt();
    combo->setCurrentIndex(value == 1 ? 0 : 1);
}

void SexDelegate::setModelData(QWidget *editor, QAbstractItemModel *model,
                               const QModelIndex &index) const
{
    auto *combo = qobject_cast<QComboBox *>(editor);
    if (!combo)
        return;
    model->setData(index, combo->currentData().toInt(), Qt::EditRole);
}

QSize SexDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QSize size = QStyledItemDelegate::sizeHint(option, index);
    size.setWidth(qMax(size.width(), 56));
    return size;
}

RelationDelegate::RelationDelegate(int idColumn, QObject *parent)
    : QStyledItemDelegate(parent)
    , m_idColumn(idColumn)
{
}

void RelationDelegate::setItems(const QVector<QPair<int, QString>> &items)
{
    m_items = items;
}

void RelationDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option,
                             const QModelIndex &index) const
{
    const int id = index.data(Qt::DisplayRole).toInt();
    QString text;
    for (const auto &item : m_items) {
        if (item.first == id) {
            text = item.second;
            break;
        }
    }
    drawText(painter, option, text);
}

QWidget *RelationDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &,
                                        const QModelIndex &) const
{
    auto *editor = new QComboBox(parent);
    for (const auto &item : m_items)
        editor->addItem(item.second, item.first);
    return editor;
}

void RelationDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
    auto *combo = qobject_cast<QComboBox *>(editor);
    if (!combo)
        return;
    const int id = index.data(Qt::DisplayRole).toInt();
    const int pos = combo->findData(id);
    if (pos >= 0)
        combo->setCurrentIndex(pos);
}

void RelationDelegate::setModelData(QWidget *editor, QAbstractItemModel *model,
                                    const QModelIndex &index) const
{
    auto *combo = qobject_cast<QComboBox *>(editor);
    if (!combo)
        return;
    model->setData(index, combo->currentData().toInt(), Qt::EditRole);
}

QSize RelationDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QSize size = QStyledItemDelegate::sizeHint(option, index);
    size.setWidth(qMax(size.width(), 90));
    return size;
}

DateDelegate::DateDelegate(QObject *parent)
    : QStyledItemDelegate(parent)
{
}

void DateDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option,
                         const QModelIndex &index) const
{
    const QVariant value = index.data(Qt::DisplayRole);
    QString text;
    if (value.canConvert<QDate>()) {
        const QDate date = value.toDate();
        text = date.isValid() ? date.toString(QStringLiteral("yyyy-MM-dd")) : QString();
    } else {
        text = value.toString();
        const int t = text.indexOf(QLatin1Char('T'));
        if (t > 0)
            text = text.left(t);
        if (text.endsWith(QLatin1String(" 00:00:00")))
            text.chop(9);
    }
    drawText(painter, option, text);
}
