#include "ItemDelegateForColor.h"
#include <QToolButton>
#include <QPainter>
#include <QHBoxLayout>
#include <QLabel>
#include <QAction>
#include "PropertiesTreeItemDelegate.h"
#include "PropertiesTreeModel.h"


ItemDelegateForColor::ItemDelegateForColor(PropertiesTreeItemDelegate *delegate)
    : PropertyAbstractEditor(delegate)
{
}

ItemDelegateForColor::~ItemDelegateForColor()
{

}

QWidget *ItemDelegateForColor::createEditor( QWidget * parent, const QStyleOptionViewItem & option, const QModelIndex & index ) const
{
    QLabel *label = new QLabel(parent);
    label->setObjectName(QString::fromUtf8("label"));
    return label;
}

QList<QAction *> ItemDelegateForColor::enumEditorActions( QWidget *parent, const QModelIndex &index ) const
{
    QList<QAction *> actions = PropertyAbstractEditor::enumEditorActions(parent, index);

    QAction *chooseColor = new QAction(tr("..."), parent);
    connect(chooseColor, SIGNAL(triggered(bool)), this, SLOT(chooseColorClicked()));
    actions.push_front(chooseColor);

    return actions;
}

void ItemDelegateForColor::setEditorData( QWidget * editor, const QModelIndex & index ) const 
{
    DAVA::Logger::Debug("ItemDelegateForColor::setEditorData");
    QLabel *label = editor->findChild<QLabel*>("label");
    label->setText(index.data(Qt::DisplayRole).toString());
}

bool ItemDelegateForColor::setModelData( QWidget * editor, QAbstractItemModel * model, const QModelIndex & index ) const 
{
    if (PropertyAbstractEditor::setModelData(editor, model, index))
        return true;

    DAVA::Logger::Debug("ItemDelegateForColor::setModelData");
    return false;
}

void ItemDelegateForColor::chooseColorClicked()
{
    
}
