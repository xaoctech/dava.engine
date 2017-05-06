#pragma once

#include <QEvent>

namespace DAVA
{
namespace TArc
{
class QtOverlayWidgetVisibilityChange : public QEvent
{
public:
    enum
    {
        OverlayWidgetVisibilityChange = QEvent::User + 5000
    };

    QtOverlayWidgetVisibilityChange(bool isVisible_);

    bool IsVisible() const;

private:
    bool isVisible = false;
};

inline QtOverlayWidgetVisibilityChange::QtOverlayWidgetVisibilityChange(bool isVisible_)
    : QEvent(static_cast<QEvent::Type>(OverlayWidgetVisibilityChange))
    , isVisible(isVisible_)
{
}

inline bool QtOverlayWidgetVisibilityChange::IsVisible() const
{
    return isVisible;
}
} // namespace TArc
} // namespace DAVA