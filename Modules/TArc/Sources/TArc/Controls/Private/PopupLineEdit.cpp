#include "TArc/Controls/PopupLineEdit.h"

#include <QtEvents>
#include <QVBoxLayout>

namespace DAVA
{
namespace TArc
{
PopupLineEdit::PopupLineEdit(const ControlDescriptorBuilder<LineEdit::Fields>& fields, DataWrappersProcessor* wrappersProcessor, Reflection model, QWidget* parent)
    : QWidget(parent, Qt::Popup)
{
    edit = new LineEdit(fields, wrappersProcessor, model, this);
    SetupControl();
}

PopupLineEdit::PopupLineEdit(const ControlDescriptorBuilder<LineEdit::Fields>& fields, ContextAccessor* accessor, Reflection model, QWidget* parent)
    : QWidget(parent, Qt::Popup)
{
    edit = new LineEdit(fields, accessor, model, this);
    SetupControl();
}

PopupLineEdit::~PopupLineEdit()
{
    edit->TearDown();
}

void PopupLineEdit::Show(const QPoint& position)
{
    move(position);
    show();
    edit->ToWidgetCast()->setFocus(Qt::PopupFocusReason);
}

void PopupLineEdit::Show(const QRect& geometry)
{
    resize(geometry.size());
    move(geometry.topLeft());
    show();
    edit->ToWidgetCast()->setFocus(Qt::PopupFocusReason);
}

bool PopupLineEdit::eventFilter(QObject* obj, QEvent* e)
{
    DVASSERT(obj == edit->ToWidgetCast());
    if (e->type() == QEvent::KeyRelease)
    {
        QKeyEvent* keyEvent = static_cast<QKeyEvent*>(e);
        if (keyEvent->matches(QKeySequence::Cancel))
        {
            edit->ForceUpdate();
        }
        else if (keyEvent->key() == Qt::Key_Enter ||
                 keyEvent->key() == Qt::Key_Return)
        {
            deleteLater();
        }
    }
    if (e->type() == QEvent::FocusOut)
    {
        deleteLater();
    }

    return false;
}

void PopupLineEdit::SetupControl()
{
    setAttribute(Qt::WA_DeleteOnClose, true);
    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->addWidget(edit->ToWidgetCast());

    edit->ToWidgetCast()->installEventFilter(this);
}

} // namespace TArc
} // namespace DAVA