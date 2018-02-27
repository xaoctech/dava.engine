#pragma once

#include "Base/BaseTypes.h"
#include "UI/Components/UIComponent.h"

namespace DAVA
{
class FormulaExpression;

class UIDataScopeComponent : public UIComponent
{
    DAVA_VIRTUAL_REFLECTION(UIDataScopeComponent, UIComponent);
    DECLARE_UI_COMPONENT(UIDataScopeComponent);

public:
    UIDataScopeComponent() = default;
    UIDataScopeComponent(const UIDataScopeComponent& c);

    UIDataScopeComponent& operator=(const UIDataScopeComponent& c) = delete;

    UIDataScopeComponent* Clone() const override;

    const String& GetExpression() const;
    void SetExpression(const String& name);

    bool IsDirty() const;
    void SetDirty(bool dirty_);

protected:
    ~UIDataScopeComponent() override = default;

private:
    String expression;
    bool isDirty = false;
};
}
