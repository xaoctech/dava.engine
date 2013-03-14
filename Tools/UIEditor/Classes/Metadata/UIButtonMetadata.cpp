    //
//  ButtonNodeMetadata.cpp
//  UIEditor
//
//  Created by Yuri Coder on 10/15/12.
//
//

#include "UIButtonMetadata.h"
#include "EditorFontManager.h"
#include "UIControlStateHelper.h"

#include "PropertyNames.h"
#include "StringUtils.h"

using namespace DAVA;

UIButtonMetadata::UIButtonMetadata(QObject* parent) :
    UITextControlMetadata(parent)
{
}

UIButton* UIButtonMetadata::GetActiveUIButton() const
{
    return dynamic_cast<UIButton*>(GetActiveUIControl());
}

void UIButtonMetadata::SetLocalizedTextKey(const QString& value)
{
    if (!VerifyActiveParamID())
    {
        return;
    }

    UITextControlMetadata::SetLocalizedTextKey(value);
    
    // Update the control with the value.
	WideString localizationValue = LocalizationSystem::Instance()->GetLocalizedString(QStrint2WideString(value));
    GetActiveUIButton()->SetStateText(this->uiControlState, localizationValue);

    UpdatePropertyDirtyFlagForLocalizedText();
}

void UIButtonMetadata::UpdatePropertyDirtyFlagForLocalizedText()
{
    // Compare all the states with reference one.
    int statesCount = UIControlStateHelper::GetUIControlStatesCount();
    for (int i = 0; i < statesCount; i ++)
    {
        UIControl::eControlState curState = UIControlStateHelper::GetUIControlState(i);
        if (curState == GetReferenceState())
        {
            continue;
        }
            
        bool curStateDirty = (GetLocalizedTextKeyForState(curState) !=
                              GetLocalizedTextKeyForState(GetReferenceState()));
        SetStateDirtyForProperty(curState, PropertyNames::LOCALIZED_TEXT_KEY_PROPERTY_NAME, curStateDirty);
    }
}

Font * UIButtonMetadata::GetFont()
{
    if (VerifyActiveParamID())
    {
        return GetFontForState(this->uiControlState);
    }
    return EditorFontManager::Instance()->GetDefaultFont();
}

void UIButtonMetadata::SetFont(Font * font)
{
    if (!VerifyActiveParamID())
    {
        return;
    }
    if (font)
    {
        font->SetSize(GetFontSize());
        GetActiveUIButton()->SetStateFont(this->uiControlState, font);
        UpdatePropertyDirtyFlagForFont();
    }
}

void UIButtonMetadata::UpdatePropertyDirtyFlagForFont()
{
    int statesCount = UIControlStateHelper::GetUIControlStatesCount();
    for (int i = 0; i < statesCount; i ++)
    {
        UIControl::eControlState curState = UIControlStateHelper::GetUIControlState(i);

        bool curStateDirty = (GetFontForState(curState) !=
                              GetFontForState(GetReferenceState()));
        SetStateDirtyForProperty(curState, PropertyNames::FONT_PROPERTY_NAME, curStateDirty);
    }
}

Font * UIButtonMetadata::GetFontForState(UIControl::eControlState state) const
{
    UIStaticText *buttonText = GetActiveUIButton()->GetStateTextControl(state);
    if (buttonText)
    {
        return buttonText->GetFont();
    }
    return EditorFontManager::Instance()->GetDefaultFont();
}

float UIButtonMetadata::GetFontSize() const
{
    if (!VerifyActiveParamID())
    {
        return -1.0f;
    }
    
    return GetFontSizeForState(this->uiControlState);
}

void UIButtonMetadata::SetFontSize(float fontSize)
{
    if (!VerifyActiveParamID())
    {
        return;
    }

    UIStaticText *buttonText = GetActiveUIButton()->GetStateTextControl(this->uiControlState);
    if (!buttonText)
    {
        return;
    }
    
    Font *font = buttonText->GetFont();
    if (!font)
    {
        return;
    }
        
    font->SetSize(fontSize);
    buttonText->SetFont(font);

    UpdatePropertyDirtyFlagForFontSize();
}

