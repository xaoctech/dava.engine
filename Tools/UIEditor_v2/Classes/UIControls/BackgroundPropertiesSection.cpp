//
//  BackgroundPropertiesSection.cpp
//  UIEditor
//
//  Created by Dmitry Belsky on 30.9.14.
//
//

#include "BackgroundPropertiesSection.h"

using namespace DAVA;

BackgroundPropertiesSection::BackgroundPropertiesSection(UIControl *control, int bgNum) : control(NULL), bgNum(bgNum)
{
    this->control = SafeRetain(control);
}

BackgroundPropertiesSection::~BackgroundPropertiesSection()
{
    SafeRelease(control);
}

DAVA::String BackgroundPropertiesSection::GetName() const
{
    return control->GetBackgroundComponentName(bgNum);
}
