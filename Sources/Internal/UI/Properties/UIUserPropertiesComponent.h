#pragma once

#include "Base/BaseTypes.h"
#include "Reflection/Reflection.h"
#include "UI/Components/UIComponent.h"
#include "UI/Properties/VarTable.h"

namespace DAVA
{
/**
    User defined local properties. 
    With `properties` property you can specify list of named properties, with type and default value in Quicked.
    All properties will be exported in binding system scope as variables at runtime.
    Properties represents as `Map` of `Any` by `String` name. Instances of `Any` store concrete type and default value of user property.
    Adding UIUserPropertiesComponent in control prototype will creates new section with user properties for prototype instances in Quicked properties panel.
*/
class UIUserPropertiesComponent : public UIComponent
{
    DAVA_VIRTUAL_REFLECTION(UIUserPropertiesComponent, UIComponent);
    DECLARE_UI_COMPONENT(UIUserPropertiesComponent);

public:
    /** Default constructor */
    UIUserPropertiesComponent();
    /** Copy constructor */
    UIUserPropertiesComponent(const UIUserPropertiesComponent& src);

    UIUserPropertiesComponent& operator=(const UIUserPropertiesComponent&) = delete;
    UIUserPropertiesComponent* Clone() const override;

    /** User properties as Map of Any. */
    VarTable& GetProperties();
    void SetProperties(const VarTable& value);

    /** Component has changes */
    bool IsDirty() const;
    void SetDirty(bool dirty_);

private:
    ~UIUserPropertiesComponent() override;

    bool isDirty = false;
    VarTable properties;
};
}
