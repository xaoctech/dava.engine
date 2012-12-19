//
//  UIControlStateHelper.cpp
//  UIEditor
//
//  Created by Yuri Coder on 10/26/12.
//
//

#include "UIControlStateHelper.h"
using namespace DAVA;

const UIControlStateHelper::CONTROLSTATESMAP UIControlStateHelper::controlStatesMap[] =
{
    {UIControl::STATE_NORMAL,           "Normal"},
    {UIControl::STATE_PRESSED_INSIDE,   "Pressed Inside"},
    {UIControl::STATE_PRESSED_OUTSIDE,  "Pressed Outside"},
    {UIControl::STATE_DISABLED,         "Disabled"},
    {UIControl::STATE_SELECTED,         "Selected"},
    {UIControl::STATE_HOVER,            "Hover"}
};

// Get the list of UIControlStates supported:
int UIControlStateHelper::GetUIControlStatesCount()
{
    return sizeof(controlStatesMap) / sizeof(*controlStatesMap);
}
    
UIControl::eControlState UIControlStateHelper::GetUIControlState(int stateID)
{
    if (ValidateUIControlStateID(stateID) == false)
    {
        return UIControl::STATE_NORMAL;
    }

    return controlStatesMap[stateID].controlState;
}

// Default UI Control State data.
int UIControlStateHelper::GetDefaultControlStateIndex()
{
    return UIControlStateHelper::DEFAULT_UI_CONTROL_STATE_INDEX;
}

QString UIControlStateHelper::GetDefaultControlStateName()
{
    return controlStatesMap[GetDefaultControlStateIndex()].controlStateName;
}

UIControl::eControlState UIControlStateHelper::GetDefaultControlState()
{
    return controlStatesMap[GetDefaultControlStateIndex()].controlState;
}

QString UIControlStateHelper::GetUIControlStateName(UIControl::eControlState controlState)
{
    int statesCount = GetUIControlStatesCount();
    for (int i = 0; i < statesCount; i ++)
    {
        if (controlState == controlStatesMap[i].controlState)
        {
            return controlStatesMap[i].controlStateName;
        }
    }
    
    Logger::Error("No Control State Name found for Control State %i!", controlState);
    return QString();
}

UIControl::eControlState UIControlStateHelper::GetUIControlStateValue(const QString& controlStateName)
{
    int statesCount = GetUIControlStatesCount();
    for (int i = 0; i < statesCount; i ++)
    {
        if (controlStateName == controlStatesMap[i].controlStateName)
        {
            return controlStatesMap[i].controlState;
        }
    }
    
    Logger::Error("No Control State found for Control State Name %s!", controlStateName.toStdString().c_str());
    return UIControl::STATE_NORMAL;
}

bool UIControlStateHelper::ValidateUIControlStateID(int stateID)
{
    if (stateID < 0|| stateID >= GetUIControlStatesCount())
    {
        Logger::Error("UI Control State ID %i is invalid!", stateID);
        return false;
    }
    
    return true;
}