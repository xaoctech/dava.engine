 /*==================================================================================
    Copyright (c) 2008, binaryzebra
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright
    notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
    notice, this list of conditions and the following disclaimer in the
    documentation and/or other materials provided with the distribution.
    * Neither the name of the binaryzebra nor the
    names of its contributors may be used to endorse or promote products
    derived from this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE binaryzebra AND CONTRIBUTORS "AS IS" AND
    ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL binaryzebra BE LIABLE FOR ANY
    DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
    ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
    SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
=====================================================================================*/



#include "UI/UIButton.h"
#include "Base/ObjectFactory.h"
#include "UI/UIYamlLoader.h"
#include "Utils/StringFormat.h"
#include "Utils/Utils.h"
#include "FileSystem/LocalizationSystem.h"
#include "FileSystem/VariantType.h"
#include "Render/2D/FontManager.h"

namespace DAVA 
{
    UIButton::BaseButtonControlData::BaseButtonControlData() :
        isInitialized(false)
    {
    }
    
    bool UIButton::BaseButtonControlData::IsInitialized() const
    {
        return isInitialized;
    }
    
    void UIButton::BaseButtonControlData::Initialize()
    {
        isInitialized = true;
    }

    UIButton::UIStaticTextData::UIStaticTextData() :
        BaseButtonControlData(),
        font(NULL),
        textAlign(ALIGN_HCENTER | ALIGN_VCENTER),
        fittingOption(TextBlock::FITTING_DISABLED)
    {
    }

    UIButton::UIStaticTextData::UIStaticTextData(UIStaticText* staticText)
    {
        DVASSERT(staticText);
        text = staticText->GetText();
        textAlign = staticText->GetTextAlign();
        requestedTextRectSize = staticText->GetTextBlock()->GetRequestedRectSize();
        font = staticText->GetFont();
        fontColor = staticText->GetTextColor();
        shadowColor = staticText->GetShadowColor();
        shadowOffset = staticText->GetShadowOffset();
        fittingOption = staticText->GetFittingOption();
    }

    bool UIButton::UIStaticTextData::operator == (const UIStaticTextData& right) const
    {
        return  (text == right.text) && (textAlign == right.textAlign) &&
            (requestedTextRectSize == right.requestedTextRectSize) &&
            (font == right.font) && (fontColor == right.fontColor) &&
            (shadowColor == right.shadowColor) && (shadowOffset == right.shadowOffset) &&
            (fittingOption == right.fittingOption);
    };

    UIButton::UIControlBackgroundData::UIControlBackgroundData() :
        BaseButtonControlData(),
        spriteFrame(0),
        drawType(UIControlBackground::DRAW_ALIGNED),
        colorInheritType(UIControlBackground::COLOR_MULTIPLY_ON_PARENT),
        align(0),
        modification(0),
        leftRightStretchCap(0.0f),
        topBottomStretchCap(0.0f)
    {
    }

    UIButton::UIControlBackgroundData::UIControlBackgroundData(UIControlBackground* background)
    {
        DVASSERT(background);
        if (background->GetSprite())
        {
            spriteName = background->GetSprite()->GetRelativePathname();
            spriteFrame = background->GetFrame();
        }
        
        color = background->GetColor();
        drawType = background->GetDrawType();
        colorInheritType = background->GetColorInheritType();
        align = background->GetAlign();
        modification = background->GetModification();
        leftRightStretchCap = background->GetLeftRightStretchCap();
        topBottomStretchCap = background->GetTopBottomStretchCap();
        
        Initialize();
    }
    
    bool UIButton::UIControlBackgroundData::operator == (const UIControlBackgroundData& right) const
    {
        return  (spriteName == right.spriteName) && (spriteFrame == right.spriteFrame) &&
            (color == right.color) && (drawType == right.drawType) &&
            (colorInheritType == right.colorInheritType) && (align == right.align) &&
            (modification == right.modification && leftRightStretchCap == right.leftRightStretchCap) &&
            (topBottomStretchCap == right.topBottomStretchCap);
    }

    ////////////////////////////////////////////////////////////////////////////////////////////

    const int32 stateArray[] = {UIControl::STATE_NORMAL, UIControl::STATE_PRESSED_INSIDE, UIControl::STATE_PRESSED_OUTSIDE, UIControl::STATE_DISABLED, UIControl::STATE_SELECTED, UIControl::STATE_HOVER};
    const String statePostfix[] = {"Normal", "PressedInside", "PressedOutside", "Disabled", "Selected", "Hover"};

	UIButton::UIButton(const Rect &rect, bool rectInAbsoluteCoordinates/* = FALSE*/)
	: UIControl(rect, rectInAbsoluteCoordinates)
	{
		oldState = 0;
		for(int i = 0; i < DRAW_STATE_COUNT; i++)
		{
            buttonStates[i].staticText = NULL;
            buttonStates[i].controlBackground = NULL;
		}
        
        buttonStates[DRAW_STATE_UNPRESSED].controlBackground = background;
        buttonStates[DRAW_STATE_UNPRESSED].controlBackgoundData = background;

		selectedBackground = background;
		selectedText = NULL;
    
		exclusiveInput = TRUE;
        SetInputEnabled(true, false);

        UpdateInnerControls();
	}

	UIButton::~UIButton()
	{
		background = buttonStates[DRAW_STATE_UNPRESSED].controlBackground;
        SafeRetain(background); // will be released in the UIControl's destructor.

        Cleanup();
		SafeRelease(selectedText);
	}

