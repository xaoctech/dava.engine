#import <UIKit/UIKit.h>
#include "DAVAEngine.h"
#include "NSStringUtils.h"

using namespace DAVA;

void LocalNotificationController::PostDelayedNotification(const WideString &title, const WideString text, int delaySeconds)
{
    UILocalNotification *notification = [[[UILocalNotification alloc] init] autorelease];
    notification.alertBody = NSStringFromWideString(text);
    notification.timeZone = [NSTimeZone defaultTimeZone];
    notification.fireDate = [NSDate dateWithTimeIntervalSinceNow:delaySeconds];
    [[UIApplication sharedApplication] scheduleLocalNotification:notification];
}

void LocalNotificationController::RemoveAllDelayedNotifications()
{
    for(UILocalNotification *notification in [[UIApplication sharedApplication] scheduledLocalNotifications])
    {
        [[UIApplication sharedApplication] cancelLocalNotification:notification];
    }
}
