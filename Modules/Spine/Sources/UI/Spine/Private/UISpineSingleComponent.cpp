#include "UI/Spine/UISpineSingleComponent.h"

#include <UI/UIControl.h>

namespace DAVA
{
void UISpineSingleComponent::Clear()
{
    spineModified.clear();
    spineNeedReload.clear();
    spineBonesModified.clear();
}
}