	void UIButton::Cleanup()
    {
        for(int i = 0; i < DRAW_STATE_COUNT; i++)
		{
            SafeRelease(buttonStates[i].controlBackground);
            RemoveControl(buttonStates[i].staticText);
            SafeRelease(buttonStates[i].staticText);
		}
    }

	UIButton *UIButton::CloneButton()
	{
		return (UIButton *)Clone();
	}
	
	UIControl *UIButton::Clone()
	{
		UIButton *b = new UIButton(GetRect());
		b->CopyDataFrom(this);
		return b;
	}

	void UIButton::CopyDataFrom(UIControl *srcControl)
	{
        SafeRetain(background); // will be released in the UIControl::CopyDataFrom().
		SafeRelease(selectedText);

        Cleanup();

		UIControl::CopyDataFrom(srcControl);
		UIButton *srcButton = (UIButton *)srcControl;
		for(int32 i = 0; i < DRAW_STATE_COUNT; i++)
		{
            const ButtonStateData& srcStateData = srcButton->buttonStates[i];
			if (srcStateData.controlBackgoundData.IsInitialized())
			{
                buttonStates[i].controlBackgoundData = srcStateData.controlBackgoundData;
			}

            if (srcStateData.staticTextData.IsInitialized())
            {
                buttonStates[i].staticTextData = srcStateData.staticTextData;
            }
		}

		// All the controls creation for the new data will be done in UpdateInnerControls().
        UpdateInnerControls();
		selectedBackground = buttonStates[DRAW_STATE_UNPRESSED].controlBackground;
		oldState = 0;
	}

	void UIButton::SetRect(const Rect &rect, bool rectInAbsoluteCoordinates/* = FALSE*/)
	{
		UIControl::SetRect(rect, rectInAbsoluteCoordinates);
        UpdateStateTextControlSize();
	}

	void UIButton::SetStateSprite(int32 state, const FilePath &spriteName, int32 spriteFrame/* = 0*/)
	{
        bool updateNeeded = false;
		for(int i = 0; i < DRAW_STATE_COUNT; i++)
		{
			if(state & 0x01)
			{
                UIControlBackgroundData& bkgData = CreateBackgroundData((eButtonDrawState)i);
                bkgData.spriteName = spriteName;
                bkgData.spriteFrame = spriteFrame;
                updateNeeded = true;
			}
			state >>= 1;
		}

        if (updateNeeded)
        {
            UpdateInnerControls();
        }
	}
	
	void UIButton::SetStateSprite(int32 state, Sprite *newSprite, int32 spriteFrame/* = 0*/)
	{
        bool updateNeeded = false;
		for(int i = 0; i < DRAW_STATE_COUNT; i++)
		{
			if(state & 0x01)
			{
                UIControlBackgroundData& bkgData = CreateBackgroundData((eButtonDrawState)i);
                bkgData.spriteName = newSprite->GetRelativePathname();
                bkgData.spriteFrame = spriteFrame;
                updateNeeded = true;
			}
			state >>= 1;
		}

        if (updateNeeded)
        {
            UpdateInnerControls();
        }
	}
	
	void UIButton::SetStateFrame(int32 state, int32 spriteFrame)
	{
        bool updateNeeded = false;
		for(int i = 0; i < DRAW_STATE_COUNT; i++)
		{
			if(state & 0x01)
			{
                CreateBackgroundData((eButtonDrawState)i).spriteFrame = spriteFrame;
                updateNeeded = true;
			}
			state >>= 1;
		}

        if (updateNeeded)
        {
            UpdateInnerControls();
        }
	}
	
	void UIButton::SetStateDrawType(int32 state, UIControlBackground::eDrawType drawType)
	{
        bool updateNeeded = false;
		for(int i = 0; i < DRAW_STATE_COUNT; i++)
		{
			if(state & 0x01)
			{
                CreateBackgroundData((eButtonDrawState)i).drawType = drawType;
                updateNeeded = true;
			}
			state >>= 1;
		}

        if (updateNeeded)
        {
            UpdateInnerControls();
        }
	}
	
	void UIButton::SetStateAlign(int32 state, int32 align)
	{
        bool updateNeeded = false;
		for(int i = 0; i < DRAW_STATE_COUNT; i++)
		{
			if(state & 0x01)
			{
                CreateBackgroundData((eButtonDrawState)i).align = align;
                updateNeeded = true;
			}
			state >>= 1;
		}

        if (updateNeeded)
        {
            UpdateInnerControls();
        }
	}

	void UIButton::SetStateModification(int32 state, int32 modification)
	{
        bool updateNeeded = false;
		for(int i = 0; i < DRAW_STATE_COUNT; i++)
		{
			if(state & 0x01)
			{
                CreateBackgroundData((eButtonDrawState)i).modification = modification;
                updateNeeded = true;
			}
			state >>= 1;
		}
		
        if (updateNeeded)
        {
            UpdateInnerControls();
        }
	}
	
	void UIButton::SetStateColor(int32 state, Color color)
	{
        bool updateNeeded = false;
		for(int i = 0; i < DRAW_STATE_COUNT; i++)
		{
			if(state & 0x01)
			{
                CreateBackgroundData((eButtonDrawState)i).color = color;
                updateNeeded = true;
			}
			state >>= 1;
		}

        if (updateNeeded)
        {
            UpdateInnerControls();
        }
	}
	
	void UIButton::SetStateColorInheritType(int32 state, UIControlBackground::eColorInheritType value)
	{
        bool updateNeeded = false;
		for(int i = 0; i < DRAW_STATE_COUNT; i++)
		{
			if(state & 0x01)
			{
                CreateBackgroundData((eButtonDrawState)i).colorInheritType = value;
                updateNeeded = false;
			}
			state >>= 1;
		}

        if (updateNeeded)
        {
            UpdateInnerControls();
        }
	}

