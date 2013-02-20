//
//  HierarchyTreeNodeExtraData.cpp
//  UIEditor
//
//  Created by Yuri Coder on 11/5/12.
//
//

#include "HierarchyTreeNodeExtraData.h"

using namespace DAVA;

HierarchyTreeNodeExtraData::HierarchyTreeNodeExtraData()
{
    // Init the map.
    this->localizationKeysMap.insert(std::make_pair(UIControl::STATE_NORMAL, WideString()));
    this->localizationKeysMap.insert(std::make_pair(UIControl::STATE_PRESSED_INSIDE, WideString()));
    this->localizationKeysMap.insert(std::make_pair(UIControl::STATE_PRESSED_OUTSIDE, WideString()));
    this->localizationKeysMap.insert(std::make_pair(UIControl::STATE_DISABLED, WideString()));
    this->localizationKeysMap.insert(std::make_pair(UIControl::STATE_SELECTED, WideString()));
    this->localizationKeysMap.insert(std::make_pair(UIControl::STATE_HOVER, WideString()));
}

// Access to Localization Key.
void HierarchyTreeNodeExtraData::SetLocalizationKey(const WideString& localizationKey, UIControl::eControlState state)
{
    if (!ValidateControlState(state))
    {
        return;
    }

    this->localizationKeysMap[state] = localizationKey;
}
    
WideString HierarchyTreeNodeExtraData::GetLocalizationKey(UIControl::eControlState state) const
{
    if (!ValidateControlState(state))
    {
        return WideString();
    }

    //return this->localizationKeysMap[state];
    
    Map<UIControl::eControlState, WideString>::const_iterator iter = this->localizationKeysMap.find(state);
    return iter->second;
}

bool HierarchyTreeNodeExtraData::ValidateControlState(UIControl::eControlState state) const
{
    Map<UIControl::eControlState, WideString>::const_iterator iter = this->localizationKeysMap.find(state);
    if (iter == this->localizationKeysMap.end())
    {
        Logger::Error("UIControlState %i is invalid in HierarchyTreeNodeExtraData!");
        return false;
    }
    
    return true;
}

void HierarchyTreeNodeExtraData::SetStatePropertyDirty(UIControl::eControlState controlState,
                                                       const QString& propertyName, bool value)
{
    STATEDIRTYPROPERTIESITER iter = this->stateDirtyProperties.find(controlState);
    if (iter == stateDirtyProperties.end())
    {
        Set<QString> propertiesSet;
        if (value)
        {
            propertiesSet.insert(propertyName);
        }

        this->stateDirtyProperties.insert(std::make_pair(controlState, propertiesSet));
        return;
    }
    
    Set<QString>& propertiesSet = this->stateDirtyProperties[controlState];
    if (value)
    {
        propertiesSet.insert(propertyName);
    }
    else
    {
        propertiesSet.erase(propertyName);
    }
}

bool HierarchyTreeNodeExtraData::IsStatePropertyDirty(UIControl::eControlState controlState,
                                                      const QString& propertyName)
{
    STATEDIRTYPROPERTIESITER iter = this->stateDirtyProperties.find(controlState);
    if (iter == stateDirtyProperties.end())
    {
        return false;
    }
    
    Set<QString>::iterator innerIter = iter->second.find(propertyName);
    return (innerIter != iter->second.end());
}

bool HierarchyTreeNodeExtraData::IsStatePropertyDirtyMapEmpty(UIControl::eControlState controlState)
{
    STATEDIRTYPROPERTIESITER iter = this->stateDirtyProperties.find(controlState);
    if (iter == stateDirtyProperties.end())
    {
        // No changes for this property at all.
        return true;
    }
    
    return (iter->second.size() == 0);
}
