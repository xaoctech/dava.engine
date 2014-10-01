#include "PropertiesTreeQVariantItemDelegate.h"
#include <QItemEditorFactory>
#include <QEvent>
#include <QKeyEvent>
#include <QApplication>
#include "QtControls/Vector2Edit.h"

PropertiesTreeQVariantItemDelegate::PropertiesTreeQVariantItemDelegate( QObject *parent /*= NULL*/ )
    : QStyledItemDelegate(parent)
{
    connect(this, SIGNAL(commitData(QWidget *)), this, SLOT(OnCommitData(QWidget *)));
    connect(this, SIGNAL(closeEditor(QWidget *, QAbstractItemDelegate::EndEditHint)), this, SLOT(OnCloseEditor(QWidget *, QAbstractItemDelegate::EndEditHint)));
}

PropertiesTreeQVariantItemDelegate::~PropertiesTreeQVariantItemDelegate()
{
    disconnect(this, SIGNAL(commitData(QWidget *)), this, SLOT(OnCommitData(QWidget *)));
    disconnect(this, SIGNAL(closeEditor(QWidget *, QAbstractItemDelegate::EndEditHint)), this, SLOT(OnCloseEditor(QWidget *, QAbstractItemDelegate::EndEditHint)));
}

QWidget * PropertiesTreeQVariantItemDelegate::createEditor( QWidget * parent, const QStyleOptionViewItem & option, const QModelIndex & index ) const 
{
    if (index.data(Qt::EditRole).type() != QVariant::Vector2D)
    {
        return NULL;
    }
    return new Vector2Edit(parent);
}

void PropertiesTreeQVariantItemDelegate::setEditorData( QWidget * editor, const QModelIndex & index ) const 
{
    if (index.data(Qt::EditRole).type() != QVariant::Vector2D)
    {
        return;
    }

    Vector2Edit *vectorEditor = static_cast<Vector2Edit *>(editor);
    vectorEditor->setValue(index.data(Qt::EditRole).value<QVector2D>());
}

void PropertiesTreeQVariantItemDelegate::setModelData( QWidget * editor, QAbstractItemModel * model, const QModelIndex & index ) const 
{
    if (index.data(Qt::EditRole).type() != QVariant::Vector2D)
    {
        return;
    }

    Vector2Edit *vectorEditor = static_cast<Vector2Edit *>(editor);
    if (!vectorEditor->isModified())
        return;

    QVector2D vector = vectorEditor->value();
    if (model->setData(index, vector, Qt::EditRole))
    {

    }
    else
    {
        int t=0;
    }
}

// bool PropertiesTreeQVariantItemDelegate::eventFilter( QObject *object, QEvent *event )
// {
//     QWidget *editor = qobject_cast<QWidget*>(object);
//     if (!editor)
//         return false;
// 
//     if (event->type() == QEvent::KeyPress)
//     {
//         switch (static_cast<QKeyEvent *>(event)->key()) {
//         case Qt::Key_Tab:
//             //emit commitData(editor);
//             emit closeEditor(editor, QAbstractItemDelegate::EditNextItem);
//             return true;
//         case Qt::Key_Backtab:
//             //emit commitData(editor);
//             emit closeEditor(editor, QAbstractItemDelegate::EditPreviousItem);
//             return true;
//         case Qt::Key_Enter:
//         case Qt::Key_Return:
//             emit commitData(editor);
//             emit closeEditor(editor, QAbstractItemDelegate::NoHint);
//             return true;
// // #ifndef QT_NO_LINEEDIT
// //             if (QLineEdit *e = qobject_cast<QLineEdit*>(editor))
// //                 if (!e->hasAcceptableInput())
// //                     return false;
// // #endif // QT_NO_LINEEDIT
// //             QMetaObject::invokeMethod(this, "_q_commitDataAndCloseEditor",
// //                 Qt::QueuedConnection, Q_ARG(QWidget*, editor));
// //             return false;
//         case Qt::Key_Escape:
//             // don't commit data
//             emit closeEditor(editor, QAbstractItemDelegate::RevertModelCache);
//             break;
//         default:
//             return false;
//         }
//         if (editor->parentWidget())
//             editor->parentWidget()->setFocus();
//         return true;
//     }
//     else if (event->type() == QEvent::FocusOut || (event->type() == QEvent::Hide && editor->isWindow()))
//     {
//         //the Hide event will take care of he editors that are in fact complete dialogs
//         if (!editor->isActiveWindow() || (QApplication::focusWidget() != editor)) {
//             QWidget *w = QApplication::focusWidget();
//             while (w) { // don't worry about focus changes internally in the editor
//                 if (w == editor)
//                     return false;
//                 w = w->parentWidget();
//             }
//             //emit commitData(editor);
//             emit closeEditor(editor, NoHint);
//         }
//     }
//     else if (event->type() == QEvent::ShortcutOverride)
//     {
//         if (static_cast<QKeyEvent*>(event)->key() == Qt::Key_Escape) {
//             event->accept();
//             return true;
//         }
//     }
//     return false;
// }

bool PropertiesTreeQVariantItemDelegate::IsEditorDataModified(QWidget *editor) const
{
    return false;
}

void PropertiesTreeQVariantItemDelegate::OnCommitData( QWidget *editor )
{
    if (!IsEditorDataModified(editor))
        return;

    emit commitData(editor);
}

void PropertiesTreeQVariantItemDelegate::OnCloseEditor( QWidget *editor, QAbstractItemDelegate::EndEditHint hint )
{
    emit closeEditor(editor);
}
