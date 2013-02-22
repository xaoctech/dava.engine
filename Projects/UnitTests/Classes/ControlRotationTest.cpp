#include "ControlRotationTest.h"
#include "TextureUtils.h"


ControlRotationTest::ControlRotationTest()
: TestTemplate<ControlRotationTest>("ControlRotationTest")
{
    angle = 0.f;
    
    for(int32 i = 0; i < 100; ++i)
    {
        RegisterFunction(this, &ControlRotationTest::TestFunction, Format("Rotation_%d", i), NULL);
    }

}

void ControlRotationTest::LoadResources()
{
    GetBackground()->SetColor(Color(0.0f, 1.0f, 0.0f, 1.0f));
    GetBackground()->SetDrawType(UIControlBackground::DRAW_FILL);

    rotatedControl = new UIControl(Rect(256.f, 256.f, 256.f, 256.f));
    rotatedControl->SetSprite("~res:/Gfx/UI/Rotation", 0);
    rotatedControl->pivotPoint = rotatedControl->GetSize()/2;

    rotatedControl->SetAngle(angle);
    AddControl(rotatedControl);
    
    
    Font *font = FTFont::Create("~res:/Fonts/korinna.ttf");
    DVASSERT(font);
    
    font->SetSize(20);
    font->SetColor(Color::White());
    
    description = new UIStaticText(Rect(0, 256, 512, 200));
    description->SetAlign(ALIGN_LEFT | ALIGN_VCENTER);
    description->SetMultiline(true);
    description->SetFont(font);
    AddControl(description);
    
    SafeRelease(font);
}


void ControlRotationTest::UnloadResources()
{
    RemoveAllControls();

    SafeRelease(rotatedControl);
}

void ControlRotationTest::TestFunction(PerfFuncData * data)
{
    angle += (PI / 4);
    rotatedControl->SetAngle(angle);
    
    description->SetText(Format(L"Angle %f", angle));

    int64 a = 0;
    for (int32 i = 0; i < 100; ++i)
    {
        for (int32 j = 0; j < 1000000; ++j)
        {
            a += i+j;
        }
    }
}