	Sprite* UIButton::GetStateSprite(int32 state)
	{
		return GetActualBackground(state)->GetSprite();
	}

	int32 UIButton::GetStateFrame(int32 state)
	{
		return GetActualBackground(state)->GetFrame();
	}

	UIControlBackground::eDrawType UIButton::GetStateDrawType(int32 state)
	{
		return GetActualBackground(state)->GetDrawType();
	}

	int32 UIButton::GetStateAlign(int32 state)
	{
		return GetActualBackground(state)->GetAlign();
	}
	
	UIControlBackground *UIButton::GetStateBackground(int32 state)
	{
		return GetActualBackground(state);
	}
	
	void UIButton::SetStateBackground(int32 state, UIControlBackground *newBackground)
	{
        bool updateNeeded = false;
		for(int i = 0; i < DRAW_STATE_COUNT; i++)
		{
			if(state & 0x01)
			{
                SafeRelease(buttonStates[i].controlBackground);
                buttonStates[i].controlBackgoundData = newBackground;
                buttonStates[i].controlBackground = newBackground->Clone();
                
                updateNeeded = true;
			}
			state >>= 1;
		}

        UpdateInnerControls();
	}

	
	void UIButton::SetStateFont(int32 state, Font *font)
	{
        bool updateNeeded = false;
		for(int i = 0; i < DRAW_STATE_COUNT; i++)
		{
			if(state & 0x01)
			{
                CreateStaticTextData((eButtonDrawState)i).font = font;
                updateNeeded = true;
			}
			state >>= 1;
		}

        if (updateNeeded)
        {
            UpdateInnerControls();
        }
	}
	
    void UIButton::SetStateFontColor(int32 state, const Color& fontColor)
    {
        bool updateNeeded = false;
		for(int i = 0; i < DRAW_STATE_COUNT; i++)
		{
			if(state & 0x01)
			{
                CreateStaticTextData((eButtonDrawState)i).fontColor = fontColor;
                updateNeeded = true;
			}
			state >>= 1;
		}
        
        if (updateNeeded)
        {
            UpdateInnerControls();
        }
    }
	
	void UIButton::SetStateShadowColor(int32 state, const Color& shadowColor)
    {
        bool updateNeeded = false;
		for(int i = 0; i < DRAW_STATE_COUNT; i++)
		{
			if(state & 0x01)
			{
                CreateStaticTextData((eButtonDrawState)i).shadowColor = shadowColor;
                updateNeeded = true;
			}

			state >>= 1;
		}

        if (updateNeeded)
        {
            UpdateInnerControls();
        }
    }

	void UIButton::SetStateShadowOffset(int32 state, const Vector2& offset)
    {
        bool updateNeeded = false;
		for(int i = 0; i < DRAW_STATE_COUNT; i++)
		{
			if(state & 0x01)
			{
                CreateStaticTextData((eButtonDrawState)i).shadowOffset = offset;
                updateNeeded = true;
			}
			state >>= 1;
		}

        if (updateNeeded)
        {
            UpdateInnerControls();
        }
    }

    void UIButton::SetStateFittingOption(int32 state, int32 fittingOption)
    {
        bool updateNeeded = false;
		for(int i = 0; i < DRAW_STATE_COUNT; i++)
		{
			if(state & 0x01)
			{
                CreateStaticTextData((eButtonDrawState)i).fittingOption = fittingOption;
                updateNeeded = true;
			}
			state >>= 1;
		}

        if (updateNeeded)
        {
            UpdateInnerControls();
        }
    }

	void UIButton::SetStateText(int32 state, const WideString &text, const Vector2 &requestedTextRectSize/* = Vector2(0,0)*/)
	{
        bool updateNeeded = false;
		for(int i = 0; i < DRAW_STATE_COUNT; i++)
		{
			if(state & 0x01)
			{
                UIStaticTextData& staticTextData = CreateStaticTextData((eButtonDrawState)i);
                staticTextData.text = text;
                staticTextData.requestedTextRectSize = requestedTextRectSize;
                updateNeeded = true;
			}
			state >>= 1;
		}
        
        if (updateNeeded)
        {
            UpdateInnerControls();
        }
	}

    void UIButton::SetStateTextAlign(int32 state, int32 align)
	{
        bool updateNeeded = false;
		for(int i = 0; i < DRAW_STATE_COUNT; i++)
		{
			if(state & 0x01)
			{
                CreateStaticTextData((eButtonDrawState)i).textAlign = align;
                updateNeeded = true;
			}
			state >>= 1;
		}

        if (updateNeeded)
        {
            UpdateInnerControls();
        }
	}

	void UIButton::SetStateTextControl(int32 state, UIStaticText *textControl)
	{
        bool updateNeeded = false;
		for(int i = 0; i < DRAW_STATE_COUNT; i++)
		{
			if(state & 0x01)
			{
                RemoveControl(buttonStates[i].staticText);
                SafeRelease(buttonStates[i].staticText);

				buttonStates[i].staticText = textControl->CloneStaticText();
                buttonStates[i].staticTextData = buttonStates[i].staticText;
                updateNeeded = true;
			}

			state >>= 1;
		}

        if (updateNeeded)
        {
            UpdateInnerControls();
        }
	}

	UIStaticText *UIButton::GetStateTextControl(int32 state)
	{
		return GetActualText(state);
	}
	
