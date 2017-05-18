#pragma once

#include <Base/BaseTypes.h>
#include <Reflection/Reflection.h>
#include <UI/Components/UIComponent.h>

namespace DAVA
{

class UISpineBonesComponent : public UIBaseComponent<UISpineBonesComponent>
{
    DAVA_VIRTUAL_REFLECTION(UISpineBonesComponent, UIComponent);

public:
    UISpineBonesComponent();
    UISpineBonesComponent(const UISpineBonesComponent& src);
    UISpineBonesComponent& operator=(const UISpineBonesComponent&) = delete;

    UISpineBonesComponent* Clone() const override;

    const Vector<std::pair<String, String>>& GetBinds() const;
    void SetBinds(const Vector<std::pair<String, String>>& binds);

    // const String& GetControlPath(const String& boneName) const;
    // const String& GetBoneName(const String& controlPath) const;
    // void PutBind(const String& boneName, const String& controlPath);
    // void RemoveBindByBone(const String& boneName);
    // void RemoveBindByControl(const String& contolPath);

protected:
    ~UISpineBonesComponent() override;

    const String& GetBindsAsString() const;
    void SetBindsFromString(const String& bindsStr);
    void MakeBindsString();

private:
    Vector<std::pair<String, String>> bonesBinds;
    String cachedBindsString;
};

inline const Vector<std::pair<String, String>>& UISpineBonesComponent::GetBinds() const
{
    return bonesBinds;
}

inline const String& UISpineBonesComponent::GetBindsAsString() const
{
    return cachedBindsString;
}

}