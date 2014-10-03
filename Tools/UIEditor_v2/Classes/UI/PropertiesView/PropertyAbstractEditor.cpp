#include "PropertyAbstractEditor.h"
#include <QHBoxLayout>
#include <QToolButton>
#include "PropertiesTreeModel.h"
#include "PropertiesTreeItemDelegate.h"

PropertyAbstractEditor::PropertyAbstractEditor(PropertiesTreeItemDelegate *delegate)
    : itemDelegate(delegate)
{

}

PropertyAbstractEditor::~PropertyAbstractEditor()
{
    itemDelegate = NULL;
}

QWidget * PropertyAbstractEditor::createEditor( QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index ) const
{
    QWidget *editor = new QWidget(parent);

    QHBoxLayout *horizontalLayout = new QHBoxLayout(parent);
    horizontalLayout->setSpacing(1);
    horizontalLayout->setContentsMargins(0, 0, 0, 0);
    horizontalLayout->setObjectName(QString::fromUtf8("horizontalLayout"));
    editor->setLayout(horizontalLayout);

    editor->setAutoFillBackground(true);
    editor->setFocusPolicy(Qt::StrongFocus);

    addEditorWidgets(editor, index);

    PropertyAbstractEditor::SetValueModified(parent, false);

    return editor;
}

void PropertyAbstractEditor::addEditorWidgets( QWidget *parent, const QModelIndex &index ) const
{
    BaseProperty *property = static_cast<BaseProperty *>(index.internalPointer());
    if (property && property->GetEditFlag() & BaseProperty::EF_CAN_RESET )
    {
        QToolButton *resetButton = new QToolButton(parent);
        resetButton->setObjectName(QString::fromUtf8("resetButton"));
        resetButton->setIcon(QIcon(":/Icons/editclear.png"));
        resetButton->setIconSize(QSize(12, 12));
        parent->layout()->addWidget(resetButton);

        connect(resetButton, SIGNAL(clicked()), this, SLOT(resetClicked()));
    }
    PropertyAbstractEditor::SetValueReseted(parent, false);
}

bool PropertyAbstractEditor::setModelData( QWidget *editor, QAbstractItemModel *model, const QModelIndex &index ) const
{
    if (!PropertyAbstractEditor::IsValueModified(editor))
        return true;

    if (PropertyAbstractEditor::IsValueReseted(editor))
    {
        if (model->setData(index, QVariant(), DAVA::ResetRole))
        {
            PropertyAbstractEditor::SetValueModified(editor, false);
            PropertyAbstractEditor::SetValueReseted(editor, false);
            return true;
        }
    }

    return false;

}

void PropertyAbstractEditor::resetClicked()
{
    QWidget *resetButton = qobject_cast<QWidget *>(sender());
    if (!resetButton)
        return;

    QWidget *editor = resetButton->parentWidget();
    if (!editor)
        return;

    PropertyAbstractEditor::SetValueReseted(editor, true);
    PropertyAbstractEditor::SetValueModified(editor, true);
    itemDelegate->emitCommitData(editor);
}
