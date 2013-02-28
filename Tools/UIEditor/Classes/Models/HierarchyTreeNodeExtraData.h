//
//  HierarchyTreeNodeExtraData.h
//  UIEditor
//
//  Created by Yuri Coder on 11/5/12.
//
//

#ifndef __UIEditor__HierarchyTreeNodeExtraData__
#define __UIEditor__HierarchyTreeNodeExtraData__

#include <QString>

#include "Base/BaseTypes.h"
#include "UI/UIControl.h"

namespace DAVA {
    
// Extra data class for Hierarchy Tree Node.
class HierarchyTreeNodeExtraData
{
    
public:
    HierarchyTreeNodeExtraData();
    
    // Access to Localization Key.
    void SetLocalizationKey(const WideString& localizationKey, UIControl::eControlState state);
    WideString GetLocalizationKey(UIControl::eControlState state) const;

    // Extended Dirty States support - for particular Properties.
    // "State is dirty" means that the content for this state was changed
    // in comparison with default state.
    void SetStatePropertyDirty(UIControl::eControlState controlState, const QString& propertyName, bool value);
    bool IsStatePropertyDirty(UIControl::eControlState controlState, const QString& propertyName);

    // Whether the "state property map" is empty for particular state? This means that
    // the property values for this state are identical to ones in Reference property.
    bool IsStatePropertyDirtyMapEmpty(UIControl::eControlState controlState);
    
private:
    // Validate the state.
    bool ValidateControlState(UIControl::eControlState state) const;

    // Localization Key. This property is state-aware.
    Map<UIControl::eControlState, WideString> localizationKeysMap;
    
    // Extended "dirty" properties list.
    typedef Map<UIControl::eControlState, Set<QString> > STATEDIRTYPROPERTIESMAP;
    typedef STATEDIRTYPROPERTIESMAP::iterator STATEDIRTYPROPERTIESITER;
    typedef STATEDIRTYPROPERTIESMAP::const_iterator STATEDIRTYPROPERTIESCONSTITER;
    
    STATEDIRTYPROPERTIESMAP stateDirtyProperties;
};

};

#endif /* defined(__UIEditor__HierarchyTreeNodeExtraData__) */
