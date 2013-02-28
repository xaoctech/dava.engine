//
//  UIStaticTextControlMetadata.cpp
//  UIEditor
//
//  Created by Yuri Coder on 10/23/12.
//
//

#include "UIStaticTextMetadata.h"
#include "EditorFontManager.h"

#include "StringUtils.h"

using namespace DAVA;

UIStaticTextMetadata::UIStaticTextMetadata(QObject* parent) :
    UITextControlMetadata(parent)
{
}

UIStaticText* UIStaticTextMetadata::GetActiveStaticText() const
{
    return dynamic_cast<UIStaticText*>(GetActiveUIControl());
}

Font * UIStaticTextMetadata::GetFont()
{
    if (VerifyActiveParamID())
    {
        return GetActiveStaticText()->GetFont();
    }
    return EditorFontManager::Instance()->GetDefaultFont();
}

void UIStaticTextMetadata::SetFont(Font * font)
{
    if (!VerifyActiveParamID())
    {
        return;
    }
    if (font)
    {
        font->SetSize(GetFontSize());
        GetActiveStaticText()->SetFont(font);
    }
}

void UIStaticTextMetadata::SetLocalizedTextKey(const QString& value)
{
    UITextControlMetadata::SetLocalizedTextKey(value);

    // Update the control with the value.
    WideString localizationValue = LocalizationSystem::Instance()->GetLocalizedString(QStrint2WideString(value));
    GetActiveStaticText()->SetText(localizationValue);
}

float UIStaticTextMetadata::GetFontSize() const
{
    if (VerifyActiveParamID())
    {
        Font *font = GetActiveStaticText()->GetFont();
        if (font)
        {
            return font->GetSize();
        }
    }
    return -1.0f;
}

void UIStaticTextMetadata::SetFontSize(float fontSize)
{
    if (!VerifyActiveParamID())
    {
        return;
    }

    Font *font = GetActiveStaticText()->GetFont();
    if (font)
    {
        font->SetSize(fontSize);
        GetActiveStaticText()->SetFont(font);
    }
}

// Initialize the control(s) attached.
void UIStaticTextMetadata::InitializeControl(const String& controlName, const Vector2& position)
{
    BaseMetadata::InitializeControl(controlName, position);
    
    int paramsCount = this->GetParamsCount();
    for (BaseMetadataParams::METADATAPARAMID i = 0; i < paramsCount; i ++)
    {
        UIStaticText* staticText = dynamic_cast<UIStaticText*>(this->treeNodeParams[i].GetUIControl());

        staticText->SetFont(EditorFontManager::Instance()->GetDefaultFont());
		staticText->SetMultiline(true);
        staticText->GetBackground()->SetDrawType(UIControlBackground::DRAW_ALIGNED);
    
        // Initialize both control text and localization key.
        WideString controlText = StringToWString(staticText->GetName());

        HierarchyTreeNode* activeNode = GetTreeNode(i);
        staticText->SetText(controlText);

        // Static text is not state-aware.
        activeNode->GetExtraData().SetLocalizationKey(controlText, this->GetReferenceState());
    }
}

void UIStaticTextMetadata::UpdateExtraData(HierarchyTreeNodeExtraData& extraData, eExtraDataUpdateStyle updateStyle)
{
    if (!VerifyActiveParamID())
    {
        return;
    }

    // Static Text has one and only state.
    UIControl::eControlState state = UIControl::STATE_NORMAL;
    UpdateStaticTextExtraData(GetActiveStaticText(), state, extraData, updateStyle);
}

QColor UIStaticTextMetadata::GetFontColor() const
{
    if (!VerifyActiveParamID())
    {
        return QColor();
    }

    Font* font = GetActiveStaticText()->GetFont();
    if (font)
    {
        return DAVAColorToQTColor(font->GetColor());
    }
    
    return QColor();
}

void UIStaticTextMetadata::SetFontColor(const QColor& value)
{
    if (!VerifyActiveParamID())
    {
        return;
    }

    GetActiveStaticText()->SetFontColor(QTColorToDAVAColor(value));
}

int UIStaticTextMetadata::GetAlign()
{
    if (!VerifyActiveParamID())
    {
        return ALIGN_LEFT;
    }
    
    return (int)GetActiveStaticText()->GetAlign();
}

void UIStaticTextMetadata::SetAlign(int value)
{
    if (!VerifyActiveParamID())
    {
        return;
    }
    
    GetActiveStaticText()->SetAlign((eAlign)value);
}
