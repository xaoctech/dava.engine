/*==================================================================================
    Copyright (c) 2008, DAVA Consulting, LLC
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are met:
    * Redistributions of source code must retain the above copyright
    notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
    notice, this list of conditions and the following disclaimer in the
    documentation and/or other materials provided with the distribution.
    * Neither the name of the DAVA Consulting, LLC nor the
    names of its contributors may be used to endorse or promote products
    derived from this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE DAVA CONSULTING, LLC AND CONTRIBUTORS "AS IS" AND
    ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL DAVA CONSULTING, LLC BE LIABLE FOR ANY
    DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
    ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
    SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
=====================================================================================*/

#include "ForcePreviewControl.h"

REGISTER_CLASS(ForcePreviewControl);

ForcePreviewControl::ForcePreviewControl() 
{
    GetBackground()->SetDrawType(UIControlBackground::DRAW_FILL);
    GetBackground()->SetColor(Color(0.2f, 0.2f, 0.2f, 1.f));
    
    Font *f = FTFont::Create("~res:/Fonts/MyriadPro-Regular.otf");
    f->SetSize(10);
    f->SetColor(Color(1,1,1,1));
    powerText = new UIStaticText();
    powerText->SetFont(f);
    AddControl(powerText);
    SafeRelease(f);
}

void ForcePreviewControl::SetRect(const DAVA::Rect &rect)
{
    UIControl::SetRect(rect);
    powerText->SetRect(Rect(0, 0, rect.dx, rect.dy/10));
}

void ForcePreviewControl::Input(DAVA::UIEvent *touch)
{

}

void ForcePreviewControl::Update(float32 timeElapsed)
{
    
}

void ForcePreviewControl::SetValue(Vector3 _value)
{
    value = _value;
    powerText->SetText(Format(L"Force Power: %.2f", value.Length()));
}

void ForcePreviewControl::Draw(const DAVA::UIGeometricData &geometricData)
{
    UIControl::Draw(geometricData);
    RenderManager::Instance()->SetColor(1, 1, 1, 1);
    Rect controlRect = GetRect();
    Vector2 center = Vector2(controlRect.x + controlRect.dx/2, controlRect.y + 11*controlRect.dy/20);
    float32 l = value.Length();
    RenderHelper::Instance()->DrawLine(center, center + Vector2(value.x, value.y)/l*controlRect.dx*0.45f);
    
    RenderHelper::Instance()->DrawCircle(center, controlRect.dx*0.5f);
}