void UIButtonMetadata::UpdatePropertyDirtyFlagForFontSize()
{
    int statesCount = UIControlStateHelper::GetUIControlStatesCount();
    for (int i = 0; i < statesCount; i ++)
    {
        UIControl::eControlState curState = UIControlStateHelper::GetUIControlState(i);

        bool curStateDirty = (GetFontSizeForState(curState) !=
                              GetFontSizeForState(GetReferenceState()));
        SetStateDirtyForProperty(curState, PropertyNames::FONT_SIZE_PROPERTY_NAME, curStateDirty);
    }
}

float UIButtonMetadata::GetFontSizeForState(UIControl::eControlState state) const
{
   UIStaticText* referenceButtonText = GetActiveUIButton()->GetStateTextControl(state);
   if (referenceButtonText)
    {
        Font* referenceFont = referenceButtonText->GetFont();
        if (referenceFont)
        {
            return referenceFont->GetSize();
        }
    }
    
    return -1.0f;
}

QColor UIButtonMetadata::GetFontColor() const
{
    if (!VerifyActiveParamID())
    {
        return -1.0f;
    }
    
    return GetFontColorForState(this->uiControlState);
}

void UIButtonMetadata::SetFontColor(const QColor& value)
{
    if (!VerifyActiveParamID())
    {
        return;
    }
    
    GetActiveUIButton()->SetStateFontColor(this->uiControlState, QTColorToDAVAColor(value));
    UpdatePropertyDirtyFlagForFontColor();
}

float UIButtonMetadata::GetShadowOffsetX() const
{
    if (VerifyActiveParamID())
    {
		UIStaticText* referenceButtonText = GetActiveUIButton()->GetStateTextControl(this->uiControlState);
    	if (referenceButtonText)
    	{
			return referenceButtonText->GetShadowOffset().x;
    	}
	}
    
	return -1.0f;	
}

void UIButtonMetadata::SetShadowOffsetX(float offset)
{
    if (!VerifyActiveParamID())
    {
        return;
    }
	
	UIStaticText* referenceButtonText = GetActiveUIButton()->GetStateTextControl(this->uiControlState);
    if (referenceButtonText)
    {
		Vector2 shadowOffset = GetOffsetX(referenceButtonText->GetShadowOffset(), offset);
		referenceButtonText->SetShadowOffset(shadowOffset);
    }
}
	
float UIButtonMetadata::GetShadowOffsetY() const
{
    if (VerifyActiveParamID())
    {
		UIStaticText* referenceButtonText = GetActiveUIButton()->GetStateTextControl(this->uiControlState);
    	if (referenceButtonText)
    	{
			return referenceButtonText->GetShadowOffset().y;
    	}
	}
    
	return -1.0f;	
}

void UIButtonMetadata::SetShadowOffsetY(float offset)
{
    if (!VerifyActiveParamID())
    {
        return;
    }
	
	UIStaticText* referenceButtonText = GetActiveUIButton()->GetStateTextControl(this->uiControlState);
    if (referenceButtonText)
    {
		Vector2 shadowOffset = GetOffsetY(referenceButtonText->GetShadowOffset(), offset);
		referenceButtonText->SetShadowOffset(shadowOffset);
    }
}
	
QColor UIButtonMetadata::GetShadowColor() const
{
    if (VerifyActiveParamID())
    {
		UIStaticText* referenceButtonText = GetActiveUIButton()->GetStateTextControl(this->uiControlState);
    	if (referenceButtonText)
    	{
			return DAVAColorToQTColor(referenceButtonText->GetShadowColor());
    	}
	}
    
	return QColor();
}

void UIButtonMetadata::SetShadowColor(const QColor& value)
{
    if (!VerifyActiveParamID())
    {
        return;
    }
	
	UIStaticText* referenceButtonText = GetActiveUIButton()->GetStateTextControl(this->uiControlState);
    if (referenceButtonText)
    {
		referenceButtonText->SetShadowColor(QTColorToDAVAColor(value));
    }
}

