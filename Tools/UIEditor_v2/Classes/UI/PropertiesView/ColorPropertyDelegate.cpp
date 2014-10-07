#include "ColorPropertyDelegate.h"
#include <QToolButton>
#include <QPainter>
#include <QHBoxLayout>
#include <QLabel>
#include <QAction>
#include <QColorDialog>

#include "PropertiesTreeItemDelegate.h"
#include "PropertiesTreeModel.h"
#include "Utils/QtDavaConvertion.h"


ColorPropertyDelegate::ColorPropertyDelegate(PropertiesTreeItemDelegate *delegate)
    : BasePropertyDelegate(delegate)
{
}

ColorPropertyDelegate::~ColorPropertyDelegate()
{

}

QWidget *ColorPropertyDelegate::createEditor( QWidget * parent, const QStyleOptionViewItem & option, const QModelIndex & index ) const
{
    QLabel *label = new QLabel(parent);
    label->setObjectName(QString::fromUtf8("label"));
    return label;
}

void ColorPropertyDelegate::enumEditorActions( QWidget *parent, const QModelIndex &index, QList<QAction *> &actions) const
{
    BasePropertyDelegate::enumEditorActions(parent, index, actions);

    QAction *chooseColor = new QAction(tr("..."), parent);
    connect(chooseColor, SIGNAL(triggered(bool)), this, SLOT(chooseColorClicked()));
    actions.push_front(chooseColor);
}

void ColorPropertyDelegate::setEditorData( QWidget * editor, const QModelIndex & index ) const 
{
    QLabel *label = editor->findChild<QLabel*>("label");
    label->setText(index.data(Qt::DisplayRole).toString());
    label->setProperty("color", ColorToQColor(index.data(Qt::EditRole).value<DAVA::VariantType>().AsColor()));
}

bool ColorPropertyDelegate::setModelData( QWidget * editor, QAbstractItemModel * model, const QModelIndex & index ) const 
{
    if (BasePropertyDelegate::setModelData(editor, model, index))
        return true;

    QLabel *label = editor->findChild<QLabel *>("label");

    DAVA::VariantType color( QColorToColor(label->property("color").value<QColor>()) );
    QVariant colorVariant;
    colorVariant.setValue<DAVA::VariantType>(color);
    return model->setData(index, colorVariant, Qt::EditRole);
}

void ColorPropertyDelegate::chooseColorClicked()
{
    QAction *chooseAction = qobject_cast<QAction *>(sender());
    if (!chooseAction)
        return;

    QWidget *editor = chooseAction->parentWidget();
    if (!editor)
        return;

    QLabel *label = editor->findChild<QLabel *>("label");

    QColorDialog dlg(editor);

    dlg.setOptions(QColorDialog::ShowAlphaChannel | QColorDialog::DontUseNativeDialog);
    dlg.setCurrentColor(label->property("color").value<QColor>());

    if (dlg.exec() == QDialog::Accepted)
    {
        label->setProperty("color", dlg.selectedColor());
        BasePropertyDelegate::SetValueModified(editor, true);
        itemDelegate->emitCommitData(editor);
    }
}
