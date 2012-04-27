#include "ModificationsPanel.h"
#include "ControlsFactory.h"

#include "ModificationPopUp.h"
#include "../EditorScene.h"

static const WideString mods[3] = { L"M", L"R", L"S"};
static const WideString axises[3] = { L"X", L"Y", L"Z"};

ModificationsPanel::ModificationsPanel(ModificationsPanelDelegate *newDelegate, const Rect &rect, bool rectInAbsoluteCoordinates)
    :   UIControl(rect)
    ,   delegate(newDelegate)
    ,   workingScene(NULL)

{
    SetInputEnabled(false, false);
    
	modState = MOD_MOVE;
	modAxis = AXIS_X;
	
	modificationPanel = ControlsFactory::CreatePanelControl(Rect(0, 5, 160, 45));
    modificationPanel->GetBackground()->SetColor(Color(1.0f, 1.0f, 1.0f, 0.2f));
	
	for (int32 i = 0; i < 3; ++i)
	{
		btnMod[i] = ControlsFactory::CreateButton(Rect((BUTTON_W + BUTTON_B) * i, 0, BUTTON_W, BUTTON_W), mods[i]);
		btnMod[i]->AddEvent(UIControl::EVENT_TOUCH_UP_INSIDE, Message(this, &ModificationsPanel::OnModificationPressed));
		modificationPanel->AddControl(btnMod[i]);
        
		btnAxis[i] = ControlsFactory::CreateButton(Rect((BUTTON_W + BUTTON_B) * i, BUTTON_W + BUTTON_B, BUTTON_W, BUTTON_W), axises[i]);
		btnAxis[i]->AddEvent(UIControl::EVENT_TOUCH_UP_INSIDE, Message(this, &ModificationsPanel::OnModificationPressed));
		modificationPanel->AddControl(btnAxis[i]);
	}
	UIStaticText * st = new UIStaticText(Rect(55, 0, 70, BUTTON_W));
    st->SetFont(ControlsFactory::GetFontLight());
	st->SetText(L"w, e, r");
    modificationPanel->AddControl(st);
    
	st = new UIStaticText(Rect(55, BUTTON_W + BUTTON_B, 80, BUTTON_W));
    st->SetFont(ControlsFactory::GetFontLight());
	st->SetText(L"5, 6, 7, 8");
    modificationPanel->AddControl(st);
	
	btnPlaceOn = ControlsFactory::CreateButton(Rect(115, 0, BUTTON_W, BUTTON_W), L"P");
	btnPlaceOn->AddEvent(UIControl::EVENT_TOUCH_UP_INSIDE, Message(this, &ModificationsPanel::OnModificationPressed));
	modificationPanel->AddControl(btnPlaceOn);
	
	btnPopUp = ControlsFactory::CreateButton(Rect(140, 0, BUTTON_W, BUTTON_W), L"#");
	btnPopUp->AddEvent(UIControl::EVENT_TOUCH_UP_INSIDE, Message(this, &ModificationsPanel::OnModificationPopUpPressed));
	modificationPanel->AddControl(btnPopUp);
	
	modificationPopUp = new ModificationPopUp();
	
	btnModeSelection = ControlsFactory::CreateButton(Rect(170, 5, BUTTON_W, BUTTON_W), L"S");
	btnModeSelection->AddEvent(UIControl::EVENT_TOUCH_UP_INSIDE, Message(this, &ModificationsPanel::OnModePressed));
	AddControl(btnModeSelection);
    
	btnModeModification = ControlsFactory::CreateButton(Rect(195, 5, BUTTON_W, BUTTON_W), L"M");
	btnModeModification->AddEvent(UIControl::EVENT_TOUCH_UP_INSIDE, Message(this, &ModificationsPanel::OnModePressed));
	AddControl(btnModeModification);

	UpdateModState();
	OnModePressed(btnModeSelection, 0, 0);
}

ModificationsPanel::~ModificationsPanel()
{
    SafeRelease(workingScene);
    
	for (int32 i = 0; i < 3; ++i)
	{
		SafeRelease(btnMod[i]);
		SafeRelease(btnAxis[i]);
	}
	SafeRelease(modificationPanel);
    SafeRelease(modificationPopUp);
}

void ModificationsPanel::SetScene(EditorScene *scene)
{
    SafeRelease(workingScene);
    workingScene = SafeRetain(scene);
}


void ModificationsPanel::OnModePressed(BaseObject * object, void * userData, void * callerData)
{
	isModeModification = (object == btnModeModification);
	
	if (isModeModification)
	{
		btnModeModification->SetState(UIControl::STATE_SELECTED);
		btnModeSelection->SetState(UIControl::STATE_NORMAL);
	}
	else
	{
		btnModeModification->SetState(UIControl::STATE_NORMAL);
		btnModeSelection->SetState(UIControl::STATE_SELECTED);
	}
}

void ModificationsPanel::OnModificationPopUpPressed(BaseObject * object, void * userData, void * callerData)
{
	if(modificationPopUp->GetParent())
	{
		RemoveControl(modificationPopUp);
		modificationPopUp->SetSelection(0);
	}
	else
	{
		modificationPopUp->SetSelection(workingScene->GetProxy());
		AddControl(modificationPopUp);
	}
}