void UIButtonMetadata::UpdatePropertyDirtyFlagForFontColor()
{
    int statesCount = UIControlStateHelper::GetUIControlStatesCount();
    for (int i = 0; i < statesCount; i ++)
    {
        UIControl::eControlState curState = UIControlStateHelper::GetUIControlState(i);
        
        bool curStateDirty = (GetFontColorForState(curState) !=
                              GetFontColorForState(GetReferenceState()));
        SetStateDirtyForProperty(curState, PropertyNames::FONT_COLOR_PROPERTY_NAME, curStateDirty);
    }
}

QColor UIButtonMetadata::GetFontColorForState(UIControl::eControlState state) const
{
    UIStaticText* referenceButtonText = GetActiveUIButton()->GetStateTextControl(state);
    if (referenceButtonText)
    {
		return DAVAColorToQTColor(referenceButtonText->GetTextColor());
    }
    
    return QColor();
}

void UIButtonMetadata::SetSprite(const QString& value)
{
    if (!VerifyActiveParamID())
    {
        return;
    }
    
    //If empty string value is used - remove sprite
    if (value.isEmpty())
    {
        GetActiveUIButton()->SetStateSprite(this->uiControlState, NULL);
    }
    else
    {
        GetActiveUIButton()->SetStateSprite(this->uiControlState,
                                            TruncateTxtFileExtension(value).toStdString());
    }

    UpdatePropertyDirtyFlagForSpriteName();
}

void UIButtonMetadata::UpdatePropertyDirtyFlagForSpriteName()
{
    int statesCount = UIControlStateHelper::GetUIControlStatesCount();
    for (int i = 0; i < statesCount; i ++)
    {
        UIControl::eControlState curState = UIControlStateHelper::GetUIControlState(i);
            
        bool curStateDirty = (GetSpriteNameForState(curState) !=
                              GetSpriteNameForState(this->GetReferenceState()));
        SetStateDirtyForProperty(curState, PropertyNames::SPRITE_PROPERTY_NAME, curStateDirty);
    }
}

QString UIButtonMetadata::GetSpriteNameForState(UIControl::eControlState state) const
{
    Sprite* sprite = GetActiveUIButton()->GetStateSprite(state);
    if (sprite == NULL)
    {
        return "<No sprite is set>";
    }

    return sprite->GetRelativePathname().c_str();
}

QString UIButtonMetadata::GetSprite()
{
    if (!VerifyActiveParamID())
    {
        return QString();
    }
    
    return GetSpriteNameForState(this->uiControlState);
}

void UIButtonMetadata::SetSpriteFrame(int value)
{
    if (!VerifyActiveParamID())
    {
        return;
    }
    
    Sprite* sprite = GetActiveUIButton()->GetStateSprite(this->uiControlState);
    if (sprite == NULL)
    {
        return;
    }
    
    if (sprite->GetFrameCount() <= value)
    {
        // No way to set this frame.
        return;
    }
    
    GetActiveUIButton()->SetStateFrame(this->uiControlState, value);
    UpdatePropertyDirtyFlagForSpriteFrame();
}

int UIButtonMetadata::GetSpriteFrame()
{
    if (!VerifyActiveParamID())
    {
        return -1;
    }

    return GetSpriteFrameForState(this->uiControlState);
}

void UIButtonMetadata::UpdatePropertyDirtyFlagForSpriteFrame()
{
    int statesCount = UIControlStateHelper::GetUIControlStatesCount();
    for (int i = 0; i < statesCount; i ++)
    {
        UIControl::eControlState curState = UIControlStateHelper::GetUIControlState(i);

        bool curStateDirty = (GetSpriteFrameForState(curState) !=
                              GetSpriteFrameForState(GetReferenceState()));
        SetStateDirtyForProperty(curState, PropertyNames::SPRITE_FRAME_PROPERTY_NAME, curStateDirty);
    }
}

int UIButtonMetadata::GetSpriteFrameForState(UIControl::eControlState state) const
{
    return GetActiveUIButton()->GetStateFrame(state);
}

UIControl::eControlState UIButtonMetadata::GetCurrentStateForLocalizedText() const
{
    // UIButton is state-aware, so return current state.
    return this->uiControlState;
}

QColor UIButtonMetadata::GetColor()
{
    if (!VerifyActiveParamID())
    {
        return -1;
    }
    
    return GetColorForState(this->uiControlState);
}


