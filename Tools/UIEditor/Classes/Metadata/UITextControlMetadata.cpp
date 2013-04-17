#include "UITextControlMetadata.h"
#include "StringUtils.h"

using namespace DAVA;

UITextControlMetadata::UITextControlMetadata(QObject* parent) :
    UIControlMetadata(parent)
{
}

QString UITextControlMetadata::GetLocalizedTextKeyForState(UIControl::eControlState controlState) const
{
    // Return the localization key from the Hierarchy Tree node.
    HierarchyTreeNode* node = this->GetActiveTreeNode();
    if (node)
    {
        return WideString2QStrint(node->GetExtraData().GetLocalizationKey(controlState));
    }
    
    return QString();
}

QString UITextControlMetadata::GetLocalizedTextKey() const
{
    return GetLocalizedTextKeyForState(GetCurrentStateForLocalizedText());
}

void UITextControlMetadata::SetLocalizedTextKey(const QString& value)
{
    if (!VerifyActiveParamID())
    {
        return;
    }
    
    WideString localizationValue = LocalizationSystem::Instance()->GetLocalizedString(QStrint2WideString(value));
    
    // Update the Tree Node Extradata level with the key.
    HierarchyTreeNode* node = this->GetActiveTreeNode();
    if (node)
    {
        node->GetExtraData().SetLocalizationKey(QStrint2WideString(value), GetCurrentStateForLocalizedText());
    }
    
    // Note - the actual update of the control's text should be performed in the derived classes!
}

void UITextControlMetadata::UpdateStaticTextExtraData(UIStaticText* staticText, UIControl::eControlState state,
                                                      HierarchyTreeNodeExtraData& extraData,
                                                      eExtraDataUpdateStyle updateStyle)
{
    if (!staticText)
    {
        DVASSERT(false);
        return;
    }

    switch (updateStyle)
    {
        case BaseMetadata::UPDATE_EXTRADATA_FROM_CONTROL:
        {
            extraData.SetLocalizationKey(staticText->GetText(), state);
            break;
        }
            
        case BaseMetadata::UPDATE_CONTROL_FROM_EXTRADATA_RAW:
        {
            staticText->SetText(extraData.GetLocalizationKey(state));
            break;
        }
            
        case BaseMetadata::UPDATE_CONTROL_FROM_EXTRADATA_LOCALIZED:
        {
            staticText->SetText(LocalizationSystem::Instance()->GetLocalizedString(extraData.GetLocalizationKey(state)));
            break;
        }
            
        default:
        {
            Logger::Error("Unknown Update Style %i!", updateStyle);
            break;
        }
    }
}

Vector2 UITextControlMetadata::GetOffsetX(const Vector2& currentOffset, float offsetX)
{
	Vector2 offset(currentOffset);
	offset.x = offsetX;
	return offset;
}

Vector2 UITextControlMetadata::GetOffsetY(const Vector2& currentOffset, float offsetY)
{
	Vector2 offset(currentOffset);
	offset.y = offsetY;
	return offset;
}