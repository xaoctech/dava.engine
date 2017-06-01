#pragma once

#include <Base/BaseTypes.h>
#include <UI/Components/UISingleComponent.h>

namespace DAVA
{
class UIControl;

struct UISpineSingleComponent : public UISingleComponent
{
    UnorderedSet<UIControl*> spineModified;
    UnorderedSet<UIControl*> spineNeedReload;
    UnorderedSet<UIControl*> spineBonesModified;

    void Clear() override;
};
}
