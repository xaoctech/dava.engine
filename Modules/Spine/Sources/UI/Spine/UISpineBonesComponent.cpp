#include "UI/Spine/UISpineBonesComponent.h"

#include <Reflection/ReflectionRegistrator.h>
#include <Utils/Utils.h>

namespace DAVA
{

DAVA_VIRTUAL_REFLECTION_IMPL(UISpineBonesComponent)
{
    ReflectionRegistrator<UISpineBonesComponent>::Begin()
    .ConstructorByPointer()
    .DestructorByPointer([](UISpineBonesComponent* c) { SafeRelease(c); })
    .Field("bonesBinds", &UISpineBonesComponent::GetBindsAsString, &UISpineBonesComponent::SetBindsFromString)
    .End();
}

UISpineBonesComponent::UISpineBonesComponent() = default;

UISpineBonesComponent::UISpineBonesComponent(const UISpineBonesComponent& src)
    : UIBaseComponent(src)
    , bonesBinds(src.bonesBinds)
    , cachedBindsString(src.cachedBindsString)
{
}

UISpineBonesComponent::~UISpineBonesComponent() = default;

UISpineBonesComponent* UISpineBonesComponent::Clone() const
{
    return new UISpineBonesComponent(*this);
}

void UISpineBonesComponent::SetBinds(const Vector<std::pair<String, String>>& binds)
{
    if (bonesBinds != binds)
    {
        bonesBinds = binds;
        MakeBindsString();
    }
}

void UISpineBonesComponent::SetBindsFromString(const String& bindsStr)
{
    if (cachedBindsString != bindsStr)
    {
        bonesBinds.clear();
        Vector<String> bindsList;
        Split(bindsStr, ";", bindsList);
        for (const String& bindStr : bindsList)
        {
            Vector<String> keyVal;
            Split(bindStr, ",", keyVal);
            bonesBinds.emplace_back(keyVal.size() > 0 ? keyVal[0] : "", keyVal.size() > 1 ? keyVal[1] : "");
        }
        MakeBindsString();
    }
}

void UISpineBonesComponent::MakeBindsString()
{
    StringStream stream;
    bool first = true;
    for (const std::pair<String, String>& boneBind : bonesBinds)
    {
        if (first)
        {
            first = false;
        }
        else
        {
            stream << ";";
        }
        stream << boneBind.first << "," << boneBind.second;
    }
    cachedBindsString = stream.str();
}

}