#pragma once

#include "Base/BaseTypes.h"
#include "FileSystem/FilePath.h"
#include "Reflection/Reflection.h"
#include "UI/Components/UIComponent.h"
#include "UI/Properties/VarTable.h"

namespace DAVA
{
class UIVisualScriptComponent : public UIComponent
{
    DAVA_VIRTUAL_REFLECTION(UIVisualScriptComponent, UIComponent);
    DECLARE_UI_COMPONENT(UIVisualScriptComponent);

public:
    UIVisualScriptComponent();
    UIVisualScriptComponent(const UIVisualScriptComponent& src);
    UIVisualScriptComponent& operator=(const UIVisualScriptComponent&) = delete;
    UIVisualScriptComponent* Clone() const override;

    const FilePath& GetScriptPath() const;
    void SetScriptPath(const FilePath& filePath);

    bool IsEnabled() const;
    void SetEnabled(bool v);

    bool IsNeedReload() const;
    void SetNeedReload(bool v);

    /** User properties as Map of Any. */
    VarTable& GetProperties();
    void SetProperties(const VarTable& value);

private:
    ~UIVisualScriptComponent() override;

    FilePath scriptPath;
    bool enabled = true;
    bool needReload = true;
    VarTable properties;
};

inline const FilePath& UIVisualScriptComponent::GetScriptPath() const
{
    return scriptPath;
}

inline bool UIVisualScriptComponent::IsEnabled() const
{
    return enabled;
}

inline bool UIVisualScriptComponent::IsNeedReload() const
{
    return needReload;
}
} // namespace DAVA
