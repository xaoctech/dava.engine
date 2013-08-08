/*==================================================================================
    Copyright (c) 2008, DAVA, INC
    All rights reserved.

    Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:
    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the DAVA, INC nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE DAVA, INC AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL DAVA, INC BE LIABLE FOR ANY
    DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
=====================================================================================*/

#include "AlignTest.h"
#include "Render/RenderManager.h"

static const float ALIGN_TEST_AUTO_CLOSE_TIME = 30.0f;
static const WideString controlText = L"THIS SOFTWARE IS PROVIDED BY THE DAVA, INC AND CONTRIBUTORS 'AS IS' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED";

const int AlignTest::alignTypesData[] =
{
    ALIGN_LEFT | ALIGN_TOP,
    ALIGN_LEFT | ALIGN_VCENTER,
    ALIGN_LEFT | ALIGN_BOTTOM,
	ALIGN_HCENTER | ALIGN_TOP,
    ALIGN_HCENTER | ALIGN_VCENTER,
    ALIGN_HCENTER | ALIGN_BOTTOM,
    ALIGN_RIGHT | ALIGN_TOP,
    ALIGN_RIGHT | ALIGN_VCENTER,
    ALIGN_RIGHT | ALIGN_BOTTOM,    
    ALIGN_HJUSTIFY
};

AlignTest::AlignTest():
TestTemplate<AlignTest>("SplitTest")
{	
	currentAlignIndex = 0;
	
	RegisterFunction(this, &AlignTest::MultilineEnable, Format("MultilineTest"), NULL);
	RegisterFunction(this, &AlignTest::ResizeControl, Format("ResizeTest"), NULL);
	RegisterFunction(this, &AlignTest::MoveControl, Format("MultilineTest"), NULL);
	// Register align function for each align option
	for (int32 i = 0; i <= GetAlignTypesCount(); ++i)
	{
		RegisterFunction(this, &AlignTest::AlignText, Format("AlignTest"), NULL);
	}
}

void AlignTest::LoadResources()
{
    font = FTFont::Create("~res:/Fonts/korinna.ttf");		
  
    staticText = new UIStaticText();
    staticText->SetRect(Rect(10.f, 10.f, 400.f, 200.f));
	staticText->SetTextColor(Color::White());
	staticText->SetDebugDraw(true);    
    staticText->SetFont(font);
	staticText->SetText(controlText);
	AddControl(staticText);	

    staticText2 = new UIStaticText();
    staticText2->SetRect(Rect(550.f, 10.f, 200.f, 100.f));
	staticText2->SetTextColor(Color::White());
	staticText2->SetDebugDraw(true);
    staticText2->SetFont(font);
	staticText2->SetText(controlText);
	AddControl(staticText2);
}

void AlignTest::UnloadResources()
{
    SafeRelease(staticText);
    SafeRelease(staticText2);
    SafeRelease(font);
}

void AlignTest::MultilineEnable(PerfFuncData * data)
{
	staticText->SetMultiline(true);
   	staticText2->SetMultiline(true);
}

void AlignTest::ResizeControl(PerfFuncData * data)
{
	Rect rect = staticText->GetRect();
	rect.dx = 500.f;
	rect.dy = 100.f;
	staticText->SetRect(rect);
		
	rect = staticText2->GetRect();
	rect.dx = 300.f;
	rect.dy = 150.f;
	staticText2->SetRect(rect);
}

void AlignTest::MoveControl(PerfFuncData * data)
{
	Rect rect = staticText->GetRect();
	rect.x = 50.f;
	rect.y = 200.f;
	staticText->SetRect(rect);
		
	rect = staticText2->GetRect();
	rect.x = 10.f;
	staticText2->SetRect(rect);
}

void AlignTest::AlignText(PerfFuncData * data)
{
	staticText->SetTextAlign(GetAlignType(currentAlignIndex));
	staticText2->SetTextAlign(GetAlignType(currentAlignIndex));
		
	currentAlignIndex++;
}

int AlignTest::GetAlignTypesCount()
{
	return sizeof(alignTypesData)/sizeof(*alignTypesData);
}

int AlignTest::GetAlignType(int index)
{
    return alignTypesData[index];
}