	void UIButton::SystemUpdate(float32 timeElapsed)
	{
		UIControl::SystemUpdate(timeElapsed);

		if(oldState != controlState)
		{
            RemoveSelectedText();
            SafeRelease(selectedText);

			selectedText = SafeRetain(GetActualText(controlState));
            RestoreSelectedText();

			selectedBackground = GetActualBackground(controlState);
			oldState = controlState;
		}
	}
	
	void UIButton::SetVisible(bool isVisible, bool hierarchic)
	{
        RemoveSelectedText();

		UIControl::SetVisible(isVisible, hierarchic);
		for(int i = 0; i < DRAW_STATE_COUNT; i++)
		{
            if (buttonStates[i].staticText)
			{
				buttonStates[i].staticText->SetVisible(isVisible);
			}
		}

        RestoreSelectedText();
    }

	void UIButton::SetInputEnabled(bool isEnabled, bool hierarchic)
	{
        RemoveSelectedText();
		UIControl::SetInputEnabled(isEnabled, hierarchic);
        RestoreSelectedText();
	}

	void UIButton::SetDisabled(bool isDisabled, bool hierarchic)
	{
        RemoveSelectedText();
		UIControl::SetDisabled(isDisabled, hierarchic);
        RestoreSelectedText();
	}

	void UIButton::SetSelected(bool isSelected, bool hierarchic)
	{
        RemoveSelectedText();
		UIControl::SetSelected(isSelected, hierarchic);
        RestoreSelectedText();
	}

	void UIButton::SetExclusiveInput(bool isExclusiveInput, bool hierarchic)
	{
        RemoveSelectedText();
		UIControl::SetExclusiveInput(isExclusiveInput, hierarchic);
        RestoreSelectedText();
	}

	void UIButton::SetMultiInput(bool isMultiInput, bool hierarchic)
	{
        RemoveSelectedText();
		UIControl::SetMultiInput(isMultiInput, hierarchic);
        RestoreSelectedText();
	}

	void UIButton::Input(UIEvent *currentInput)
	{
		UIControl::Input(currentInput);
		currentInput->SetInputHandledType(UIEvent::INPUT_HANDLED_SOFT); // Drag is not handled - see please DF-2508.
	}

	void UIButton::SetBackground(UIControlBackground *newBg)
	{
		DVASSERT(false);
	}
	
	UIControlBackground * UIButton::GetBackground()
	{
		return selectedBackground;
	}

	void UIButton::SetLeftAlign(int32 align)
	{
		UIControl::SetLeftAlign(align);	
		UpdateStateTextControlSize();
	}

	void UIButton::SetHCenterAlign(int32 align)
	{
		UIControl::SetHCenterAlign(align);
		UpdateStateTextControlSize();
	}
	
	void UIButton::SetRightAlign(int32 align)
	{
		UIControl::SetRightAlign(align);
		UpdateStateTextControlSize();
	}
	
	void UIButton::SetTopAlign(int32 align)
	{
		UIControl::SetTopAlign(align);
		UpdateStateTextControlSize();
	}

	void UIButton::SetVCenterAlign(int32 align)
	{
		UIControl::SetVCenterAlign(align);
		UpdateStateTextControlSize();
	}
	
	void UIButton::SetBottomAlign(int32 align)
	{
		UIControl::SetBottomAlign(align);
		UpdateStateTextControlSize();
	}
	
	void UIButton::SystemDraw(const UIGeometricData &geometricData)
	{
		background = selectedBackground;
		UIControl::SystemDraw(geometricData);
		background = buttonStates[DRAW_STATE_UNPRESSED].controlBackground;
	}
	
	UIControlBackground *UIButton::GetActualBackground(int32 state)
	{
        return buttonStates[BackgroundIndexForState(GetDrawStateForControlState(state))].controlBackground;
	}

	UIStaticText *UIButton::GetActualText(int32 state)
	{
		return buttonStates[TextIndexForState(GetDrawStateForControlState(state))].staticText;
	}

	UIButton::UIControlBackgroundData& UIButton::CreateBackgroundData(eButtonDrawState state)
	{
		if (buttonStates[state].controlBackgoundData.IsInitialized())
		{
			return buttonStates[state].controlBackgoundData;
		}

        // Background Data has to be cloned from the target.
		UIControlBackgroundData& targetBkgData = buttonStates[BackgroundIndexForState(state)].controlBackgoundData;
		if (targetBkgData.IsInitialized())
		{
            buttonStates[state].controlBackgoundData = targetBkgData;
        }

        buttonStates[state].controlBackgoundData.Initialize();;
		return buttonStates[state].controlBackgoundData;
	}

    UIButton::UIStaticTextData& UIButton::CreateStaticTextData(eButtonDrawState state)
    {
		if (buttonStates[state].staticTextData.IsInitialized())
		{
			return buttonStates[state].staticTextData;
		}
        
        UIStaticTextData& targetTextData = buttonStates[TextIndexForState(state)].staticTextData;
        if (targetTextData.IsInitialized())
        {
            buttonStates[state].staticTextData = targetTextData;
        }

        buttonStates[state].staticTextData.Initialize();
        return buttonStates[state].staticTextData;
    }

	UIButton::eButtonDrawState UIButton::GetDrawStateForControlState(int32 state)
	{
		if(state & UIControl::STATE_DISABLED)
		{
			return DRAW_STATE_DISABLED;
		}
		else if(state & UIControl::STATE_SELECTED)
		{
			return DRAW_STATE_SELECTED;
		}
		else if(state & UIControl::STATE_PRESSED_INSIDE)
		{
			return DRAW_STATE_PRESSED_INSIDE;
		}
		else if(state & UIControl::STATE_PRESSED_OUTSIDE)
		{
			return DRAW_STATE_PRESSED_OUTSIDE;
		}
		else if(state & UIControl::STATE_HOVER)
		{
			return DRAW_STATE_HOVERED;
		}
		
		return DRAW_STATE_UNPRESSED;
	}