void UIButtonMetadata::SetColor(const QColor& value)
{
    if (!VerifyActiveParamID())
    {
        return;
    }

    // Yuri Coder, 08/11/2012. According to Dizzer's request update the button background
    // before assigning color - otherwise it might be assigned to the default background.
    GetActiveUIButton()->CreateBackgroundForState(this->uiControlState);

    UIControlBackground* background = GetActiveUIButton()->GetStateBackground(this->uiControlState);
    if (!background)
    {
        return;
    }

    background->color = QTColorToDAVAColor(value);
    UpdatePropertyDirtyFlagForColor();
}

void UIButtonMetadata::UpdatePropertyDirtyFlagForColor()
{
    int statesCount = UIControlStateHelper::GetUIControlStatesCount();
    for (int i = 0; i < statesCount; i ++)
    {
        UIControl::eControlState curState = UIControlStateHelper::GetUIControlState(i);
        
        bool curStateDirty = (GetColorForState(curState) !=
                              GetColorForState(GetReferenceState()));
        SetStateDirtyForProperty(curState, PropertyNames::BACKGROUND_COLOR_PROPERTY_NAME, curStateDirty);
    }
}

QColor UIButtonMetadata::GetColorForState(UIControl::eControlState state) const
{
    UIControlBackground* background = GetActiveUIButton()->GetStateBackground(state);
    if (background)
    {
        return DAVAColorToQTColor(background->color);
    }
    
    return QColor();
}

int UIButtonMetadata::GetDrawType()
{
    if (!VerifyActiveParamID())
    {
        return UIControlBackground::DRAW_ALIGNED;
    }
    
    return GetActiveUIButton()->GetStateDrawType(this->uiControlState);
}

void UIButtonMetadata::SetDrawType(int value)
{
    if (!VerifyActiveParamID())
    {
        return;
    }

    GetActiveUIButton()->SetStateDrawType(this->uiControlState, (UIControlBackground::eDrawType)value);
    UpdatePropertyDirtyFlagForDrawType();
}

void UIButtonMetadata::UpdatePropertyDirtyFlagForDrawType()
{
    int statesCount = UIControlStateHelper::GetUIControlStatesCount();
    for (int i = 0; i < statesCount; i ++)
    {
        UIControl::eControlState curState = UIControlStateHelper::GetUIControlState(i);
        
        bool curStateDirty = (GetActiveUIButton()->GetStateDrawType(curState) !=
                              GetActiveUIButton()->GetStateDrawType(GetReferenceState()));
        SetStateDirtyForProperty(curState, PropertyNames::DRAW_TYPE_PROPERTY_NAME, curStateDirty);
    }
}

int UIButtonMetadata::GetColorInheritType()
{
    if (!VerifyActiveParamID())
    {
        return UIControlBackground::COLOR_MULTIPLY_ON_PARENT;
    }

    return GetColorInheritTypeForState(this->uiControlState);
}

void UIButtonMetadata::SetColorInheritType(int value)
{
    if (!VerifyActiveParamID())
    {
        return;
    }
    
    UIControlBackground* background = GetActiveUIButton()->GetStateBackground(this->uiControlState);
    if (!background)
    {
        return;
    }

    background->SetColorInheritType((UIControlBackground::eColorInheritType)value);
    UpdatePropertyDirtyFlagForColorInheritType();
}

int UIButtonMetadata::GetColorInheritTypeForState(UIControl::eControlState state) const
{
    UIControlBackground* background = GetActiveUIButton()->GetStateBackground(state);
    if (!background)
    {
        return UIControlBackground::COLOR_MULTIPLY_ON_PARENT;
    }
    
    return background->GetColorInheritType();
}

void UIButtonMetadata::UpdatePropertyDirtyFlagForColorInheritType()
{
    int statesCount = UIControlStateHelper::GetUIControlStatesCount();
    for (int i = 0; i < statesCount; i ++)
    {
        UIControl::eControlState curState = UIControlStateHelper::GetUIControlState(i);
        
        bool curStateDirty = (GetColorInheritTypeForState(curState) !=
                              GetColorInheritTypeForState(GetReferenceState()));
        SetStateDirtyForProperty(curState, PropertyNames::COLOR_INHERIT_TYPE_PROPERTY_NAME, curStateDirty);
    }
}

