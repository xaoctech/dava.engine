#ifndef __PROPERTYABSTRACTEDITOR_H__
#define __PROPERTYABSTRACTEDITOR_H__

#include <QObject>
#include <QWidget>
#include <QModelIndex>
#include <QStyleOptionViewItem>

class PropertiesTreeItemDelegate;

class PropertyAbstractEditor: public QObject
{
    Q_OBJECT
public:
    explicit PropertyAbstractEditor(PropertiesTreeItemDelegate *delegate = NULL);
    virtual ~PropertyAbstractEditor();
    virtual QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const;
    virtual void addEditorWidgets(QWidget *parent, const QModelIndex &index) const;;
    virtual void setEditorData(QWidget *editor, const QModelIndex &index) const = 0;
    virtual bool setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const;
    virtual void paint( QPainter * painter, QStyleOptionViewItem & option, const QModelIndex & index ) const{}

    static bool IsValueModified(QWidget *editor)
    {
        QVariant data = editor->property("valueModified");
        return data.toBool();
    }

    static void SetValueModified(QWidget *editor, bool value)
    {
        editor->setProperty("valueModified", QVariant(value));
    }

    static bool IsValueReseted(QWidget *editor)
    {
        QVariant data = editor->property("valueReseted");
        return data.toBool();
    }

    static void SetValueReseted(QWidget *editor, bool value)
    {
        editor->setProperty("valueReseted", QVariant(value));
    }
private slots:
    void resetClicked();

protected:
    PropertiesTreeItemDelegate *itemDelegate;

};

#endif // __PROPERTYABSTRACTEDITOR_H__
