#ifndef __PROPERTYABSTRACTEDITOR_H__
#define __PROPERTYABSTRACTEDITOR_H__

#include <QObject>
#include <QWidget>
#include <QModelIndex>
#include <QStyleOptionViewItem>

class PropertyAbstractEditor: public QObject
{
    Q_OBJECT
public:
    virtual QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const = 0;
    virtual void setEditorData(QWidget *editor, const QModelIndex &index) const = 0;
    virtual void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const = 0;

    static bool IsValueModified(QWidget *editor)
    {
        QVariant data = editor->property("modified");
        return data.toBool();
    }

    static void SetValueModified(QWidget *editor, bool value)
    {
        editor->setProperty("modified", QVariant(value));
    }
};

#endif // __PROPERTYABSTRACTEDITOR_H__
