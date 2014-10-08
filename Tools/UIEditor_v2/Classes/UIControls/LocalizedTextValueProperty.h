//
//  LocalizedTextValueProperty.h
//  UIEditor
//
//  Created by Dmitry Belsky on 7.10.14.
//
//

#ifndef __UI_EDITOR_LOCALIZED_TEXT_VALUE_PROPERTY__
#define __UI_EDITOR_LOCALIZED_TEXT_VALUE_PROPERTY__

#include "ValueProperty.h"

class LocalizedTextValueProperty : public ValueProperty
{
public:
    LocalizedTextValueProperty(DAVA::BaseObject *object, const DAVA::InspMember *member);
    virtual ~LocalizedTextValueProperty();
    
    virtual DAVA::VariantType GetValue() const override;
    virtual void SetValue(const DAVA::VariantType &newValue) override;
    virtual void ResetValue() override;
  
    virtual void PrepareToEdit() override;

protected:
    DAVA::WideString text;
    bool isEditMode;
};

#endif // __UI_EDITOR_LOCALIZED_TEXT_VALUE_PROPERTY__
