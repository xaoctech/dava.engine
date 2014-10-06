#include "PropertyAbstractEditor.h"
#include <QHBoxLayout>
#include <QToolButton>
#include <QAction>

#include "PropertiesTreeModel.h"
#include "PropertiesTreeItemDelegate.h"
#include "Utils/QtDavaConvertion.h"

PropertyAbstractEditor::PropertyAbstractEditor(PropertiesTreeItemDelegate *delegate)
    : itemDelegate(delegate)
{

}

PropertyAbstractEditor::~PropertyAbstractEditor()
{
    itemDelegate = NULL;
}

QWidget * PropertyAbstractEditor::createEditorWidget( QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index ) const
{
    QWidget *editorWidget = new QWidget(parent);
    editorWidget->setObjectName(QString::fromUtf8("editorWidget"));

    QHBoxLayout *horizontalLayout = new QHBoxLayout(editorWidget);
    horizontalLayout->setSpacing(1);
    horizontalLayout->setContentsMargins(0, 0, 0, 0);
    horizontalLayout->setObjectName(QString::fromUtf8("horizontalLayout"));
    editorWidget->setLayout(horizontalLayout);

    editorWidget->setAutoFillBackground(true);
    editorWidget->setFocusPolicy(Qt::StrongFocus);

    //addEditorWidgets(editor, index);
    QWidget *editor = createEditor(editorWidget, option, index);
    if (editor)
        editorWidget->layout()->addWidget(editor);

    QList<QAction *> actions = enumEditorActions(editorWidget, index);
    foreach (QAction *action, actions)
    {
        QToolButton *toolButton = new QToolButton(editorWidget);
        toolButton->setDefaultAction(action);
        toolButton->setIconSize(QSize(12, 12));
        editorWidget->layout()->addWidget(toolButton);
    }

    PropertyAbstractEditor::SetValueModified(editorWidget, false);

    return editorWidget;
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
    QAction *resetAction = qobject_cast<QAction *>(sender());
    if (!resetAction)
        return;

    QWidget *editor = resetAction->parentWidget();
    if (!editor)
        return;

    DAVA::Logger::Debug( "resetClicked editor name %s", QStringToString(editor->objectName()).c_str());

    PropertyAbstractEditor::SetValueReseted(editor, true);
    PropertyAbstractEditor::SetValueModified(editor, true);
    itemDelegate->emitCommitData(editor);
}

QList<QAction *> PropertyAbstractEditor::enumEditorActions( QWidget *parent, const QModelIndex &index ) const
{
    QList<QAction *> actions;

    BaseProperty *property = static_cast<BaseProperty *>(index.internalPointer());
    if (property && property->GetEditFlag() & BaseProperty::EF_CAN_RESET )
    {
        QAction *resetAction = new QAction(QIcon(":/Icons/editclear.png"), tr("reset"), parent);
        resetAction->setToolTip(tr("reset property value to default"));
        actions.push_back(resetAction);
        connect(resetAction, SIGNAL(triggered(bool)), this, SLOT(resetClicked()));
    }

    return actions;
}
