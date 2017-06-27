#include "UI/Spine/UISpineAttachControlsToBonesComponent.h"

#include "UI/Spine/UISpineSingleComponent.h"
#include "UI/Spine/UISpineSystem.h"

#include <Reflection/ReflectionRegistrator.h>
#include <UI/UIControlSystem.h>
#include <Utils/Utils.h>

namespace DAVA
{
DAVA_VIRTUAL_REFLECTION_IMPL(UISpineAttachControlsToBonesComponent)
{
    ReflectionRegistrator<UISpineAttachControlsToBonesComponent>::Begin()[M::DisplayName("SpineAttachControlsToBones")]
    .ConstructorByPointer()
    .DestructorByPointer([](UISpineAttachControlsToBonesComponent* c) { SafeRelease(c); })
    .Field("bonesBinds", &UISpineAttachControlsToBonesComponent::GetBindsAsString, &UISpineAttachControlsToBonesComponent::SetBindsFromString)
    .End();
}

UISpineAttachControlsToBonesComponent::UISpineAttachControlsToBonesComponent() = default;

UISpineAttachControlsToBonesComponent::UISpineAttachControlsToBonesComponent(const UISpineAttachControlsToBonesComponent& src)
    : UIComponent(src)
    , bonesBinds(src.bonesBinds)
    , cachedBindsString(src.cachedBindsString)
{
}

UISpineAttachControlsToBonesComponent::~UISpineAttachControlsToBonesComponent() = default;

UISpineAttachControlsToBonesComponent* UISpineAttachControlsToBonesComponent::Clone() const
{
    return new UISpineAttachControlsToBonesComponent(*this);
}

void UISpineAttachControlsToBonesComponent::SetBinds(const Vector<AttachInfo>& binds)
{
    if (bonesBinds != binds)
    {
        bonesBinds = binds;
        MakeBindsString();
        Modify();
    }
}

void UISpineAttachControlsToBonesComponent::SetBindsFromString(const String& bindsStr)
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
            bonesBinds.push_back(AttachInfo{ keyVal.size() > 0 ? keyVal[0] : "", keyVal.size() > 1 ? keyVal[1] : "" });
        }
        MakeBindsString();
        Modify();
    }
}

void UISpineAttachControlsToBonesComponent::MakeBindsString()
{
    StringStream stream;
    bool first = true;
    for (const AttachInfo& boneBind : bonesBinds)
    {
        if (first)
        {
            first = false;
        }
        else
        {
            stream << ";";
        }
        stream << boneBind.boneName << "," << boneBind.controlPath;
    }
    cachedBindsString = stream.str();
}

void UISpineAttachControlsToBonesComponent::Modify()
{
    UIControl* control = GetControl();
    UIControlSystem* scene = control->GetScene();
    if (scene)
    {
        UISpineSingleComponent* spineSingle = scene->GetSingleComponent<UISpineSingleComponent>();
        if (spineSingle)
        {
            spineSingle->spineBonesModified.insert(control);
        }
    }
}

bool UISpineAttachControlsToBonesComponent::AttachInfo::operator==(const AttachInfo& other) const
{
    return boneName == other.boneName && controlPath == other.controlPath;
}

bool UISpineAttachControlsToBonesComponent::AttachInfo::operator!=(const AttachInfo& other) const
{
    return !operator==(other);
}
}