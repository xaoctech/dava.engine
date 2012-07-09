//
//  InfoControl.cpp
//  UIViewerMacOS
//
//  Created by Dmitry Shpakov on 6/29/12.
//  Copyright (c) 2012 DAVA Consulting. All rights reserved.
//

#include "InfoControl.h"

InfoControl::InfoControl(UIControl* watched) : UIControl(Rect(0.0f, 0.0f, watched->GetSize().x, watched->GetSize().y), false)
, watchedControl(SafeRetain(watched))
{
    //TODO: remember input enabled
    ParseSpecificInfo();
    watchedControl->SetInputEnabled(false, false);
    
    emptyString = L"";
}
InfoControl::~InfoControl()
{
    specificInfoKeys.clear();
    specificInfoValues.clear();
}

void InfoControl::ParseSpecificInfo()
{
    if(!watchedControl) return;
    
    //TODO: subclass to parse specific info
    WideString key = L"Input:";
    //WideString value = (watchedControl->GetInputEnabled() ? L"Enabled" : L"Disabled");
    WideString value;
    if(watchedControl->GetInputEnabled())
    {
        value = L"Enabled";
    }
    else 
    {
        value = L"Disabled";
    }
    specificInfoKeys.push_back(key);
    specificInfoValues.push_back(value);
}

WideString InfoControl::GetInfoValueText(int32 infoType)
{
    WideString text = L"";
    switch (infoType) 
    {
        case IT_TYPE:
        {
            //TODO: rewrite yaml parsing to get types directly from yaml
            text = StringToWString(watchedControl->GetClassName());
        }
            break;
        case IT_NAME:
        {
            text = StringToWString(watchedControl->GetName());
        }
            break;
        case IT_RECT:
        {
            text = Format(L"[%.1f, %.1f, %.1f, %.1f]", watchedControl->GetRect().x, watchedControl->GetRect().y, watchedControl->GetRect().dx, watchedControl->GetRect().dy);
        }
            break;
        case IT_PIVOT:
        {
            text = Format(L"[%.1f, %.1f]", watchedControl->pivotPoint.x, watchedControl->pivotPoint.y);
        }
            break;
        default:
            break;
    }
    return text;
}

int32 InfoControl::GetSpecificInfoCount()
{
    return specificInfoKeys.size();
}

const WideString &InfoControl::GetSpecificInfoKeyText(int32 index)
{
    if(index < specificInfoKeys.size())
    {
        return specificInfoKeys[index];
    }
    return emptyString;
}

const WideString &InfoControl::GetSpecificInfoValueText(int32 index)
{
    if(index < specificInfoValues.size())
    {
        return specificInfoValues[index];
    }
    return emptyString;
}

void InfoControl::Draw(const UIGeometricData &geometricData)
{
    UIControl::Draw(geometricData);
    
    RenderManager::Instance()->SetColor(GetBackground()->color);
    RenderHelper::Instance()->DrawRect(geometricData.GetUnrotatedRect());
    
    Rect outerRect(geometricData.GetUnrotatedRect());
    outerRect.x -= 1.0f;
    outerRect.y -= 1.0f;
    outerRect.dx += 2.0f;
    outerRect.dy += 2.0f;
    Color outerColor(GetBackground()->color);
    outerColor.r = 1.0f - outerColor.r;
    outerColor.b = 1.0f - outerColor.b;
    
    RenderManager::Instance()->SetColor(outerColor);
    RenderHelper::Instance()->DrawRect(outerRect);
    
    RenderManager::Instance()->ResetColor();
}