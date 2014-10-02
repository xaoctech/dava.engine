//
//  BackgroundPropertiesSection.h
//  UIEditor
//
//  Created by Dmitry Belsky on 30.9.14.
//
//

#ifndef __UI_EDITOR_BACKGROUND_PROPERTIES_SECTION_H__
#define __UI_EDITOR_BACKGROUND_PROPERTIES_SECTION_H__

#include "PropertiesSection.h"

class BackgroundPropertiesSection : public PropertiesSection
{
public:
    BackgroundPropertiesSection(DAVA::UIControl *control, int bgNum);
    virtual ~BackgroundPropertiesSection();
    
    DAVA::String GetName() const;
    virtual int GetCount() const;
    
    void HideContent();

private:
    DAVA::UIControl *control;
    int bgNum;
    bool isContentHidden;
};

#endif // __UI_EDITOR_BACKGROUND_PROPERTIES_SECTION_H__
