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



#include "UI/UISlider.h"
#include "UI/UIButton.h"
#include "Render/RenderManager.h"
#include "Render/RenderHelper.h"
#include "FileSystem/YamlNode.h"
#include "Base/ObjectFactory.h"
#include "Utils/Utils.h"
#include "Core/Core.h"

namespace DAVA 
{
	
	
// Use these names for children buttons to define UISlider in .yaml
static const String UISLIDER_THUMB_SPRITE_CONTROL_NAME = "thumbSpriteControl";
static const String UISLIDER_MIN_SPRITE_CONTROL_NAME = "minSpriteControl";
static const String UISLIDER_MAX_SPRITE_CONTROL_NAME = "maxSpriteControl";

UISlider::UISlider(const Rect & rect)
:	UIControl(rect)
,	minBackground(NULL)
,	maxBackground(NULL)
,	thumbButton(NULL)
,	minDrawType(UIControlBackground::DRAW_ALIGNED)
,	maxDrawType(UIControlBackground::DRAW_ALIGNED)
,   needSetMinDrawType(false)
,   needSetMaxDrawType(false)
,   spritesEmbedded(false)
{
    SetInputEnabled(true, false);
	isEventsContinuos = true;
	
	leftInactivePart = 0;
	rightInactivePart = 0;
	minValue = 0.0f;
	maxValue = 1.0f;
	currentValue = 0.5f;

	InitSubcontrols();
}
	
void UISlider::InitThumb()
{
	if (!thumbButton)
	{
		thumbButton = new UIControl(Rect(0, 0, 40.f, 40.f));
		thumbButton->SetName(UISLIDER_THUMB_SPRITE_CONTROL_NAME);
		UIControl::AddControl(thumbButton);
	}
	
	thumbButton->SetInputEnabled(false);
	thumbButton->relativePosition.y = size.y * 0.5f;
    thumbButton->pivotPoint = thumbButton->size*0.5f;
	
	SetValue(currentValue);
}

void UISlider::AddControl(DAVA::UIControl *control)
{
	// Synchronize the pointers to the buttons each time new control is added.
	UIControl::AddControl(control);

	if (control->GetName() == UISLIDER_THUMB_SPRITE_CONTROL_NAME && control != thumbButton)
	{
        RemoveAndReleaseControl(thumbButton);
		thumbButton = SafeRetain(control);
	}
}
		
void UISlider::InitMinBackground()
{
	if (!minBackground)
	{
        minBackground = new UIControlBackground();
	}
}

void UISlider::InitMaxBackground()
{
	if (!maxBackground)
	{
		maxBackground = new UIControlBackground();
	}
}

void UISlider::ReleaseAllSubcontrols()
{
    RemoveAndReleaseControl(thumbButton);
    SafeRelease(minBackground);
    SafeRelease(maxBackground);
}

void UISlider::InitInactiveParts(Sprite* spr)
{
	if(NULL == spr)
	{
		return;
	}

	leftInactivePart = rightInactivePart = (int32)((spr->GetWidth() / 2.0f));
}

void UISlider::SetThumb(UIControl *newThumb)
{
    RemoveAndReleaseControl(thumbButton);

    thumbButton = SafeRetain(newThumb);
	thumbButton->SetName(UISLIDER_THUMB_SPRITE_CONTROL_NAME);
	thumbButton->SetInputEnabled(false);
	
	thumbButton->relativePosition.y = size.y * 0.5f;
    thumbButton->pivotPoint = thumbButton->size*0.5f;
	
	UIControl::AddControl(thumbButton);
	
	SetValue(currentValue);
}
    
    
UISlider::~UISlider()
{
	SafeRelease(minBackground);
	SafeRelease(maxBackground);
	SafeRelease(thumbButton);
}

void UISlider::SetThumbSprite(Sprite * sprite, int32 frame)
{
	thumbButton->SetSprite(sprite, frame);
	InitInactiveParts(sprite);
}

void UISlider::SetThumbSprite(const FilePath & spriteName, int32 frame)
{
	thumbButton->SetSprite(spriteName, frame);
	InitInactiveParts(thumbButton->GetBackground()->GetSprite());
}

void UISlider::SetMinSprite(Sprite * sprite, int32 frame)
{
	InitMinBackground();
	minBackground->SetSprite(sprite, frame);
}
void UISlider::SetMinSprite(const FilePath & spriteName, int32 frame)
{
	InitMinBackground();
	minBackground->SetSprite(spriteName, frame);
}
	
void UISlider::SetMinDrawType(UIControlBackground::eDrawType drawType)
{
	InitMinBackground();
    minBackground->SetDrawType(drawType);
}
    
void UISlider::SetMinLeftRightStretchCap(float32 stretchCap)
{
	InitMinBackground();
    minBackground->SetLeftRightStretchCap(stretchCap);
}

void UISlider::SetMinTopBottomStretchCap(float32 stretchCap)
{
    InitMinBackground();
    minBackground->SetTopBottomStretchCap(stretchCap);
}

void UISlider::SetMaxSprite(Sprite * sprite, int32 frame)
{
	InitMaxBackground();
	maxBackground->SetSprite(sprite, frame);
}
	
void UISlider::SetMaxSprite(const FilePath & spriteName, int32 frame)
{
	InitMaxBackground();
	maxBackground->SetSprite(spriteName, frame);
}
    
void UISlider::SetMaxDrawType(UIControlBackground::eDrawType drawType)
{
	InitMaxBackground();
    maxBackground->SetDrawType(drawType);
}

void UISlider::SetMaxLeftRightStretchCap(float32 stretchCap)
{
	InitMaxBackground();
    maxBackground->SetLeftRightStretchCap(stretchCap);
}

void UISlider::SetMaxTopBottomStretchCap(float32 stretchCap)
{
    InitMaxBackground();
    maxBackground->SetTopBottomStretchCap(stretchCap);
}

void UISlider::RecalcButtonPos()
{
	if (thumbButton)
	{
		thumbButton->relativePosition.x = Interpolation::Linear((float32)leftInactivePart, size.x - rightInactivePart, minValue, currentValue, maxValue);
        thumbButton->relativePosition.y = GetSize().y / 2; // thumb button pivot point is on center.
	}
}

void UISlider::SyncThumbWithSprite()
{
   	RecalcButtonPos();
}

void UISlider::SetValue(float32 value)
{
	currentValue = value;
	RecalcButtonPos();
}

void UISlider::SetMinValue(float32 value)
{
    minValue = value;
    RecalcButtonPos();
}
    
void UISlider::SetMaxValue(float32 value)
{
    maxValue = value;
    RecalcButtonPos();
}
    
void UISlider::SetMinMaxValue(float32 _minValue, float32 _maxValue)
{
	minValue = _minValue;
	maxValue = _maxValue;
	SetValue((minValue + maxValue) / 2.0f);
}

void UISlider::Input(UIEvent *currentInput)
{
#if !defined(__DAVAENGINE_IPHONE__) && !defined(__DAVAENGINE_ANDROID__)                                        
	if (currentInput->phase == UIEvent::PHASE_MOVE || currentInput->phase == UIEvent::PHASE_KEYCHAR)
		return;
#endif
	
	const Rect & absRect = GetGeometricData().GetUnrotatedRect();
	//absTouchPoint = currentInput->point;
	
	relTouchPoint = currentInput->point;
	relTouchPoint -= absRect.GetPosition();
	
	
	float oldVal = currentValue;
	currentValue = Interpolation::Linear(minValue, maxValue, (float32)leftInactivePart, relTouchPoint.x, size.x - (float32)rightInactivePart);
	
	if(currentValue < minValue)
	{
		currentValue = minValue;
	}
	if(currentValue > maxValue)
	{
		currentValue = maxValue;
	}

	if (isEventsContinuos) // if continuos events
	{
		if(oldVal != currentValue)
		{
			PerformEvent(EVENT_VALUE_CHANGED);
		}
	}else if (currentInput->phase == UIEvent::PHASE_ENDED) 
	{
		/* if not continuos always perform event because last move position almost always the same as end pos */
		PerformEvent(EVENT_VALUE_CHANGED);
	}

	RecalcButtonPos();
	currentInput->SetInputHandledType(UIEvent::INPUT_HANDLED_HARD); // Drag is handled - see please DF-2508.
}

void UISlider::Draw(const UIGeometricData &geometricData)
{
	const Rect & aRect =  thumbButton->GetGeometricData().GetUnrotatedRect();
	float32 clipPointAbsolute = aRect.x + aRect.dx * 0.5f;

    const Vector2& drawTranslate = RenderManager::Instance()->GetDrawTranslate();
    const Vector2& drawScale = RenderManager::Instance()->GetDrawScale();

    float32 screenXMin = (Core::Instance()->GetVirtualScreenXMin() - drawTranslate.x) / drawScale.x;
    float32 screenXMax = (Core::Instance()->GetVirtualScreenXMax() - drawTranslate.x) / drawScale.x;
    float32 screenYMin = - drawTranslate.y / drawScale.y;
    float32 screenYMax = (GetScreenHeight() - drawTranslate.y) / drawScale.y;

	if (minBackground)
	{
		minBackground->SetParentColor(GetBackground()->GetDrawColor());
		RenderManager::Instance()->ClipPush();
        RenderManager::Instance()->ClipRect(Rect(screenXMin, screenYMin, clipPointAbsolute - screenXMin, screenYMax));
		minBackground->Draw(geometricData);
		RenderManager::Instance()->ClipPop();
	}
	if (maxBackground)
	{
		maxBackground->SetParentColor(GetBackground()->GetDrawColor());
		RenderManager::Instance()->ClipPush();
        RenderManager::Instance()->ClipRect(Rect(clipPointAbsolute, screenYMin, screenXMax - clipPointAbsolute, screenYMax));
		maxBackground->Draw(geometricData);
		RenderManager::Instance()->ClipPop();
	}

	if (!minBackground && !maxBackground)
	{
		UIControl::Draw(geometricData);
	}
}

void UISlider::LoadFromYamlNode(const YamlNode * node, UIYamlLoader * loader)
{
	UIControl::LoadFromYamlNode(node, loader);
	
	ReleaseAllSubcontrols();
	const YamlNode * thumbSpriteNode = node->Get("thumbSprite");

	if (thumbSpriteNode)
	{
		// Yuri Coder, 2012/04/24. This is old configuration version without the subcontrols.
		// Need to create sprite subcontrol.
		InitThumb();
		const YamlNode * spriteNode = thumbSpriteNode->Get(0);
		const YamlNode * frameNode = thumbSpriteNode->Get(1);
		
		if (spriteNode)
		{
			SetThumbSprite(spriteNode->AsString(), frameNode->AsInt32());
		}
	}
	
	const YamlNode * minSpriteNode = node->Get("minSprite");
	
	if (minSpriteNode)
	{
		// Yuri Coder, 2012/04/24. This is old configuration version without the subcontrols.
		// Need to create min background subcontrol.
		InitMinBackground();
		const YamlNode * spriteNode = minSpriteNode->Get(0);
		const YamlNode * frameNode = minSpriteNode->Get(1);
		
		if (spriteNode)
		{
			SetMinSprite(spriteNode->AsString(), frameNode->AsInt32());
		}
	}
	
	const YamlNode * maxSpriteNode = node->Get("maxSprite");
	
	if (maxSpriteNode)
	{
		// Yuri Coder, 2012/04/24. This is old configuration version without the subcontrols.
		// Need to create max background subcontrol.
		InitMaxBackground();
		const YamlNode * spriteNode = maxSpriteNode->Get(0);
		const YamlNode * frameNode = maxSpriteNode->Get(1);
		
		if (spriteNode)
		{
			SetMaxSprite(spriteNode->AsString(), frameNode->AsInt32());
		}
	}
	
	// Values
	const YamlNode * valueNode = node->Get("value");
	
	if (valueNode)
		SetValue(valueNode->AsFloat());
		
	const YamlNode * minValueNode= node->Get("minValue");
	
	if (minValueNode)
		SetMinValue(minValueNode->AsFloat());
		
	const YamlNode * maxValueNode= node->Get("maxValue");
	
	if (maxValueNode)
		SetMaxValue(maxValueNode->AsFloat());
	
	
	// Load the Min/Max draw types to apply them when the loading will be completed.
	const YamlNode * minDrawTypeNode = node->Get("minDrawType");

	if(minDrawTypeNode)
	{
		this->minDrawType =(UIControlBackground::eDrawType)loader->GetDrawTypeFromNode(minDrawTypeNode);
        this->needSetMinDrawType = true;
	}
	
	const YamlNode * maxDrawTypeNode = node->Get("maxDrawType");
	if(maxDrawTypeNode)
	{
		this->maxDrawType= (UIControlBackground::eDrawType)loader->GetDrawTypeFromNode(maxDrawTypeNode);
        this->needSetMaxDrawType = true;
	}

    const YamlNode* spritesEmbeddedNode = node->Get("spritesEmbedded");
    if (spritesEmbeddedNode)
    {
        this->spritesEmbedded = spritesEmbeddedNode->AsBool();
    }
    
    if (this->spritesEmbedded)
    {
        // File is saved in new format - load the backgrounds.
        LoadBackgound("min", minBackground, node, loader);
        LoadBackgound("max", maxBackground, node, loader);
    }
}

void UISlider::SetSize(const DAVA::Vector2 &newSize)
{
    UIControl::SetSize(newSize);
    RecalcButtonPos();
}
    
void UISlider::LoadFromYamlNodeCompleted()
{
    AttachToSubcontrols();
    if (!spritesEmbedded)
    {
        // Old Yaml format is used - have to take their data and remove subcontrols.
        UIControl* minBgControl = FindByName(UISLIDER_MIN_SPRITE_CONTROL_NAME, false);
        CopyBackgroundAndRemoveControl(minBgControl, &minBackground);
        
        UIControl* maxBgControl = FindByName(UISLIDER_MAX_SPRITE_CONTROL_NAME, false);
        CopyBackgroundAndRemoveControl(maxBgControl, &maxBackground);
    }
    
    if (this->needSetMinDrawType)
    {
        minBackground->SetDrawType(minDrawType);
    }

    if (needSetMaxDrawType)
    {
        maxBackground->SetDrawType(maxDrawType);
    }

    SyncThumbWithSprite();
}
	
YamlNode * UISlider::SaveToYamlNode(UIYamlLoader * loader)
{
    thumbButton->SetName(UISLIDER_THUMB_SPRITE_CONTROL_NAME);

    YamlNode *node = UIControl::SaveToYamlNode(loader);

	// Sprite value
	float32 value = this->GetValue();
	node->Set("value", value);
	
	// Sprite min value
	value = this->GetMinValue();
	node->Set("minValue", value);
	
	// Sprite max value
	value = this->GetMaxValue();
	node->Set("maxValue", value);

    // Min/max background sprites.
    SaveBackground("min", minBackground, node, loader);
    SaveBackground("max", maxBackground, node, loader);

    // Sprites are now embedded into UISlider.
    node->Set("spritesEmbedded", true);

    return node;
}
	
UIControl* UISlider::Clone()
{
	UISlider *t = new UISlider(GetRect());
	t->CopyDataFrom(this);
	return t;
}
	
void UISlider::CopyDataFrom(UIControl *srcControl)
{
	UIControl::CopyDataFrom(srcControl);
	UISlider* t = (UISlider*) srcControl;

	isEventsContinuos = t->isEventsContinuos;
	
	leftInactivePart = t->leftInactivePart;
	rightInactivePart = t->rightInactivePart;
	
	minValue = t->minValue;
	maxValue = t->maxValue;
	
	currentValue = t->currentValue;

    ReleaseAllSubcontrols();
	if (t->thumbButton)
	{
		UIControl *c = t->thumbButton->Clone();
		AddControl(c);
		c->Release();
	}
	if (t->minBackground)
	{
        minBackground = t->minBackground->Clone();
	}
	if (t->maxBackground)
	{
        maxBackground = t->maxBackground->Clone();
	}
	
	relTouchPoint = t->relTouchPoint;
}

void UISlider::InitSubcontrols()
{
	InitThumb();
	InitMinBackground();
	InitMaxBackground();
}
	
void UISlider::AttachToSubcontrols()
{
	if (!thumbButton)
	{
		thumbButton = FindByName(UISLIDER_THUMB_SPRITE_CONTROL_NAME);
		DVASSERT(thumbButton);
        thumbButton->Retain();
        
		InitInactiveParts(thumbButton->GetBackground()->GetSprite());
	}
}

List<UIControl*> UISlider::GetSubcontrols()
{
	List<UIControl*> subControls;
	AddControlToList(subControls, UISLIDER_THUMB_SPRITE_CONTROL_NAME);

	return subControls;
}

void UISlider::RemoveAndReleaseControl(UIControl* &control)
{
    if (!control)
    {
        return;
    }
    
    RemoveControl(control);
    SafeRelease(control);
}

void UISlider::SetVisibleForUIEditor(bool value, bool hierarchic/* = true*/)
{
    UIControl::SetVisibleForUIEditor(value, hierarchic);
    if (thumbButton)
    {
        thumbButton->SetVisibleForUIEditor(value, hierarchic);
    }
}

void UISlider::LoadBackgound(const char* prefix, UIControlBackground* background, const YamlNode* rootNode, UIYamlLoader* loader)
{
    const YamlNode * colorNode = rootNode->Get(Format("%scolor", prefix));
    const YamlNode * spriteNode = rootNode->Get(Format("%ssprite", prefix));
    const YamlNode * frameNode = rootNode->Get(Format("%sframe", prefix));
    const YamlNode * alignNode = rootNode->Get(Format("%salign", prefix));
    const YamlNode * colorInheritNode = rootNode->Get(Format("%scolorInherit", prefix));
    const YamlNode * drawTypeNode = rootNode->Get(Format("%sdrawType", prefix));
    const YamlNode * leftRightStretchCapNode = rootNode->Get(Format("%sleftRightStretchCap", prefix));
    const YamlNode * topBottomStretchCapNode = rootNode->Get(Format("%stopBottomStretchCap", prefix));
    const YamlNode * spriteModificationNode = rootNode->Get(Format("%sspriteModification", prefix));

    if (colorNode)
    {
        background->SetColor(colorNode->AsColor());
    }

    if (spriteNode)
    {
        background->SetSprite(Sprite::Create(spriteNode->AsString()), 0);
    }
    
    if (frameNode)
    {
        background->SetFrame(frameNode->AsInt32());
    }

    if (alignNode)
    {
        background->SetAlign(loader->GetAlignFromYamlNode(alignNode));
    }

    if (colorInheritNode)
    {
        background->SetColorInheritType((UIControlBackground::eColorInheritType)loader->GetColorInheritTypeFromNode(colorInheritNode));
    }
    
    if(drawTypeNode)
    {
        background->SetDrawType((UIControlBackground::eDrawType)loader->GetDrawTypeFromNode(drawTypeNode));
        
        if(leftRightStretchCapNode)
        {
            background->SetLeftRightStretchCap(leftRightStretchCapNode->AsFloat());
        }
        
        if(topBottomStretchCapNode)
        {
            background->SetTopBottomStretchCap(topBottomStretchCapNode->AsFloat());
        }
    }

    if (spriteModificationNode)
    {
        background->SetModification(spriteModificationNode->AsInt32());
    }
}

void UISlider::SaveBackground(const char* prefix, UIControlBackground* background, YamlNode* rootNode, UIYamlLoader * loader)
{
    if (!background)
    {
        return;
    }

    ScopedPtr<UIControlBackground> baseBackground(new UIControlBackground());

    // Color.
    Color color = background->GetColor();
    if (baseBackground->GetColor() != color)
    {
        VariantType* nodeValue = new VariantType();
        nodeValue->SetColor(color);
        rootNode->Set(Format("%scolor", prefix), nodeValue);
        SafeDelete(nodeValue);
    }

    // Sprite.
    Sprite *sprite = background->GetSprite();
    if (sprite)
    {
        rootNode->Set(Format("%ssprite", prefix), GetSpriteFrameworkPath(sprite));
    }
    int32 frame = background->GetFrame();
    if (baseBackground->GetFrame() != frame)
    {
        rootNode->Set(Format("%sframe", prefix), frame);
    }

    // Align
    int32 align = background->GetAlign();
    if (baseBackground->GetAlign() != align)
    {
        rootNode->AddNodeToMap(Format("%salign", prefix), loader->GetAlignNodeValue(align));
    }

    // Color inherit
    UIControlBackground::eColorInheritType colorInheritType =  background->GetColorInheritType();
    if (baseBackground->GetColorInheritType() != colorInheritType)
    {
        rootNode->Set(Format("%scolorInherit", prefix), loader->GetColorInheritTypeNodeValue(colorInheritType));
    }

    // Draw type.
    UIControlBackground::eDrawType drawType = background->GetDrawType();
    rootNode->Set(Format("%sdrawType", prefix), loader->GetDrawTypeNodeValue(drawType));

    // Stretch Cap.
    int32 leftRightStretchCap = background->GetLeftRightStretchCap();
    if (baseBackground->GetLeftRightStretchCap() != leftRightStretchCap)
    {
        rootNode->Set(Format("%sleftRightStretchCap", prefix), leftRightStretchCap);
        }
    int32 topBottomStretchCap = background->GetTopBottomStretchCap();
    if (baseBackground->GetTopBottomStretchCap() != topBottomStretchCap)
    {
        rootNode->Set(Format("%stopBottomStretchCap", prefix), topBottomStretchCap);
    }

    // spriteModification
    int32 modification = background->GetModification();
    if (baseBackground->GetModification() != modification)
    {
        rootNode->Set(Format("%sspriteModification", prefix), modification);
    }
}

void UISlider::CopyBackgroundAndRemoveControl(UIControl* from, UIControlBackground** to)
{
    if (!from)
    {
        return;
    }
    
    if (*to)
    {
        SafeRelease(*to);
    }

    *to = from->GetBackground()->Clone();
    RemoveControl(from);
}

void UISlider::SetMinSpriteFrame(int32 frame)
{
    InitMinBackground();
    if (minBackground->GetSprite())
    {
        minBackground->SetFrame(frame);
    }
}

void UISlider::SetMaxSpriteFrame(int32 frame)
{
    InitMaxBackground();
    if (maxBackground->GetSprite())
    {
        maxBackground->SetFrame(frame);
    }
}

void UISlider::SetMinSpriteModification(int32 value)
{
    InitMinBackground();
    minBackground->SetModification(value);
}

void UISlider::SetMaxSpriteModification(int32 value)
{
    InitMaxBackground();
    maxBackground->SetModification(value);
}

void UISlider::SetMinColorInheritType(UIControlBackground::eColorInheritType inheritType)
{
    InitMinBackground();
    minBackground->SetColorInheritType(inheritType);
}

void UISlider::SetMaxColorInheritType(UIControlBackground::eColorInheritType inheritType)
{
    InitMaxBackground();
    maxBackground->SetColorInheritType(inheritType);
}

void UISlider::SetMinAlign(int32 value)
{
    InitMinBackground();
    minBackground->SetAlign(value);
}

void UISlider::SetMaxAlign(int32 value)
{
    InitMaxBackground();
    maxBackground->SetAlign(value);
}
	
} // ns
