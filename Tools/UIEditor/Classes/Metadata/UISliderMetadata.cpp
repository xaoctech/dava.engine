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



#include "UISliderMetadata.h"
#include "StringUtils.h"
#include "StringConstants.h"
#include "ColorHelper.h"

using namespace DAVA;

UISliderMetadata::UISliderMetadata(QObject* parent) :
    UIControlMetadata(parent)
{
}

UISlider* UISliderMetadata::GetActiveUISlider() const
{
    return static_cast<UISlider*>(GetActiveUIControl());
}

// Initialize the control(s) attached.
void UISliderMetadata::InitializeControl(const String& controlName, const Vector2& position)
{
    UIControlMetadata::InitializeControl(controlName, position);

    int paramsCount = this->GetParamsCount();
    for (BaseMetadataParams::METADATAPARAMID i = 0; i < paramsCount; i ++)
    {
		// Initialize UISlider with default parameters
        UISlider* slider = static_cast<UISlider*>(this->treeNodeParams[i].GetUIControl());
        slider->SetMinMaxValue(0.0f, 100.0f);
    }
}

float UISliderMetadata::GetSliderValue() const
{
    if (!VerifyActiveParamID())
    {
        return -1.0f;
    }
    
	return GetActiveUISlider()->GetValue();
}

void UISliderMetadata::SetSliderValue(float value)
{
    if (!VerifyActiveParamID())
    {
        return;
    }

	GetActiveUISlider()->SetValue(value);
}

float UISliderMetadata::GetSliderMinValue() const
{
    if (!VerifyActiveParamID())
    {
        return -1.0f;
    }

	return GetActiveUISlider()->GetMinValue();
}

void UISliderMetadata::SetSliderMinValue(float value)
{
    if (!VerifyActiveParamID())
    {
        return;
    }

	GetActiveUISlider()->SetMinValue(value);
}

float UISliderMetadata::GetSliderMaxValue() const
{
    if (!VerifyActiveParamID())
    {
        return -1.0f;
    }
    
	return GetActiveUISlider()->GetMaxValue();
}

void UISliderMetadata::SetSliderMaxValue(float value)
{
    if (!VerifyActiveParamID())
    {
        return;
    }

	GetActiveUISlider()->SetMaxValue(value);
}

void UISliderMetadata::ApplyResize(const Rect& /*originalRect*/, const Rect& newRect)
{
    if (!VerifyActiveParamID())
    {
        return;
    }	
	
    GetActiveUISlider()->SetRect(newRect);
}

QColor UISliderMetadata::GetMinColor() const
{
    if (!VerifyActiveParamID())
    {
        return UIControlBackground::DRAW_ALIGNED;
    }
    
    return ColorHelper::DAVAColorToQTColor(GetActiveUISlider()->GetBgMin()->GetColor());
}

void UISliderMetadata::SetMinColor(const QColor& value)
{
    if (!VerifyActiveParamID())
    {
        return;
    }

    GetActiveUISlider()->GetBgMin()->SetColor(ColorHelper::QTColorToDAVAColor(value));
}

int UISliderMetadata::GetMinDrawType() const
{
    if (!VerifyActiveParamID())
    {
        return UIControlBackground::DRAW_ALIGNED;
    }
    
    return GetActiveUISlider()->GetBgMin()->GetDrawType();
}

void UISliderMetadata::SetMinDrawType(int value)
{
    if (!VerifyActiveParamID())
    {
        return;
    }
    
    GetActiveUISlider()->SetMinDrawType((UIControlBackground::eDrawType)value);
}

int UISliderMetadata::GetMinColorInheritType() const
{
    if (!VerifyActiveParamID())
    {
        return UIControlBackground::COLOR_MULTIPLY_ON_PARENT;
    }
    
    return (int)GetActiveUISlider()->GetBgMin()->GetColorInheritType();
}

void UISliderMetadata::SetMinColorInheritType(int value)
{
    if (!VerifyActiveParamID())
    {
        return;
    }
    
    GetActiveUISlider()->SetMinColorInheritType((UIControlBackground::eColorInheritType)value);
}

int UISliderMetadata::GetMinAlign() const
{
    if (!VerifyActiveParamID())
    {
        return ALIGN_LEFT;
    }
    
    return (int)GetActiveUISlider()->GetBgMin()->GetAlign();
}

