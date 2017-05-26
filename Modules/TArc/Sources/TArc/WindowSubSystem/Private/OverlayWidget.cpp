#include "TArc/WindowSubSystem/Private/OverlayWidget.h"
#include "TArc/WindowSubSystem/Private/QtEvents.h"
#include "TArc/WindowSubSystem//QtTArcEvents.h"

#include <QHBoxLayout>
#include <QKeyEvent>

namespace DAVA
{
namespace TArc
{
OverlayWidget::OverlayWidget(const OverCentralPanelInfo& info, QWidget* content_, QWidget* parent)
    : QFrame(parent)
    , geometryProccessor(info.geometryProcessor)
    , content(content_)
{
    setWindowFlags(static_cast<Qt::WindowFlags>(Qt::FramelessWindowHint | Qt::Tool));
    setFrameShadow(QFrame::Sunken);
    setFrameShape(QFrame::Panel);
    setLineWidth(2);
    setFocusProxy(content);

    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->setMargin(4);
    layout->setSpacing(0);

    DVASSERT(content != nullptr);
    layout->addWidget(content);
    content->installEventFilter(this);
    parent->installEventFilter(this);
    parent->window()->installEventFilter(this);

    setVisible(content->isVisible());
}

bool OverlayWidget::eventFilter(QObject* obj, QEvent* e)
{
    QEvent::Type t = e->type();
    if (obj == content)
    {
        if (t == QT_EVENT_TYPE(EventsTable::OverlayWidgetVisibilityChange))
        {
            bool isVisible = static_cast<QtOverlayWidgetVisibilityChange*>(e)->IsVisible();
            executor.DelayedExecute([this, isVisible]()
                                    {
                                        setVisible(isVisible);
                                        UpdateGeometry();
                                        if (isVisible == true)
                                        {
                                            activateWindow();
                                        }
                                    });
            return true;
        }
    }

    if (t == QEvent::Resize || QEvent::Move)
    {
        executor.DelayedExecute([this]()
                                {
                                    UpdateGeometry();
                                });
    }

    return false;
}

void OverlayWidget::UpdateGeometry()
{
    QWidget* parentW = parentWidget();
    QRect r = geometryProccessor->GetWidgetGeometry(parentWidget(), content);

    QPoint pivot = parentW->mapToGlobal(r.topLeft());
    move(pivot);
    resize(r.size());
}

} // namespace TArc
} // namespace DAVA