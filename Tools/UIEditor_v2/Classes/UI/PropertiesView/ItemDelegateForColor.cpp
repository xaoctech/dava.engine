#include "ItemDelegateForColor.h"
#include <QToolButton>
#include <QPainter>
#include <QHBoxLayout>
#include <QLabel>
#include "PropertiesTreeItemDelegate.h"
#include "PropertiesTreeModel.h"

ItemDelegateForColor::ItemDelegateForColor(PropertiesTreeItemDelegate *delegate)
    : PropertyAbstractEditor(delegate)
{
}

ItemDelegateForColor::~ItemDelegateForColor()
{

}

void ItemDelegateForColor::addEditorWidgets(QWidget *parent, const QModelIndex &index) const
{
    QLabel *label = new QLabel(parent);
    label->setObjectName(QString::fromUtf8("label"));
    parent->layout()->addWidget(label);

    QToolButton *chooseButton = new QToolButton(parent);
    chooseButton->setObjectName(QString::fromUtf8("chooseButton"));
    chooseButton->setText("...");
    parent->layout()->addWidget(chooseButton);
    connect(chooseButton, SIGNAL(clicked()), this, SLOT(chooseColorClicked()));

    PropertyAbstractEditor::addEditorWidgets(parent, index);
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