void UISliderMetadata::SetMinAlign(int value)
{
    if (!VerifyActiveParamID())
    {
        return;
    }

    GetActiveUISlider()->SetMinAlign(value);
}

float UISliderMetadata::GetMinLeftRightStretchCap() const
{
    if (!VerifyActiveParamID())
    {
        return -1.0;
    }

    return GetActiveUISlider()->GetBgMin()->GetLeftRightStretchCap();
}

void UISliderMetadata::SetMinLeftRightStretchCap(float value)
{
    if (!VerifyActiveParamID())
    {
        return;
    }
    
    GetActiveUISlider()->SetMinLeftRightStretchCap(value);
}

float UISliderMetadata::GetMinTopBottomStretchCap() const
{
    if (!VerifyActiveParamID())
    {
        return -1.0;
    }

    return GetActiveUISlider()->GetBgMin()->GetTopBottomStretchCap();
}

void UISliderMetadata::SetMinTopBottomStretchCap(float value)
{
    if (!VerifyActiveParamID())
    {
        return;
    }
    
    GetActiveUISlider()->SetMinTopBottomStretchCap(value);
}

void UISliderMetadata::SetMinSprite(const QString& value)
{
    if (!VerifyActiveParamID())
    {
        return;
    }

    //If empty string value is used - remove sprite
    if (value.isEmpty())
    {
        GetActiveUISlider()->SetMinSprite(NULL, 0);
    }
    else
    {
        Sprite* sprite = Sprite::Create(value.toStdString());
        if (sprite)
        {
            GetActiveUISlider()->SetMinSprite(sprite, 0);
            SafeRelease(sprite);
        }
    }
}

QString UISliderMetadata::GetMinSprite() const
{
    if (!VerifyActiveParamID())
    {
        return QString();
    }
    
    Sprite* sprite = GetActiveUISlider()->GetBgMin()->GetSprite();
    if (sprite)
    {
        return QString::fromStdString(sprite->GetRelativePathname().GetAbsolutePathname());
    }
    
    return StringConstants::NO_SPRITE_IS_SET;
}

void UISliderMetadata::SetMinSpriteFrame(int value)
{
    if (!VerifyActiveParamID())
    {
        return;
    }
    
    Sprite* sprite = GetActiveUISlider()->GetBgMin()->GetSprite();
    if (sprite && sprite->GetFrameCount() > value)
    {
        GetActiveUISlider()->SetMinSpriteFrame(value);
    }
}

int UISliderMetadata::GetMinSpriteFrame() const
{
    if (!VerifyActiveParamID())
    {
        return -1;
    }
    
    if (GetActiveUISlider()->GetBgMin()->GetSprite())
    {
        return GetActiveUISlider()->GetBgMin()->GetFrame();
    }
    
    return 0;
}

void UISliderMetadata::SetMinSpriteModification(int value)
{
    if (!VerifyActiveParamID())
    {
        return;
    }

    Sprite* sprite = GetActiveUISlider()->GetBgMin()->GetSprite();
    if (sprite)
    {
        GetActiveUISlider()->SetMinSpriteModification(value);
    }
}

int UISliderMetadata::GetMinSpriteModification() const
{
    if (!VerifyActiveParamID())
    {
        return -1;
    }

    Sprite* sprite = GetActiveUIControl()->GetBackground()->GetSprite();
    if (sprite)
    {
        return GetActiveUISlider()->GetBgMax()->GetModification();
    }

    return 0;
}

// Max Background.

QColor UISliderMetadata::GetMaxColor() const
{
    if (!VerifyActiveParamID())
    {
        return UIControlBackground::DRAW_ALIGNED;
    }

    return ColorHelper::DAVAColorToQTColor(GetActiveUISlider()->GetBgMax()->GetColor());
}

void UISliderMetadata::SetMaxColor(const QColor& value)
{
    if (!VerifyActiveParamID())
    {
        return;
    }

    GetActiveUISlider()->GetBgMax()->SetColor(ColorHelper::QTColorToDAVAColor(value));
}

int UISliderMetadata::GetMaxDrawType() const
{
    if (!VerifyActiveParamID())
    {
        return UIControlBackground::DRAW_ALIGNED;
    }

    return GetActiveUISlider()->GetBgMax()->GetDrawType();
}

void UISliderMetadata::SetMaxDrawType(int value)
{
    if (!VerifyActiveParamID())
    {
        return;
    }
    
    GetActiveUISlider()->SetMaxDrawType((UIControlBackground::eDrawType)value);
}

