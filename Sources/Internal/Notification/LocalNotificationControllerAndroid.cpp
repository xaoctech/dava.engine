#include "DAVAEngine.h"

using namespace DAVA;

void LocalNotificationController::PostDelayedNotification(const WideString &title, const WideString text, int delaySeconds)
{
    LocalNotificationDelayed *notification = new LocalNotificationDelayed();
    notification->SetTitle(title);
    notification->SetText(text);
    notification->SetDelaySeconds(delaySeconds);
    notification->Show();
    SafeRelease(notification);
}

void LocalNotificationController::RemoveAllDelayedNotifications()
{
    LocalNotificationDelayed *notification = new LocalNotificationDelayed();
    notification->RemoveAllDelayedNotifications();
    SafeRelease(notification);
}
