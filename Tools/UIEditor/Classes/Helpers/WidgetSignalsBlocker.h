//
//  WidgetUpdatesBlocker.h
//  UIEditor
//
//  Created by Yuri Coder on 10/24/12.
//
//

#ifndef __UIEditor__WidgetSignalsBlocker__
#define __UIEditor__WidgetSignalsBlocker__

#include <QWidget>
namespace DAVA {
    
// Helper class to block/unblock signals from the widgets.
class WidgetSignalsBlocker
{
public:
    WidgetSignalsBlocker(QWidget* widget);
    ~WidgetSignalsBlocker();
    
private:
    QWidget* widget;
    bool signalsWereBlocked;
};

};

#endif /* defined(__UIEditor__WidgetSignalsBlocker__) */
