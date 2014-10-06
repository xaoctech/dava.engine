#include "ItemDelegateForVector2.h"
#include <QItemEditorFactory>
#include <QEvent>
#include <QKeyEvent>
#include <QApplication>
#include "QtControls/Vector2DEdit.h"
#include "UIControls/BaseProperty.h"
#include "Utils/QtDavaConvertion.h"
#include "PropertiesTreeModel.h"

ItemDelegateForVector2::ItemDelegateForVector2(PropertiesTreeItemDelegate *delegate)
    : PropertyAbstractEditor(delegate)
{
}

ItemDelegateForVector2::~ItemDelegateForVector2()
{
}

QWidget * ItemDelegateForVector2::createEditor( QWidget * parent, const QStyleOptionViewItem & option, const QModelIndex & index ) const 
{
    Vector2DEdit *vectorEditor = new Vector2DEdit(parent);
    vectorEditor->setObjectName(QString::fromUtf8("vectorEditor"));
    return vectorEditor;
}

void ItemDelegateForVector2::setEditorData( QWidget * editor, const QModelIndex & index ) const 
{
    Vector2DEdit *vectorEditor = editor->findChild<Vector2DEdit *>("vectorEditor");
    vectorEditor->setVector2D(Vector2ToQVector2D(index.data(Qt::EditRole).value<DAVA::VariantType>().AsVector2()));
}

bool ItemDelegateForVector2::setModelData( QWidget * editor, QAbstractItemModel * model, const QModelIndex & index ) const 
{
    if (PropertyAbstractEditor::setModelData(editor, model, index))
        return true;

    Vector2DEdit *vectorEditor = editor->findChild<Vector2DEdit *>("vectorEditor");
    if (!vectorEditor->isModified())
        return true;

    DAVA::VariantType vectorType( QVector2DToVector2(vectorEditor->vector2D()) );
    QVariant vectorVariant;
    vectorVariant.setValue<DAVA::VariantType>(vectorType);
    return model->setData(index, vectorVariant, Qt::EditRole);
}
