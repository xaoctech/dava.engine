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

#include "PreviewControl.h"

REGISTER_CLASS(PreviewControl); 

PreviewControl::PreviewControl()
{
    GetBackground()->SetDrawType(UIControlBackground::DRAW_FILL);
    GetBackground()->SetColor(Color(0, 0, 0, 1));
}

PreviewControl::PreviewControl(ParticleEmitter *emitt)
{
    GetBackground()->SetDrawType(UIControlBackground::DRAW_FILL);
    GetBackground()->SetColor(Color(0, 0, 0, 1));
    emitter = emitt;
    emitter->SetPosition(Vector2(GetRect().x + GetRect().dx/2, GetRect().y + GetRect().dy/2));
}

void PreviewControl::Input(DAVA::UIEvent *touch)
{
	if( (touch->phase == UIEvent::PHASE_DRAG || touch->phase == UIEvent::PHASE_BEGAN) && (touch->tid == UIEvent::BUTTON_1) && (touch->point.x > GetPosition().x) )
	{
		emitter->SetPosition(Vector2(touch->point.x, touch->point.y));
	}
    
	if( (touch->phase == UIEvent::PHASE_ENDED) && (touch->tid == UIEvent::BUTTON_2))
	{
		emitter->Restart(true);
	}
}

void PreviewControl::SetEmitter(ParticleEmitter *emitt)
{
    emitter = emitt;
    emitter->SetPosition(Vector2(GetRect().x + GetRect().dx/2, GetRect().y + GetRect().dy/2));
}

ParticleEmitter * PreviewControl::GetEmitter()
{
    return emitter;
}

void PreviewControl::Update(float32 timeElapsed)
{
    emitter->Update(timeElapsed);
}

void PreviewControl::Draw(const DAVA::UIGeometricData &geometricData)
{
    UIControl::Draw(geometricData);
    if(emitter)
    {
        emitter->Draw();
    }
}