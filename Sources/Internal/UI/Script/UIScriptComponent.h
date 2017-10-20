#pragma once

#include "Base/BaseTypes.h"
#include "FileSystem/FilePath.h"
#include "Reflection/Reflection.h"
#include "UI/Components/UIComponent.h"

namespace DAVA
{
class UIScriptComponent : public UIComponent
{
    DAVA_VIRTUAL_REFLECTION(UIScriptComponent, UIComponent);
    IMPLEMENT_UI_COMPONENT(UIScriptComponent);

public:
    UIScriptComponent();
    UIScriptComponent(const UIScriptComponent& src);
    UIScriptComponent& operator=(const UIScriptComponent&) = delete;
    UIScriptComponent* Clone() const override;

    const String& GetReflectionTypeName() const;
    void SetReflectionTypeName(const String& typeName);

    const FilePath& GetLuaScriptPath() const;
    void SetLuaScriptPath(const FilePath& filePath);

    const String& GetParameters() const;
    void SetParameters(const String& value);

    bool GetModifiedParameters() const;
    void SetModifiedParameters(bool value);

    bool GetModifiedScripts() const;
    void SetModifiedScripts(bool value);

private:
    ~UIScriptComponent() override;

    String reflectionTypeName;
    FilePath luaScriptPath;
    String parameters;

    bool modifiedParameters = false;
    bool modifiedScripts = false;
};

inline const String& UIScriptComponent::GetReflectionTypeName() const
{
    return reflectionTypeName;
}

inline const FilePath& UIScriptComponent::GetLuaScriptPath() const
{
    return luaScriptPath;
}

inline const String& UIScriptComponent::GetParameters() const
{
    return parameters;
}
}
