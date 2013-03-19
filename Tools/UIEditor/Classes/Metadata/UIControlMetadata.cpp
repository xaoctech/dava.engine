//
//  ControlNodeMetadata.cpp
//  UIEditor
//
//  Created by Yuri Coder on 10/15/12.
//
//

#include "UIControlMetadata.h"
#include "HierarchyTreeController.h"
#include "StringUtils.h"

#include <QtGlobal>

namespace DAVA {

UIControlMetadata::UIControlMetadata(QObject* parent) :
    BaseMetadata(parent)
{
}

QString UIControlMetadata::GetName() const
{
    if (!VerifyActiveParamID())
    {
        return QString();
    }

    return GetActiveUIControl()->GetName().c_str();
}

void UIControlMetadata::SetName(const QString& name)
{
    if (!VerifyActiveParamID() || !GetActiveTreeNode())
    {
        return;
    }

    // Need to update on the Tree Node level too.
    GetActiveUIControl()->SetName(name.toStdString());
    GetActiveTreeNode()->SetName(name);
}

int UIControlMetadata::GetTag() const
{
    if (!VerifyActiveParamID())
    {
        return -1;
    }

    return GetActiveUIControl()->GetTag();
}

void UIControlMetadata::SetTag(int tag)
{
    if (!VerifyActiveParamID())
    {
        return;
    }

    GetActiveUIControl()->SetTag(tag);
}

float UIControlMetadata::GetRelativeX() const
{
    if (!VerifyActiveParamID())
    {
        return -1.0f;
    }

    return GetActiveUIControl()->GetPosition(false).x;
}

void UIControlMetadata::SetRelativeX(float value)
{
    if (!VerifyActiveParamID())
    {
        return;
    }
    
    Vector2 relativePosition = GetActiveUIControl()->GetPosition(false);
    relativePosition.x = value;
    GetActiveUIControl()->SetPosition(relativePosition, false);
}
    
float UIControlMetadata::GetRelativeY() const
{
    if (!VerifyActiveParamID())
    {
        return -1.0f;
    }
    
    return GetActiveUIControl()->GetPosition(false).y;
}

void UIControlMetadata::SetRelativeY(float value)
{
    if (!VerifyActiveParamID())
    {
        return;
    }
    
    Vector2 relativePosition = GetActiveUIControl()->GetPosition(false);
    relativePosition.y = value;
    GetActiveUIControl()->SetPosition(relativePosition, false);
}

float UIControlMetadata::GetAbsoluteX() const
{
    if (!VerifyActiveParamID())
    {
        return -1.0f;
    }

    return GetActiveUIControl()->GetPosition(true).x;
}
    
void UIControlMetadata::SetAbsoluteX(float value)
{
    if (!VerifyActiveParamID())
    {
        return;
    }
    
    Vector2 absolutePosition = GetActiveUIControl()->GetPosition(true);
    absolutePosition.x = value;
    GetActiveUIControl()->SetPosition(absolutePosition, true);
}
    
float UIControlMetadata::GetAbsoluteY() const
{
    if (!VerifyActiveParamID())
    {
        return -1.0f;
    }
    
    return GetActiveUIControl()->GetPosition(true).y;
}
    
void UIControlMetadata::SetAbsoluteY(float value)
{
    if (!VerifyActiveParamID())
    {
        return;
    }

    Vector2 absolutePosition = GetActiveUIControl()->GetPosition(true);
    absolutePosition.y = value;
    GetActiveUIControl()->SetPosition(absolutePosition, true);
}

float UIControlMetadata::GetSizeX() const
{
    if (!VerifyActiveParamID())
    {
        return -1.0f;
    }
    
    return GetActiveUIControl()->GetSize().x;
}

void UIControlMetadata::SetSizeX(float value)
{
    if (!VerifyActiveParamID())
    {
        return;
    }
	
	Rect rect = GetActiveUIControl()->GetRect();
	rect.dx = value;
	
	SetActiveControlRect(rect);
}

float UIControlMetadata::GetSizeY() const
{
    if (!VerifyActiveParamID())
    {
        return -1.0f;
    }
    
    return GetActiveUIControl()->GetSize().y;
}
    
void UIControlMetadata::SetSizeY(float value)
{
    if (!VerifyActiveParamID())
    {
        return;
    }

	Rect rect = GetActiveUIControl()->GetRect();
	rect.dy = value;
	
	SetActiveControlRect(rect);
}

float UIControlMetadata::GetPivotX() const
{
    if (!VerifyActiveParamID())
    {
        return -1.0f;
    }

    return GetActiveUIControl()->pivotPoint.x;
}
    
void UIControlMetadata::SetPivotX(float value)
{
    if (!VerifyActiveParamID())
    {
        return;
    }
    
    GetActiveUIControl()->pivotPoint.x = value;
}

float UIControlMetadata::GetPivotY() const
{
    if (!VerifyActiveParamID())
    {
        return -1.0f;
    }

    return GetActiveUIControl()->pivotPoint.y;
}
    
void UIControlMetadata::SetPivotY(float value)
{
    if (!VerifyActiveParamID())
    {
        return;
    }
    
    GetActiveUIControl()->pivotPoint.y = value;
}

float UIControlMetadata::GetAngle() const
{
    if (!VerifyActiveParamID())
    {
        return -1.0f;
    }
	
    // Angle is stored in radians, but displayed in degrees.
	// RedToDeg is rather inaccurate - so we should round ret value to higher value
    return qRound(RadToDeg(GetActiveUIControl()->GetAngle()));
}
    
void UIControlMetadata::SetAngle(float value)
{
    if (!VerifyActiveParamID())
    {
        return;
    }

    // Angle is passed in degrees, but stored in radians.
    GetActiveUIControl()->SetAngle(DegToRad(value));
}

//Boolean getters/setters
bool UIControlMetadata::GetSelected() const
{
    if (!VerifyActiveParamID())
    {
        return false;
    }

    return GetActiveUIControl()->GetSelected();
}

void UIControlMetadata::SetSelected(const bool value)
{
    if (!VerifyActiveParamID())
    {
        return;
    }
    
    GetActiveUIControl()->SetSelected(value);
}

bool UIControlMetadata::GetVisible() const
{
    if (!VerifyActiveParamID())
    {
        return false;
    }
    
    return GetActiveUIControl()->GetVisible();
}

void UIControlMetadata::SetVisible(const bool value)
{
    
    if (!VerifyActiveParamID())
    {
        return;
    }
    
    GetActiveUIControl()->SetVisible(value);
}

bool UIControlMetadata::GetEnabled() const
{
    if (!VerifyActiveParamID())
    {
        return false;
    }
    
    return !GetActiveUIControl()->GetDisabled();
}

void UIControlMetadata::SetEnabled(const bool value)
{
    if (!VerifyActiveParamID())
    {
        return;
    }
    
    GetActiveUIControl()->SetDisabled(!value);
}

bool UIControlMetadata::GetInput() const
{
    if (!VerifyActiveParamID())
    {
        return false;
    }
    
    return GetActiveUIControl()->GetInputEnabled();
}

void UIControlMetadata::SetInput(const bool value)
{
    if (!VerifyActiveParamID())
    {
        return;
    }
    
    GetActiveUIControl()->SetInputEnabled(value);
}

bool UIControlMetadata::GetClipContents() const
{
    if (!VerifyActiveParamID())
    {
        return false;
    }
    
    return GetActiveUIControl()->GetClipContents();
}

void UIControlMetadata::SetClipContents(const bool value)
{
    if (!VerifyActiveParamID())
    {
        return;
    }
    
    GetActiveUIControl()->SetClipContents(value);
}

void UIControlMetadata::ApplyMove(const Vector2& moveDelta)
{
    if (!VerifyActiveParamID())
    {
        return;
    }

    Vector2 controlPosition = GetActiveUIControl()->GetPosition();
    controlPosition += moveDelta;
	
	Rect rect = GetActiveUIControl()->GetRect();
	rect.x = controlPosition.x;
	rect.y = controlPosition.y;
	
	SetActiveControlRect(rect);
}

void UIControlMetadata::ApplyResize(const Rect& /*originalRect*/, const Rect& newRect)
{
    if (!VerifyActiveParamID())
    {
        return;
    }
    
	SetActiveControlRect(newRect);
}
                 
QColor UIControlMetadata::GetColor()
{
    if (!VerifyActiveParamID())
    {
        return QColor();
    }

    return DAVAColorToQTColor(GetActiveUIControl()->GetBackground()->color);
}
    
void UIControlMetadata::SetColor(const QColor& value)
{
    if (!VerifyActiveParamID())
    {
        return;
    }

    GetActiveUIControl()->GetBackground()->SetColor(QTColorToDAVAColor(value));
}
    
int UIControlMetadata::GetDrawType()
{
    if (!VerifyActiveParamID())
    {
        return UIControlBackground::DRAW_ALIGNED;
    }

    return GetActiveUIControl()->GetBackground()->GetDrawType();
}
    
void UIControlMetadata::SetDrawType(int value)
{
    if (!VerifyActiveParamID())
    {
        return;
    }
    
    GetActiveUIControl()->GetBackground()->SetDrawType((UIControlBackground::eDrawType)value);
}
    
int UIControlMetadata::GetColorInheritType()
{
    if (!VerifyActiveParamID())
    {
        return UIControlBackground::COLOR_MULTIPLY_ON_PARENT;
    }
    
    return (int)GetActiveUIControl()->GetBackground()->GetColorInheritType();
}
    
void UIControlMetadata::SetColorInheritType(int value)
{
    if (!VerifyActiveParamID())
    {
        return;
    }
    
    GetActiveUIControl()->GetBackground()->SetColorInheritType((UIControlBackground::eColorInheritType)value);
}
    
int UIControlMetadata::GetAlign()
{
    if (!VerifyActiveParamID())
    {
        return ALIGN_LEFT;
    }
    
    return (int)GetActiveUIControl()->GetBackground()->GetAlign();
}
    
void UIControlMetadata::SetAlign(int value)
{
    if (!VerifyActiveParamID())
    {
        return;
    }
    
    GetActiveUIControl()->GetBackground()->SetAlign((eAlign)value);
}

float UIControlMetadata::GetLeftRightStretchCap()
{
    if (!VerifyActiveParamID())
    {
        return -1.0;
    }
    
    return GetActiveUIControl()->GetBackground()->GetLeftRightStretchCap();
}

float UIControlMetadata::GetTopBottomStretchCap()
{
    if (!VerifyActiveParamID())
    {
        return -1.0;
    }
    
    return GetActiveUIControl()->GetBackground()->GetTopBottomStretchCap();
}

void UIControlMetadata::SetLeftRightStretchCap(float value)
{
    if (!VerifyActiveParamID())
    {
        return;
    }
    
    GetActiveUIControl()->GetBackground()->SetLeftRightStretchCap(value);
}

void UIControlMetadata::SetTopBottomStretchCap(float value)
{
    if (!VerifyActiveParamID())
    {
        return;
    }
    
    GetActiveUIControl()->GetBackground()->SetTopBottomStretchCap(value);
}
    
void UIControlMetadata::SetSprite(const QString& value)
{
    if (!VerifyActiveParamID())
    {
        return;
    }
    
    //If empty string value is used - remove sprite
    if (value.isEmpty())
    {
        GetActiveUIControl()->GetBackground()->SetSprite(NULL, 0); 
    }
    else
    {
        Sprite* sprite = Sprite::Create(TruncateTxtFileExtension(value).toStdString());
        if (sprite)
        {
            GetActiveUIControl()->GetBackground()->SetSprite(sprite, 0);
            SafeRelease(sprite);
        }
    }
}
    
QString UIControlMetadata::GetSprite()
{
    if (!VerifyActiveParamID())
    {
        return QString();
    }

    Sprite* sprite = GetActiveUIControl()->GetBackground()->GetSprite();
    if (sprite == NULL)
    {
        return "<No sprite is set>";
    }
    
    return sprite->GetRelativePathname().c_str();
}
    
void UIControlMetadata::SetSpriteFrame(int value)
{
    if (!VerifyActiveParamID())
    {
        return;
    }
    
    Sprite* sprite = GetActiveUIControl()->GetBackground()->GetSprite();
    if (sprite == NULL)
    {
        return;
    }

    if (sprite->GetFrameCount() <= value)
    {
        // No way to set this frame.
        return;
    }

    GetActiveUIControl()->GetBackground()->SetFrame(value);
}
    
int UIControlMetadata::GetSpriteFrame()
{
    if (!VerifyActiveParamID())
    {
        return -1;
    }
    
    Sprite* sprite = GetActiveUIControl()->GetBackground()->GetSprite();
    if (sprite == NULL)
    {
        return 0;
    }
    
    return GetActiveUIControl()->GetBackground()->GetFrame();
}

void UIControlMetadata::SetSpriteModification(int value)
{
    if (!VerifyActiveParamID())
    {
        return;
    }
    
    Sprite* sprite = GetActiveUIControl()->GetBackground()->GetSprite();
    if (sprite == NULL)
    {
        return;
    }

    GetActiveUIControl()->GetBackground()->SetModification(value);
}

int UIControlMetadata::GetSpriteModification()
{
    if (!VerifyActiveParamID())
    {
        return -1;
    }
    
    Sprite* sprite = GetActiveUIControl()->GetBackground()->GetSprite();
    if (sprite == NULL)
    {
        return 0;
    }
    
    return GetActiveUIControl()->GetBackground()->GetModification();
}

int UIControlMetadata::GetLeftAlign()
{
    if (!VerifyActiveParamID())
    {
        return 0;
    }
	return GetActiveUIControl()->GetLeftAlign();
}

void UIControlMetadata::SetLeftAlign(int value)
{
    if (!VerifyActiveParamID())
    {
        return;
    }
	
	GetActiveUIControl()->SetLeftAlign(value);
}
	
int UIControlMetadata::GetHCenterAlign()
{
    if (!VerifyActiveParamID())
    {
        return 0;
    }
	
	return GetActiveUIControl()->GetHCenterAlign();
}

void UIControlMetadata::SetHCenterAlign(int value)
{
    if (!VerifyActiveParamID())
    {
        return;
    }

	GetActiveUIControl()->SetHCenterAlign(value);
}

int UIControlMetadata::GetRightAlign()
{
    if (!VerifyActiveParamID())
    {
        return 0;
    }
	
	return GetActiveUIControl()->GetRightAlign();
}

void UIControlMetadata::SetRightAlign(int value)
{
    if (!VerifyActiveParamID())
    {
        return;
    }
	
	GetActiveUIControl()->SetRightAlign(value);
}

int UIControlMetadata::GetTopAlign()
{
    if (!VerifyActiveParamID())
    {
        return 0;
    }
	
	return GetActiveUIControl()->GetTopAlign();
}

void UIControlMetadata::SetTopAlign(int value)
{
    if (!VerifyActiveParamID())
    {
        return;
    }
	
	GetActiveUIControl()->SetTopAlign(value);
}

int UIControlMetadata::GetVCenterAlign()
{
    if (!VerifyActiveParamID())
    {
        return 0;
    }
	
	return GetActiveUIControl()->GetVCenterAlign();
}

void UIControlMetadata::SetVCenterAlign(int value)
{
    if (!VerifyActiveParamID())
    {
        return;
    }
	
	GetActiveUIControl()->SetVCenterAlign(value);
}

int UIControlMetadata::GetBottomAlign()
{
    if (!VerifyActiveParamID())
    {
        return 0;
    }
	
	return GetActiveUIControl()->GetBottomAlign();
}

void UIControlMetadata::SetBottomAlign(int value)
{
    if (!VerifyActiveParamID())
    {
        return;
    }
	
	GetActiveUIControl()->SetBottomAlign(value);
}

bool UIControlMetadata::GetLeftAlignEnabled() const
{
    if (!VerifyActiveParamID())
    {
        return false;
    }
	
	return GetActiveUIControl()->GetLeftAlignEnabled();
}

void UIControlMetadata::SetLeftAlignEnabled(const bool value)
{
    if (!VerifyActiveParamID())
    {
        return;
    }
	
	GetActiveUIControl()->SetLeftAlignEnabled(value);
}
	
bool UIControlMetadata::GetHCenterAlignEnabled() const
{
    if (!VerifyActiveParamID())
    {
        return false;
    }
	
	return GetActiveUIControl()->GetHCenterAlignEnabled();
}

void UIControlMetadata::SetHCenterAlignEnabled(const bool value)
{
    if (!VerifyActiveParamID())
    {
        return;
    }
	
	GetActiveUIControl()->SetHCenterAlignEnabled(value);
}
	
bool UIControlMetadata::GetRightAlignEnabled() const
{
    if (!VerifyActiveParamID())
    {
        return false;
    }
	
	return GetActiveUIControl()->GetRightAlignEnabled();
}

void UIControlMetadata::SetRightAlignEnabled(const bool value)
{
    if (!VerifyActiveParamID())
    {
        return;
    }
	
	GetActiveUIControl()->SetRightAlignEnabled(value);	
}
	
bool UIControlMetadata::GetTopAlignEnabled() const
{
    if (!VerifyActiveParamID())
    {
        return false;
    }
	
	return GetActiveUIControl()->GetTopAlignEnabled();
}

void UIControlMetadata::SetTopAlignEnabled(const bool value)
{
    if (!VerifyActiveParamID())
    {
        return;
    }
	
	GetActiveUIControl()->SetTopAlignEnabled(value);
}
	
bool UIControlMetadata::GetVCenterAlignEnabled() const
{
    if (!VerifyActiveParamID())
    {
        return false;
    }
	
	return GetActiveUIControl()->GetVCenterAlignEnabled();
}

void UIControlMetadata::SetVCenterAlignEnabled(const bool value)
{
    if (!VerifyActiveParamID())
    {
        return;
    }
	
	GetActiveUIControl()->SetVCenterAlignEnabled(value);
}
	
bool UIControlMetadata::GetBottomAlignEnabled() const
{
    if (!VerifyActiveParamID())
    {
        return false;
    }
	
	return GetActiveUIControl()->GetBottomAlignEnabled();
}

void UIControlMetadata::SetBottomAlignEnabled(const bool value)
{
    if (!VerifyActiveParamID())
    {
        return;
    }
	
	GetActiveUIControl()->SetBottomAlignEnabled(value);
}

void UIControlMetadata::SetActiveControlRect(const Rect& rect)
{
	GetActiveUIControl()->SetRect(rect);
}

};