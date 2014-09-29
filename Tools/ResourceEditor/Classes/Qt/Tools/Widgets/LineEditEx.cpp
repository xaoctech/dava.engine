#include "LineEditEx.h"

#include <QAction>
#include <QActionEvent>
#include <QDebug>
#include <QHBoxLayout>
#include <QPushButton>
#include <QCursor>

#include "LineEditStyle.h"


LineEditEx::LineEditEx(QWidget* parent)
    : QLineEdit(parent)
    , timer(new QTimer(this))
    , useDelayedUpdate(true)
    , buttonsWidth(0)
{
    timer->setSingleShot(true);
    SetAcceptInterval(500);
    SetUseDelayedUpdate(useDelayedUpdate);

    QHBoxLayout *l = new QHBoxLayout();
    l->setContentsMargins(QMargins());
    l->addStretch();
    setLayout(l);

    LineEditStyle *proxyStyle = new LineEditStyle( style() );
    setStyle(proxyStyle);
}

LineEditEx::~LineEditEx()
{
}

void LineEditEx::SetAcceptInterval(int msec)
{
    const bool wasActive = timer->isActive();
    timer->stop();
    timer->setInterval(msec);
    if (wasActive)
        timer->start();
}

bool LineEditEx::IsDelayedUpdateUsed() const
{
    return useDelayedUpdate;
}

void LineEditEx::SetUseDelayedUpdate(bool use)
{
    useDelayedUpdate = use;

    SetupConnections(false, false); // Reset connections first
    SetupConnections(useDelayedUpdate, !useDelayedUpdate);
}

QAbstractButton* LineEditEx::CreateButton(const QAction * action) const
{
    QPushButton *btn = new QPushButton();
    btn->setToolTip(action->toolTip());
    btn->setIcon(action->icon());

    btn->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    btn->setFixedSize(ButtonSizeHint(action));
    btn->setCursor(Qt::ArrowCursor);

    return btn;
}

QSize LineEditEx::ButtonSizeHint(const QAction* action) const
{
    Q_UNUSED(action);
    return QSize(20, 20);
}

int LineEditEx::ButtonsWidth() const
{
    return buttonsWidth;
}

void LineEditEx::OnTextEdit()
{
    timer->start();
}

void LineEditEx::OnAcceptEdit()
{
    timer->stop();
    emit textUpdated(text());
}

void LineEditEx::UpdatePadding()
{
    buttonsWidth = 0;
    for ( auto it = buttons.constBegin(); it != buttons.constEnd(); ++it )
    {
        buttonsWidth += it.value()->width();
    }
    if ( buttons.size() > 0 )
    {
        buttonsWidth += layout()->spacing() * ( buttons.size() - 1 );
    }
    update();
}

void LineEditEx::SetupConnections(bool delayed, bool instant)
{
    if (delayed)
    {
        connect(timer, SIGNAL( timeout() ), this, SLOT( OnAcceptEdit() ));
        connect(this, SIGNAL( editingFinished() ), this, SLOT( OnAcceptEdit() ));
        connect(this, SIGNAL( textChanged( const QString& ) ), this, SLOT( OnTextEdit() ));
    }
    else
    {
        disconnect(timer, SIGNAL( timeout() ), this, SLOT( OnAcceptEdit() ));
        disconnect(this, SIGNAL( editingFinished() ), this, SLOT( OnAcceptEdit() ));
        disconnect(this, SIGNAL( textChanged( const QString& ) ), this, SLOT( OnTextEdit() ));
    }

    if (instant)
    {
        connect(this, SIGNAL( textChanged( const QString& ) ), this, SIGNAL( textUpdated( const QString& ) ));
    }
    else
    {
        disconnect(this, SIGNAL( textChanged( const QString& ) ), this, SIGNAL( textUpdated( const QString& ) ));
    }
}

void LineEditEx::AddActionHandler(QAction* action)
{
    QAbstractButton *btn = CreateButton(action);
    buttons[action] = btn;
    layout()->addWidget(btn);

    UpdatePadding();
}

void LineEditEx::RemoveActionHandler(QAction* action)
{
    auto it = buttons.find(action);
    if ( it != buttons.end() )
    {
        delete it.value();
        buttons.erase(it);
        UpdatePadding();
    }
}

void LineEditEx::actionEvent(QActionEvent* event)
{
    switch ( event->type() )
    {
    case QEvent::ActionAdded:
        AddActionHandler(event->action());
        break;

    case QEvent::ActionRemoved:
        RemoveActionHandler(event->action());
        break;

    default:
        break;
    }
}

void LineEditEx::paintEvent(QPaintEvent* event)
{
    QLineEdit::paintEvent(event);
}