int UISliderMetadata::GetMaxColorInheritType() const
{
    if (!VerifyActiveParamID())
    {
        return UIControlBackground::COLOR_MULTIPLY_ON_PARENT;
    }
    
    return (int)GetActiveUISlider()->GetBgMax()->GetColorInheritType();
}

void UISliderMetadata::SetMaxColorInheritType(int value)
{
    if (!VerifyActiveParamID())
    {
        return;
    }
    
    GetActiveUISlider()->SetMaxColorInheritType((UIControlBackground::eColorInheritType)value);
}

int UISliderMetadata::GetMaxAlign() const
{
    if (!VerifyActiveParamID())
    {
        return ALIGN_LEFT;
    }
    
    return (int)GetActiveUISlider()->GetBgMax()->GetAlign();
}

void UISliderMetadata::SetMaxAlign(int value)
{
    if (!VerifyActiveParamID())
    {
        return;
    }
    
    GetActiveUISlider()->SetMaxAlign(value);
}

float UISliderMetadata::GetMaxLeftRightStretchCap() const
{
    if (!VerifyActiveParamID())
    {
        return -1.0;
    }

    return GetActiveUISlider()->GetBgMax()->GetLeftRightStretchCap();
}

void UISliderMetadata::SetMaxLeftRightStretchCap(float value)
{
    if (!VerifyActiveParamID())
    {
        return;
    }
    
    GetActiveUISlider()->SetMaxLeftRightStretchCap(value);
}

float UISliderMetadata::GetMaxTopBottomStretchCap() const
{
    if (!VerifyActiveParamID())
    {
        return -1.0;
    }
    
    return GetActiveUISlider()->GetBgMax()->GetTopBottomStretchCap();
}

void UISliderMetadata::SetMaxTopBottomStretchCap(float value)
{
    if (!VerifyActiveParamID())
    {
        return;
    }

    GetActiveUISlider()->SetMaxTopBottomStretchCap(value);
}

void UISliderMetadata::SetMaxSprite(const QString& value)
{
    if (!VerifyActiveParamID())
    {
        return;
    }
    
    //If empty string value is used - remove sprite
    if (value.isEmpty())
    {
        GetActiveUISlider()->SetMaxSprite(NULL, 0);
    }
    else
    {
        Sprite* sprite = Sprite::Create(value.toStdString());
        if (sprite)
        {
            GetActiveUISlider()->SetMaxSprite(sprite, 0);
            SafeRelease(sprite);
        }
    }
}

QString UISliderMetadata::GetMaxSprite() const
{
    if (!VerifyActiveParamID())
    {
        return QString();
    }

    Sprite* sprite = GetActiveUISlider()->GetBgMax()->GetSprite();
    if (sprite)
    {
        return QString::fromStdString(sprite->GetRelativePathname().GetAbsolutePathname());
    }

    return StringConstants::NO_SPRITE_IS_SET;
}

void UISliderMetadata::SetMaxSpriteFrame(int value)
{
    if (!VerifyActiveParamID())
    {
        return;
    }
    
    Sprite* sprite = GetActiveUISlider()->GetBgMax()->GetSprite();
    if (sprite && sprite->GetFrameCount() > value)
    {
        GetActiveUISlider()->SetMaxSpriteFrame(value);
    }
}

int UISliderMetadata::GetMaxSpriteFrame() const
{
    if (!VerifyActiveParamID())
    {
        return -1;
    }
    
    if (GetActiveUISlider()->GetBgMax()->GetSprite())
    {
        return GetActiveUISlider()->GetBgMax()->GetFrame();
    }
    
    return 0;
}

void UISliderMetadata::SetMaxSpriteModification(int value)
{
    if (!VerifyActiveParamID())
    {
        return;
    }
    
    Sprite* sprite = GetActiveUISlider()->GetBgMin()->GetSprite();
    if (sprite)
    {
        GetActiveUISlider()->SetMaxSpriteModification(value);
    }
}

int UISliderMetadata::GetMaxSpriteModification() const
{
    if (!VerifyActiveParamID())
    {
        return -1;
    }

    Sprite* sprite = GetActiveUIControl()->GetBackground()->GetSprite();
    if (sprite)
    {
        return GetActiveUISlider()->GetBgMax()->GetModification();
    }

    return 0;
}
