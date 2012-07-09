//
//  InfoControl.h
//  UIViewerMacOS
//
//  Created by Dmitry Shpakov on 6/29/12.
//  Copyright (c) 2012 DAVA Consulting. All rights reserved.
//

#ifndef __INFO_CONTROL_H__
#define __INFO_CONTROL_H__

#include "DAVAEngine.h"

using namespace DAVA;

class InfoControl : public UIControl
{
public:
    InfoControl(UIControl* watched);
    virtual ~InfoControl();
    
    enum eInfoTypes
    {
        IT_TYPE = 0,
        IT_NAME,
        IT_RECT,
        IT_PIVOT,
        
        INFO_TYPES_COUNT
    };
    
    WideString GetInfoValueText(int32 infoType);
    
    int32 GetSpecificInfoCount();
    
    const WideString &GetSpecificInfoKeyText(int32 index);
    const WideString &GetSpecificInfoValueText(int32 index);
    
    virtual void Draw(const UIGeometricData &geometricData);
    
protected:
    virtual void ParseSpecificInfo();
    
    UIControl* watchedControl;
    
    WideString emptyString;
    
    Vector<WideString> specificInfoKeys;
    Vector<WideString> specificInfoValues;
};

#endif //__INFO_CONTROL_H__
