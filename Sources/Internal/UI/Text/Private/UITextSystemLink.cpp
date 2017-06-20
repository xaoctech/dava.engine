#include "UITextSystemLink.h"
#include "UI/Text/UITextComponent.h"

#include "UI/UIControl.h"
#include "Utils/UTF8Utils.h"
#include "Render/2D/FontManager.h"

namespace DAVA
{
UITextSystemLink::UITextSystemLink(UIControl* control_, UITextComponent* component_)
{
    control = control_;
    component = component_;

    Rect rect = control->GetRect();
    control->SetInputEnabled(false, false);
    textBlock.Set(TextBlock::Create(Vector2(rect.dx, rect.dy)));

    textBg.Set(new UIControlBackground());
    textBg->SetDrawType(UIControlBackground::DRAW_ALIGNED);
    textBg->SetColorInheritType(component->GetColorInheritType());
    textBg->SetPerPixelAccuracyType(component->GetPerPixelAccuracyType());

    shadowBg.Set(new UIControlBackground());
    shadowBg->SetDrawType(UIControlBackground::DRAW_ALIGNED);
    shadowBg->SetColorInheritType(component->GetColorInheritType());
    shadowBg->SetPerPixelAccuracyType(component->GetPerPixelAccuracyType());
}

};
