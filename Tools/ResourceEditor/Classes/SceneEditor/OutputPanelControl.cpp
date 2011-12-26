#include "OutputPanelControl.h"

#include "ControlsFactory.h"

#include "OutputControl.h"
#include "CameraOutputControl.h"
#include "EditorScene.h"

#include "OutputManager.h"

OutputPanelControl::OutputPanelControl(EditorScene *scene, const Rect & rect)
    :   UIControl(rect)
{
    ControlsFactory::CusomizeBottomLevelControl(this);
    
    CreateButtons();
    
    Rect r = Rect(0, ControlsFactory::BUTTON_HEIGHT, rect.dx, rect.dy - ControlsFactory::BUTTON_HEIGHT);
    logControl = new OutputControl(r);
    AddControl(logControl);
    
    cameraLog = new CameraOutputControl(scene, r);
}


OutputPanelControl::~OutputPanelControl()
{
    SafeRelease(cameraLog);
    SafeRelease(logControl);
    SafeRelease(cameraLog);
    
    ReleaseButtons();
}

void OutputPanelControl::WillAppear()
{
    if(logControl->GetParent())
    {
        OutputManager::Instance()->SetActiveOutput(logControl);
    }
}

void OutputPanelControl::Update(float32 timeElapsed)
{
    UIControl::Update(timeElapsed);
}

void OutputPanelControl::CreateButtons()
{
    Vector2 pos(0,0);
    int32 dx = ControlsFactory::BUTTON_WIDTH;
    btnOutput = ControlsFactory::CreateButton(pos, L"Output");
    pos.x += dx + 1;
    btnCamera = ControlsFactory::CreateButton(pos, L"Camera");
    pos.x += dx + 1;
    
    btnClear = ControlsFactory::CreateButton(Vector2(GetRect().dx - ControlsFactory::BUTTON_WIDTH, 0), L"Clear");

    AddControl(btnOutput);
    AddControl(btnCamera);
    AddControl(btnClear);

    btnOutput->AddEvent(UIControl::EVENT_TOUCH_UP_INSIDE, Message(this, &OutputPanelControl::OnOutputPressed));
    btnCamera->AddEvent(UIControl::EVENT_TOUCH_UP_INSIDE, Message(this, &OutputPanelControl::OnCameraPressed));
    btnClear->AddEvent(UIControl::EVENT_TOUCH_UP_INSIDE, Message(this, &OutputPanelControl::OnClearPressed));
}

void OutputPanelControl::ReleaseButtons()
{
    SafeRelease(btnOutput);
    SafeRelease(btnCamera);
    SafeRelease(btnClear);
}

void OutputPanelControl::OnClearPressed(DAVA::BaseObject *obj, void *, void *)
{
    if(logControl->GetParent())
    {
        logControl->Clear();
    }
    else if(cameraLog->GetParent())
    {
    }
}


void OutputPanelControl::OnCameraPressed(DAVA::BaseObject *obj, void *, void *)
{
    if(logControl->GetParent())
    {
        RemoveControl(logControl);
    }
    
    if(btnClear->GetParent())
    {
        RemoveControl(btnClear);
    }
    
    if(!cameraLog->GetParent())
    {
        AddControl(cameraLog);
    }
}

void OutputPanelControl::OnOutputPressed(DAVA::BaseObject *obj, void *, void *)
{
    if(cameraLog->GetParent())
    {
        RemoveControl(cameraLog);
    }
    if(!logControl->GetParent())
    {
        AddControl(logControl);
    }
    
    if(!btnClear->GetParent())
    {
        AddControl(btnClear);
    }
}

void OutputPanelControl::SetRect(const Rect &rect, bool rectInAbsoluteCoordinates)
{
    UIControl::SetRect(rect, rectInAbsoluteCoordinates);
   
    Rect r = logControl->GetRect();
    r.dx = rect.dx;
    logControl->SetRect(r);

    r = cameraLog->GetRect();
    r.dx = rect.dx;
    cameraLog->SetRect(r);
    
    r = btnClear->GetRect();
    r.x = rect.dx - ControlsFactory::BUTTON_WIDTH;
    btnClear->SetRect(r);
}

void OutputPanelControl::UpdateCamera()
{
    if(cameraLog->GetParent())
    {
        cameraLog->ReadCamera(true);
    }
}