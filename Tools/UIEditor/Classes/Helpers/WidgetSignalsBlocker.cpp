//
//  WidgetSignalsBlocker.cpp
//  UIEditor
//
//  Created by Yuri Coder on 10/24/12.
//
//

#include "WidgetSignalsBlocker.h"
using namespace DAVA;

WidgetSignalsBlocker::WidgetSignalsBlocker(QWidget* widget)
{
    this->widget = widget;
    if (this->widget)
    {
        // Save the current signals state and block signals.
        this->signalsWereBlocked = widget->signalsBlocked();
        widget->blockSignals(true);
    }
}


WidgetSignalsBlocker::~WidgetSignalsBlocker()
{
    // Restore the previous signals state.
    if (this->widget)
    {
        this->widget->blockSignals(this->signalsWereBlocked);
    }
}