	int32 UIButton::BackgroundIndexForState(eButtonDrawState buttonState)
	{
		if (buttonStates[buttonState].controlBackground)
		{
            //return current state if dada for state is present
			return buttonState;
		}
        
		switch (buttonState) 
		{
            //find other state if data for the requested state is absent
			case DRAW_STATE_PRESSED_INSIDE:
			{
				if(buttonStates[DRAW_STATE_PRESSED_OUTSIDE].controlBackground)
				{
					return DRAW_STATE_PRESSED_OUTSIDE;
				}
				return DRAW_STATE_UNPRESSED;
			}

			case DRAW_STATE_PRESSED_OUTSIDE:
			{
				return DRAW_STATE_UNPRESSED;
			}

			case DRAW_STATE_SELECTED:
			{
				if(buttonStates[DRAW_STATE_PRESSED_INSIDE].controlBackground)
				{
					return DRAW_STATE_PRESSED_INSIDE;
				}
				if(buttonStates[DRAW_STATE_PRESSED_OUTSIDE].controlBackground)
				{
					return DRAW_STATE_PRESSED_OUTSIDE;
				}
				return DRAW_STATE_UNPRESSED;
			}

			case DRAW_STATE_DISABLED:
			{
				return DRAW_STATE_UNPRESSED;
			}

			case DRAW_STATE_HOVERED:
			{
				return DRAW_STATE_UNPRESSED;
			}

            default:
                break;
		}

		return DRAW_STATE_UNPRESSED;
	}

	int32 UIButton::TextIndexForState(eButtonDrawState buttonState)
	{
		if(buttonStates[buttonState].staticText)
		{
			return buttonState;
		}

		switch (buttonState) 
		{
            //find other state if data for the requested state is absent
			case DRAW_STATE_PRESSED_INSIDE:
			{
				if(buttonStates[DRAW_STATE_PRESSED_OUTSIDE].staticText)
				{
					return DRAW_STATE_PRESSED_OUTSIDE;
				}

				return DRAW_STATE_UNPRESSED;
			}

			case DRAW_STATE_PRESSED_OUTSIDE:
			{
				return DRAW_STATE_UNPRESSED;
			}

			case DRAW_STATE_SELECTED:
			{
				if(buttonStates[DRAW_STATE_PRESSED_INSIDE].staticText)
				{
					return DRAW_STATE_PRESSED_INSIDE;
				}
				if(buttonStates[DRAW_STATE_PRESSED_OUTSIDE].staticText)
				{
					return DRAW_STATE_PRESSED_OUTSIDE;
				}
				return DRAW_STATE_UNPRESSED;
			}

			case DRAW_STATE_DISABLED:
			{
				return DRAW_STATE_UNPRESSED;
			}

			case DRAW_STATE_HOVERED:
			{
				return DRAW_STATE_UNPRESSED;
			}

            default:
                break;
		}

		return DRAW_STATE_UNPRESSED;
	}
	