void ModificationsPanel::OnModificationPressed(BaseObject * object, void * userData, void * callerData)
{
	if (object == btnPlaceOn)
	{
        PlaceOnLandscape();
	}
    else 
    {
        for (int32 i = 0; i < 3; ++i)
        {
            if (object == btnMod[i])
            {
                modState = (eModState)i;
            }
            else if (object == btnAxis[i])
            {
                modAxis = (eModAxis)i;
            }
        }
        UpdateModState();
    }
}

void ModificationsPanel::PlaceOnLandscape()
{
    if(delegate)
    {
        delegate->OnPlaceOnLandscape();
    }
}


void ModificationsPanel::UpdateModState(void)
{
	for (int32 i = 0; i < 3; ++i)
	{
		btnMod[i]->SetState(UIControl::STATE_NORMAL);
		btnAxis[i]->SetState(UIControl::STATE_NORMAL);
	}
    
	btnMod[modState]->SetState(UIControl::STATE_SELECTED);

	switch (modAxis) 
	{
	case AXIS_X:
	case AXIS_Y:
	case AXIS_Z:
		btnAxis[modAxis]->SetState(UIControl::STATE_SELECTED);
		break;
	case AXIS_XY:
		btnAxis[AXIS_X]->SetState(UIControl::STATE_SELECTED);
		btnAxis[AXIS_Y]->SetState(UIControl::STATE_SELECTED);
		break;
	case AXIS_YZ:
		btnAxis[AXIS_Y]->SetState(UIControl::STATE_SELECTED);
		btnAxis[AXIS_Z]->SetState(UIControl::STATE_SELECTED);
		break;
	case AXIS_XZ:
		btnAxis[AXIS_X]->SetState(UIControl::STATE_SELECTED);
		btnAxis[AXIS_Z]->SetState(UIControl::STATE_SELECTED);
		break;
	default:
		break;
	}
}


void ModificationsPanel::Input(DAVA::UIEvent *event)
{    
    if (event->phase == UIEvent::PHASE_KEYCHAR)
    {
        UITextField *tf = dynamic_cast<UITextField *>(UIControlSystem::Instance()->GetFocusedControl());
        if(!tf)
        {
            switch(event->tid)
            {
                case DVKEY_Q:
                    modState = MOD_MOVE;
                    break;

                case DVKEY_E:
                    modState = MOD_ROTATE;
                    break;

                case DVKEY_R:
                    modState = MOD_SCALE;
                    break;

                case DVKEY_5:
                {
                    bool altIsPressed = InputSystem::Instance()->GetKeyboard()->IsKeyPressed(DVKEY_ALT);
                    if(!altIsPressed)
                    {
                        modAxis = AXIS_X;
                    }
                    break;
                }

                case DVKEY_6:
                {
                    bool altIsPressed = InputSystem::Instance()->GetKeyboard()->IsKeyPressed(DVKEY_ALT);
                    if(!altIsPressed)
                    {
                        modAxis = AXIS_Y;
                    }
                    break;
                }

                case DVKEY_7:
                {
                    bool altIsPressed = InputSystem::Instance()->GetKeyboard()->IsKeyPressed(DVKEY_ALT);
                    if(!altIsPressed)
                    {
                        modAxis = AXIS_Z;
                    }
                    break;
                }

                case DVKEY_8:
                {
                    bool altIsPressed = InputSystem::Instance()->GetKeyboard()->IsKeyPressed(DVKEY_ALT);
                    if(!altIsPressed)
                    {
                        if (modAxis < AXIS_XY) modAxis = AXIS_XY;
                        else modAxis = (eModAxis)(AXIS_XY + ((modAxis + 1 - AXIS_XY) % 3));
                    }
                    
                    break;
                }

                case DVKEY_P:
                {
					PlaceOnLandscape();
					break;
                }
					
					
                default:
                    break;
            }
            
			UpdateModState();
        }
	}
	
	UIControl::Input(event);
}

ModificationsPanel::eModState ModificationsPanel::GetModState()
{
    return modState;
}

ModificationsPanel::eModAxis ModificationsPanel::GetModAxis()
{
    return modAxis;
}

bool ModificationsPanel::IsModificationMode()
{
    return isModeModification;
}

void ModificationsPanel::IsModificationMode(bool value)
{
    isModeModification = value;
}

void ModificationsPanel::Update(float32 timeElapsed)
{
	SceneNode * selection = workingScene->GetProxy();
	modificationPopUp->SetSelection(selection);
	if (isModeModification && selection && modificationPanel->GetParent() == 0)
	{
		AddControl(modificationPanel);
	}
	else if ((selection == 0 && modificationPanel->GetParent() != 0) || !isModeModification)
	{
		RemoveControl(modificationPanel);
		modificationPopUp->SetSelection(0);
		if (modificationPopUp->GetParent())
        {
			modificationPopUp->GetParent()->RemoveControl(modificationPopUp);
        }
	}
	
    UIControl::Update(timeElapsed);
}