int UIButtonMetadata::GetAlign()
{
    if (!VerifyActiveParamID())
    {
        return ALIGN_TOP;
    }
    
    return GetAlignForState(this->uiControlState);
}

void UIButtonMetadata::SetAlign(int value)
{
    if (!VerifyActiveParamID())
    {
        return;
    }
    
	GetActiveUIButton()->SetStateAlign(this->uiControlState, value);
    UpdatePropertyDirtyFlagForAlign();
}

int UIButtonMetadata::GetAlignForState(UIControl::eControlState state) const
{
	return GetActiveUIButton()->GetStateAlign(state);
}

void UIButtonMetadata::UpdatePropertyDirtyFlagForAlign()
{
    int statesCount = UIControlStateHelper::GetUIControlStatesCount();
    for (int i = 0; i < statesCount; i ++)
    {
        UIControl::eControlState curState = UIControlStateHelper::GetUIControlState(i);
        
        bool curStateDirty = (GetAlignForState(curState) !=
                              GetAlignForState(GetReferenceState()));
        SetStateDirtyForProperty(curState, PropertyNames::ALIGN_PROPERTY_NAME, curStateDirty);
    }
}

// Initialize the control(s) attached.
void UIButtonMetadata::InitializeControl(const String& controlName, const Vector2& position)
{
    BaseMetadata::InitializeControl(controlName, position);

    int paramsCount = this->GetParamsCount();
    for (BaseMetadataParams::METADATAPARAMID i = 0; i < paramsCount; i ++)
    {
        UIButton* button = dynamic_cast<UIButton*>(this->treeNodeParams[i].GetUIControl());

        WideString controlText = StringToWString(button->GetName());
        HierarchyTreeNode* activeNode = GetTreeNode(i);
    
        // Initialize the button for all states.
        int statesCount = UIControlStateHelper::GetUIControlStatesCount();
        for (int stateID = 0; stateID < statesCount; stateID ++)
        {
            UIControl::eControlState state = UIControlStateHelper::GetUIControlState(stateID);
            button->SetStateFont(state, EditorFontManager::Instance()->GetDefaultFont());
            button->SetStateText(state, controlText);

            // Button is state-aware.
            activeNode->GetExtraData().SetLocalizationKey(controlText, state);
        }
        
        // Define some properties for the reference state.
        button->SetStateDrawType(GetReferenceState(), UIControlBackground::DRAW_SCALE_TO_RECT);
    }
}

void UIButtonMetadata::UpdateExtraData(HierarchyTreeNodeExtraData& extraData, eExtraDataUpdateStyle updateStyle)
{
    if (!VerifyActiveParamID())
    {
        return;
    }

    UIButton* button = GetActiveUIButton();

    // Button is state-aware.
    int statesCount = UIControlStateHelper::GetUIControlStatesCount();
    for (int stateID = 0; stateID < statesCount; stateID ++)
    {
        UIControl::eControlState state = UIControlStateHelper::GetUIControlState(stateID);
        UIStaticText* textControl = button->GetStateTextControl(state);
        if (!textControl)
        {
            continue;
        }
        
        UpdateStaticTextExtraData(textControl, state, extraData, updateStyle);
    }
    
    if (updateStyle == UPDATE_EXTRADATA_FROM_CONTROL)
    {
        // Also need to recover Dirty Flags in this case.
        RecoverPropertyDirtyFlags();
    }
}

void UIButtonMetadata::RecoverPropertyDirtyFlags()
{
    UpdatePropertyDirtyFlagForLocalizedText();
    UpdatePropertyDirtyFlagForFont();
    UpdatePropertyDirtyFlagForFontSize();
    UpdatePropertyDirtyFlagForColor();

    UpdatePropertyDirtyFlagForSpriteName();
    UpdatePropertyDirtyFlagForSpriteFrame();
    
    UpdatePropertyDirtyFlagForDrawType();
    UpdatePropertyDirtyFlagForColorInheritType();
    UpdatePropertyDirtyFlagForAlign();
}
