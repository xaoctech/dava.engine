#include "UITextSystemLink.h"
#include "UI/Text/UITextComponent.h"

#include "UI/UIControl.h"
#include "Utils/UTF8Utils.h"
#include "Render/2D/FontManager.h"

namespace DAVA
{
UITextSystemLink::UITextSystemLink(const UITextComponent* component_)
    : BaseObject()
{
    component = const_cast<UITextComponent*>(component_);

    textBlock.Set(TextBlock::Create(Vector2::Zero));

    textBg.Set(new UIControlBackground());
    textBg->SetDrawType(UIControlBackground::DRAW_ALIGNED);

    shadowBg.Set(new UIControlBackground());
    shadowBg->SetDrawType(UIControlBackground::DRAW_ALIGNED);
}

UITextSystemLink::~UITextSystemLink()
{
    component = nullptr;
}
};