	void UIButton::LoadFromYamlNode(const YamlNode * node, UIYamlLoader * loader)
	{
		UIControl::LoadFromYamlNode(node, loader);

		for (int k = 0; k < STATE_COUNT; ++k)
		{
			const YamlNode * stateSpriteNode = node->Get(Format("stateSprite%s", statePostfix[k].c_str()));
            bool needInitializeBkgState = false;
            UIControlBackgroundData& bkgData = buttonStates[k].controlBackgoundData;
			if (stateSpriteNode)
			{
				const YamlNode * spriteNode = stateSpriteNode->Get(0);
				const YamlNode * frameNode = stateSpriteNode->Get(1);
				const YamlNode * backgroundModificationNode = NULL;
				if(stateSpriteNode->GetCount() > 2)
				{
					backgroundModificationNode = stateSpriteNode->Get(2);
				}
				
				if (frameNode)
                {
                    bkgData.spriteFrame = frameNode->AsInt();
                    needInitializeBkgState = true;
                }
				if (spriteNode)
				{
                    bkgData.spriteName = spriteNode->AsString();
                    needInitializeBkgState = true;
				}
				if (backgroundModificationNode)
				{
                    bkgData.modification = backgroundModificationNode->AsInt();
                    needInitializeBkgState = true;
				}
			}
            
            const YamlNode * stateDrawTypeNode = node->Get(Format("stateDrawType%s", statePostfix[k].c_str()));
			if (stateDrawTypeNode)
			{
                bkgData.drawType = (UIControlBackground::eDrawType)loader->GetDrawTypeFromNode(stateDrawTypeNode);
                needInitializeBkgState = true;
                
                const YamlNode * leftRightStretchCapNode = node->Get(Format("leftRightStretchCap%s", statePostfix[k].c_str()));
                const YamlNode * topBottomStretchCapNode = node->Get(Format("topBottomStretchCap%s", statePostfix[k].c_str()));

                if(leftRightStretchCapNode)
                {
                    bkgData.leftRightStretchCap = leftRightStretchCapNode->AsFloat();
                }
                
                if(topBottomStretchCapNode)
                {
                    bkgData.topBottomStretchCap = topBottomStretchCapNode->AsFloat();
                }
			}
            
            const YamlNode * stateAlignNode = node->Get(Format("stateAlign%s", statePostfix[k].c_str()));
			if (stateAlignNode)
			{
				bkgData.align = loader->GetAlignFromYamlNode(stateAlignNode);
                needInitializeBkgState = true;
			}

            const YamlNode * colorInheritNode = node->Get(Format("stateColorInherit%s", statePostfix[k].c_str()));
			if(colorInheritNode)
			{
                bkgData.colorInheritType =(UIControlBackground::eColorInheritType)loader->GetColorInheritTypeFromNode(colorInheritNode);
                needInitializeBkgState = true;
			}
			
			const YamlNode * colorNode = node->Get(Format("stateColor%s", statePostfix[k].c_str()));
			if(colorNode)
			{
				bkgData.color = loader->GetColorFromYamlNode(colorNode);
                needInitializeBkgState = true;
			}

            if (needInitializeBkgState)
            {
                bkgData.Initialize();
            }

            // Background loading is complete, starting text loading.
            UIStaticTextData& textData = buttonStates[k].staticTextData;
            bool needInitializeTextState = false;

			const YamlNode * stateFontNode = node->Get(Format("stateFont%s", statePostfix[k].c_str()));
			if (stateFontNode)
			{
                textData.font = loader->GetFontByName(stateFontNode->AsString());
                needInitializeTextState = true;
			}
			
			const YamlNode * stateTextNode = node->Get(Format("stateText%s", statePostfix[k].c_str()));
			if (stateTextNode)
			{
                textData.text = LocalizedString(stateTextNode->AsWString());
                needInitializeTextState = true;
			}

            const YamlNode * stateTextAlignNode = node->Get(Format("stateTextAlign%s", statePostfix[k].c_str()));
            if (stateTextAlignNode)
            {
                textData.textAlign = loader->GetAlignFromYamlNode(stateTextAlignNode);
                needInitializeTextState = true;
			}

			const YamlNode * stateFontColorNode = node->Get(Format("stateTextcolor%s", statePostfix[k].c_str()));
			if (stateFontColorNode)
			{
                textData.fontColor = stateFontColorNode->AsColor();
                needInitializeTextState = true;
			}
            
			const YamlNode * stateShadowColorNode = node->Get(Format("stateShadowcolor%s", statePostfix[k].c_str()));
			if (stateShadowColorNode)
			{
                textData.shadowColor = stateShadowColorNode->AsColor();
                needInitializeTextState = true;
			}
			
			const YamlNode * stateShadowOffsetNode = node->Get(Format("stateShadowoffset%s", statePostfix[k].c_str()));
			if (stateShadowOffsetNode)
			{
                textData.shadowOffset = stateShadowOffsetNode->AsVector2();
                needInitializeTextState = true;
			}

            const YamlNode * stateFittingOptionNode = node->Get(Format("stateFittingOption%s", statePostfix[k].c_str()));
			if (stateFittingOptionNode)
			{
                needInitializeTextState = true;
                textData.fittingOption = stateFittingOptionNode->AsInt32();
			}
            
            // Text loading is complete.
            if (needInitializeTextState)
            {
                textData.Initialize();
            }
		}

        // Create the appropriate controls for unique states.
        UpdateInnerControls();
	}

	YamlNode * UIButton::SaveToYamlNode(UIYamlLoader * loader)
	{
		YamlNode *node = UIControl::SaveToYamlNode(loader);

		//Control Type
		SetPreferredNodeType(node, "UIButton");
        
		//Remove values of UIControl
		//UIButton has state specific properties
		const YamlNode *colorNode = node->Get("color");
		const YamlNode *spriteNode = node->Get("sprite");
		const YamlNode *drawTypeNode = node->Get("drawType");
		const YamlNode *colorInheritNode = node->Get("colorInherit");
		const YamlNode *alignNode = node->Get("align");
		const YamlNode *leftRightStretchCapNode = node->Get("leftRightStretchCap");
		const YamlNode *topBottomStretchCapNode = node->Get("topBottomStretchCap");
		const YamlNode *spriteModificationNode = node->Get("spriteModification");
        
		if (colorNode)
		{
			node->RemoveNodeFromMap("color");
		}
		if (spriteNode)
		{
			node->RemoveNodeFromMap("sprite");
		}
		if (drawTypeNode)
		{
			node->RemoveNodeFromMap("drawType");
		}
		if (colorInheritNode)
		{
			node->RemoveNodeFromMap("colorInherit");
		}
		if (alignNode)
		{
			node->RemoveNodeFromMap("align");
		}
		if (leftRightStretchCapNode)
		{
			node->RemoveNodeFromMap("leftRightStretchCap");
		}
		if (topBottomStretchCapNode)
		{
			node->RemoveNodeFromMap("topBottomStretchCap");
		}
		if (spriteModificationNode)
		{
			node->RemoveNodeFromMap("spriteModification");
		}

        // Build the distinct list of states and save them only.
        Map<int32, UIStaticTextData> distinctTexts;
        Map<int32, UIControlBackgroundData> distinctBackgrounds;
        for(int32 i = 0; i < DRAW_STATE_COUNT; i++)
		{
            if (buttonStates[i].staticTextData.IsInitialized())
            {
                bool staticTextFound = false;
                for (Map<int, UIStaticTextData>::iterator iter = distinctTexts.begin();
                     iter != distinctTexts.end(); iter ++)
                {
                    if (iter->second == buttonStates[i].staticTextData)
                    {
                        staticTextFound = true;
                        break;
                    }
                }
                
                if (!staticTextFound)
                {
                    distinctTexts[i] = buttonStates[i].staticTextData;
                }
            }
            
            // The same is for background.
            if (buttonStates[i].controlBackgoundData.IsInitialized())
            {
                bool backgroundFound = false;
                for (Map<int, UIControlBackgroundData>::iterator iter = distinctBackgrounds.begin();
                     iter != distinctBackgrounds.end(); iter ++)
                {
                    if (iter->second == buttonStates[i].controlBackgoundData)
                    {
                        backgroundFound = true;
                        break;
                    }
                }
                
                if (!backgroundFound)
                {
                    distinctBackgrounds[i] = buttonStates[i].controlBackgoundData;
                }
            }
		}

        SaveTexts(loader, node, distinctTexts);
        SaveBackgrounds(loader, node, distinctBackgrounds);

		return node;
	}

