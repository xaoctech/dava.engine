#ifndef STATECOMBOBOXITEMDELEGATE_H
#define STATECOMBOBOXITEMDELEGATE_H

#include <QItemDelegate>
#include <QPainter>
#include <QStyleOptionViewItem>
#include <QModelIndex>
#include <QList>

// Delegate reimplamentation
class StateComboBoxItemDelegate : public QItemDelegate
{
    Q_OBJECT

public:
    StateComboBoxItemDelegate(QObject *parent = 0);
    void SetBoldTextIndexesList(const QList<int>& textIndexesList);

    virtual void paint(QPainter * painter, const QStyleOptionViewItem & option, const QModelIndex & index) const;
    
private:
    QList<int> boldTextIndexesList;
};

#endif // STATECOMBOBOXITEMDELEGATE_H
