//
//  UIControlStateHelper.h
//  UIEditor
//
//  Created by Yuri Coder on 10/26/12.
//
//

#ifndef __UIEditor__UIControlStateHelper__
#define __UIEditor__UIControlStateHelper__

#include "UI/UIControl.h"
#include <QString>

namespace DAVA {

class UIControlStateHelper
{
public:
    // Get the list of UIControlStates supported:
    static int GetUIControlStatesCount();

    static UIControl::eControlState GetUIControlState(int stateID);
    static QString GetUIControlStateName(UIControl::eControlState controlState);
    static UIControl::eControlState GetUIControlStateValue(const QString& controlStateName);

    // Default UI Control State data.
    static int GetDefaultControlStateIndex();
    static QString GetDefaultControlStateName();
    static UIControl::eControlState GetDefaultControlState();

private:
    static const int DEFAULT_UI_CONTROL_STATE_INDEX = 0;
    
    // Validate the control state ID.
    static bool ValidateUIControlStateID(int stateID);

    struct CONTROLSTATESMAP
    {
        UIControl::eControlState controlState;
        const char* controlStateName;
    };

    static const CONTROLSTATESMAP controlStatesMap[];
};

};

#endif /* defined(__UIEditor__UIControlStateHelper__) */