	List<UIControl* >& UIButton::GetRealChildren()
	{
		List<UIControl* >& realChildren = UIControl::GetRealChildren();
		for (uint32 i = 0; i < DRAW_STATE_COUNT; ++i)
		{
			realChildren.remove(buttonStates[i].staticText);
		}

		return realChildren;
	}
	
	void UIButton::UpdateStateTextControlSize()
	{
        const Rect rect = this->GetRect();
        for(int i = 0; i < DRAW_STATE_COUNT; i++)
		{
            if (buttonStates[i].staticText)
			{
                buttonStates[i].staticText->SetRect(Rect(0, 0, rect.dx, rect.dy));
			}
		}
	}
    
    void UIButton::SetVisibleForUIEditor(bool value, bool hierarchic/* = true*/)
	{
        UIControl::SetVisibleForUIEditor(value, hierarchic);
        for(int i = 0; i < DRAW_STATE_COUNT; i++)
		{
            if (buttonStates[i].staticText)
            {
                buttonStates[i].staticText->SetVisibleForUIEditor(value, hierarchic);
            }
		}
    }

    void UIButton::UpdateInnerControls()
    {
        for(int32 i = 0; i < DRAW_STATE_COUNT; i++)
		{
            UpdateBackgroundControl(i);
            UpdateStaticTextControl(i);
		}
        
        selectedBackground = GetActualBackground(controlState);
        oldState = 0;
    }
    
    void UIButton::UpdateBackgroundControl(int32 state)
    {
        const UIControlBackgroundData& curStateData = buttonStates[state].controlBackgoundData;
        if (!curStateData.IsInitialized())
        {
            // Nothing to update for this state.
            return;
        }
        
        // Verify whether this state was already used before.
        int32 duplicateIndex = -1;
        bool backgroundAlreadyUsed = false;

        for (int32 i = 0; i < state; i ++)
        {
            if (duplicateIndex == -1 && buttonStates[i].controlBackgoundData.IsInitialized() && buttonStates[i].controlBackgoundData == curStateData)
            {
                duplicateIndex = i;
            }
            
            if (buttonStates[i].controlBackground == buttonStates[state].controlBackground)
            {
                backgroundAlreadyUsed = true;
            }
        }

        SafeRelease(buttonStates[state].controlBackground);
        if (duplicateIndex >= 0)
        {
            // There is a duplicated state for this one - reuse its background.
            SafeRelease(buttonStates[state].controlBackground);
            buttonStates[state].controlBackground = SafeRetain(buttonStates[duplicateIndex].controlBackground);
        }
        else
        {
            // There is no duplicated data for this state - need to update exising one.
            if (backgroundAlreadyUsed)
            {
                SafeRelease(buttonStates[state].controlBackground);
            }

            UpdateBackground(state, curStateData);
        }
    }
    
    void UIButton::UpdateStaticTextControl(int32 state)
    {
        UIStaticTextData& curStateData = buttonStates[state].staticTextData;
        if (!curStateData.IsInitialized())
        {
            // Nothing to update for this state.
            return;
        }
        
        // Verify whether this state was already used before.
        int32 duplicateIndex = -1;
        bool staticTextAlreadyUsed = false;
        for (int32 i = 0; i < state; i ++)
        {
            if (duplicateIndex == -1 && buttonStates[i].staticTextData.IsInitialized() && buttonStates[i].staticTextData == curStateData)
            {
                duplicateIndex = i;
            }
            
            if (buttonStates[i].staticText == buttonStates[state].staticText)
            {
                staticTextAlreadyUsed = true;
            }
        }

        if (duplicateIndex >= 0)
        {
            // There is a duplicated state for this one - reuse its background.
            SafeRelease(buttonStates[state].staticText);
            buttonStates[state].staticText = SafeRetain(buttonStates[duplicateIndex].staticText);
            SafeRetain(buttonStates[state].staticTextData.font);
        }
        else
        {
            // There is no duplicated data for this state - need to create/update existing one.
            if (staticTextAlreadyUsed)
            {
                // The pointer to this static text is already used in some other state, but
                // its staticTextData is unique. Have to recreate the static text control.
                SafeRelease(buttonStates[state].staticText);
            }

            UpdateStaticText(state, curStateData);
        }
    }

    void UIButton::UpdateBackground(int32 state, const UIControlBackgroundData& stateData)
    {
        if (!buttonStates[state].controlBackground)
        {
            buttonStates[state].controlBackground = new UIControlBackground();
        }

        UIControlBackground* bkg = buttonStates[state].controlBackground;
        if (stateData.spriteName.IsEmpty())
        {
            bkg->SetSprite(NULL, 0);
        }
        else
        {
            bkg->SetSprite(stateData.spriteName, stateData.spriteFrame);
        }

        bkg->SetColor(stateData.color);
        bkg->SetDrawType(stateData.drawType);
        bkg->SetColorInheritType(stateData.colorInheritType);
        bkg->SetAlign(stateData.align);
        bkg->SetModification(stateData.modification);
        bkg->SetLeftRightStretchCap(stateData.leftRightStretchCap);
        bkg->SetTopBottomStretchCap(stateData.topBottomStretchCap);
    }

    void UIButton::UpdateStaticText(int32 state, const UIStaticTextData& stateData)
    {
        if (!buttonStates[state].staticText)
        {
            buttonStates[state].staticText = new UIStaticText();
        }
    
        Rect rect = GetRect();
        UIStaticText* staticText = buttonStates[state].staticText;
        staticText->SetRect(Rect(0, 0, rect.dx, rect.dy));
        staticText->SetVisible(GetVisible());
        staticText->SetVisibleForUIEditor(GetVisibleForUIEditor());

        staticText->SetFont(stateData.font);
        staticText->SetText(stateData.text, stateData.requestedTextRectSize);
        staticText->SetTextColor(stateData.fontColor);
        staticText->SetTextAlign(stateData.textAlign);
        staticText->SetShadowColor(stateData.shadowColor);
        staticText->SetShadowOffset(stateData.shadowOffset);
        staticText->SetFittingOption(stateData.fittingOption);
    }

    void UIButton::RemoveSelectedText()
    {
        if(selectedText)
        {
            RemoveControl(selectedText);
        }
    }
    
    void UIButton::RestoreSelectedText()
    {
		if(selectedText)
		{
			AddControl(selectedText);
            BringChildBack(selectedText);
		}
	}
    
    void UIButton::SaveTexts(UIYamlLoader * loader, YamlNode *node, const Map<int, UIStaticTextData>& texts)
    {
        UIStaticText* base = new UIStaticText();
        VariantType* nodeValue = new VariantType();

        for (Map<int32, UIStaticTextData>::const_iterator iter = texts.begin(); iter != texts.end(); iter ++)
        {
            int32 state = iter->first;
            const UIStaticTextData& data = iter->second;

            if (base->GetText() != data.text)
            {
                node->Set(Format("stateText%s", statePostfix[state].c_str()), data.text);
            }
            if (base->GetTextAlign() != data.textAlign)
            {
                node->SetNodeToMap(Format("stateTextAlign%s", statePostfix[state].c_str()), loader->GetAlignNodeValue(data.textAlign));
            }

            if (base->GetFont() != data.font)
            {
                node->Set(Format("stateFont%s", statePostfix[state].c_str()), FontManager::Instance()->GetFontName(data.font));
            }
            if (base->GetTextColor() != data.fontColor)
            {
                nodeValue->SetColor(data.fontColor);
                node->Set(Format("stateTextcolor%s", statePostfix[state].c_str()), nodeValue);
            }
            if (base->GetShadowColor() != data.shadowColor)
            {
                nodeValue->SetColor(data.shadowColor);
                node->Set(Format("stateShadowcolor%s", statePostfix[state].c_str()), nodeValue);
            }
            if (base->GetShadowOffset() != data.shadowOffset)
            {
                nodeValue->SetVector2(data.shadowOffset);
                node->Set(Format("stateShadowoffset%s", statePostfix[state].c_str()), nodeValue);
            }
            if (base->GetFittingOption() != data.fittingOption)
            {
                nodeValue->SetInt32(data.fittingOption);
                node->Set(Format("stateFittingOption%s", statePostfix[state].c_str()), nodeValue);
            }
        }

        SafeRelease(base);
        SafeDelete(nodeValue);
    }

    void UIButton::SaveBackgrounds(UIYamlLoader* loader, YamlNode *node, const Map<int, UIControlBackgroundData>& backgrounds)
    {
        UIControlBackground* base = new UIControlBackground();
        VariantType* nodeValue = new VariantType();
        
        for (Map<int32, UIControlBackgroundData>::const_iterator iter = backgrounds.begin(); iter != backgrounds.end(); iter ++)
        {
            int32 state = iter->first;
            const UIControlBackgroundData& data = iter->second;

            if (!data.spriteName.IsEmpty())
            {
                YamlNode *spriteNode = new YamlNode(YamlNode::TYPE_ARRAY);
                
                spriteNode->AddValueToArray(GetSpriteFrameworkPath(data.spriteName));
                spriteNode->AddValueToArray(data.spriteFrame);
                spriteNode->AddValueToArray(data.modification);
                node->AddNodeToMap(Format("stateSprite%s", statePostfix[state].c_str()), spriteNode);
            }
            if (base->GetColor() != data.color)
            {
                nodeValue->SetColor(data.color);
                node->Set(Format("stateColor%s", statePostfix[state].c_str()), nodeValue);
            }
            if (base->GetDrawType() != data.drawType)
            {
                node->Set(Format("stateDrawType%s", statePostfix[state].c_str()), loader->GetDrawTypeNodeValue(data.drawType));
            }
            if (base->GetColorInheritType() != data.colorInheritType)
            {
                node->Set(Format("stateColorInherit%s", statePostfix[state].c_str()), loader->GetColorInheritTypeNodeValue(data.colorInheritType));
            }
            if (base->GetAlign() != data.align)
            {
                node->AddNodeToMap(Format("stateAlign%s", statePostfix[state].c_str()), loader->GetAlignNodeValue(data.align));
            }
            
            if (base->GetLeftRightStretchCap() != data.leftRightStretchCap)
            {
                node->Set(Format("leftRightStretchCap%s", statePostfix[state].c_str()), data.leftRightStretchCap);
            }
            if (base->GetTopBottomStretchCap() != data.topBottomStretchCap)
            {
                node->Set(Format("topBottomStretchCap%s", statePostfix[state].c_str()), data.topBottomStretchCap);
            }
        }

        SafeRelease(base);
        SafeDelete(nodeValue);
    }
};