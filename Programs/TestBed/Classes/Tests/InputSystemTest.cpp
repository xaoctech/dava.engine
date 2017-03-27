#include "Infrastructure/TestBed.h"
#include "Tests/InputSystemTest.h"
#include "Engine/Engine.h"
#include "DeviceManager/DeviceManager.h"
#include "Utils/UTF8Utils.h"

using namespace DAVA;

static const DAVA::FastName FIRE = DAVA::FastName("FIRE");
static const DAVA::FastName JUMP = DAVA::FastName("JUMP");
static const DAVA::FastName HIGH_JUMP = DAVA::FastName("HIGH_JUMP");
static const DAVA::FastName ROTATE_CAMERA = DAVA::FastName("ROTATE_CAMERA");
static const DAVA::FastName AIM = DAVA::FastName("AIM");

InputSystemTest::InputSystemTest(TestBed& app)
    : BaseScreen(app, "InputSystemTest")
{
}

void InputSystemTest::LoadResources()
{
    BaseScreen::LoadResources();

    CreateKeyboardUI();
    CreateMouseUI();
    CreateActionsUI();

    Engine::Instance()->beginFrame.Connect(this, &InputSystemTest::OnBeginFrame);
    GetEngineContext()->inputSystem->AddInputEventHandler(MakeFunction(this, &InputSystemTest::OnInputEvent));
    GetEngineContext()->actionSystem->ActionTriggered.Connect(this, &InputSystemTest::OnAction);

    ActionSet set;
    
    DigitalBinding fireBinding;
    fireBinding.actionId = FIRE;
    fireBinding.requiredStates[0].elementId = eInputElements::KB_KEY_J;
    fireBinding.requiredStates[0].stateMask = eDigitalElementState::PRESSED;
    set.digitalBindings.push_back(fireBinding);

    DigitalBinding jumpBinding;
    jumpBinding.actionId = JUMP;
    jumpBinding.requiredStates[0].elementId = eInputElements::KB_SPACE;
    jumpBinding.requiredStates[0].stateMask = eDigitalElementState::JUST_PRESSED;
    set.digitalBindings.push_back(jumpBinding);

    DigitalBinding highJumpBinding;
    highJumpBinding.actionId = HIGH_JUMP;
    highJumpBinding.requiredStates[0].elementId = eInputElements::KB_SPACE;
    highJumpBinding.requiredStates[0].stateMask = eDigitalElementState::JUST_PRESSED;
    highJumpBinding.requiredStates[1].elementId = eInputElements::KB_LSHIFT;
    highJumpBinding.requiredStates[1].stateMask = eDigitalElementState::PRESSED;
    set.digitalBindings.push_back(highJumpBinding);

    AnalogBinding rotateCameraBinding;
    rotateCameraBinding.actionId = ROTATE_CAMERA;
    rotateCameraBinding.analogElementId = eInputElements::MOUSE_POSITION;
    rotateCameraBinding.requiredDigitalElementStates[0].elementId = eInputElements::MOUSE_LBUTTON;;
    rotateCameraBinding.requiredDigitalElementStates[0].stateMask = eDigitalElementState::PRESSED;
    rotateCameraBinding.requiredDigitalElementStates[1].elementId = eInputElements::KB_LCTRL;
    rotateCameraBinding.requiredDigitalElementStates[1].stateMask = eDigitalElementState::PRESSED;
    set.analogBindings.push_back(rotateCameraBinding);

    AnalogBinding aimBinding;
    aimBinding.actionId = AIM;
    aimBinding.analogElementId = eInputElements::MOUSE_POSITION;
    set.analogBindings.push_back(aimBinding);

    GetEngineContext()->actionSystem->BindSet(set, 1, 2);
    GetEngineContext()->actionSystem->BindSet(set, 1, 2);
    GetEngineContext()->actionSystem->BindSet(set, 1, 2);

    /*
    AnalogBinding rotateCameraBinding;
    rotateCameraBinding.actionId = ROTATE_CAMERA;
    rotateCameraBinding.analogControlId = eInputControl::MOUSE_POSITION;
    rotateCameraBinding.deviceId = 2;
    rotateCameraBinding.requiredDigitalControlStates[0].deviceId = 2;
    rotateCameraBinding.requiredDigitalControlStates[0].controlId = eInputControl::MOUSE_LBUTTON;
    rotateCameraBinding.requiredDigitalControlStates[0].stateMask = eDigitalControlState::PRESSED;
    rotateCameraBinding.requiredDigitalControlStates[1].deviceId = 1;
    rotateCameraBinding.requiredDigitalControlStates[1].controlId = static_cast<uint32>(eInputControl::KB_LCTRL);
    rotateCameraBinding.requiredDigitalControlStates[1].stateMask = eDigitalControlState::PRESSED;
    GetEngineContext()->actionSystem->BindAnalogAction(rotateCameraBinding);

    AnalogBinding aimBinding;
    aimBinding.actionId = AIM;
    aimBinding.analogControlId = eInputControl::MOUSE_POSITION;
    aimBinding.deviceId = 2;
    GetEngineContext()->actionSystem->BindAnalogAction(aimBinding);*/
}

void InputSystemTest::UnloadResources()
{
    BaseScreen::UnloadResources();
}

bool InputSystemTest::OnInputEvent(InputEvent const& event)
{
    UIButton* button = nullptr;
    if (event.deviceType == KeyboardInputDevice::TYPE)
    {
        button = keyboardButtons[event.elementId];
    }
    else if (event.deviceType == MouseInputDevice::TYPE)
    {
        button = mouseButtons[event.elementId];
    }

    if (button != nullptr)
    {
        if (event.elementId == eInputElements::MOUSE_POSITION)
        {
            std::wstringstream ss;
            ss << event.analogState.x << L", " << event.analogState.y;
            button->SetStateText(0xFF, ss.str());
        }
        else
        {
            if ((event.digitalState & eDigitalElementState::PRESSED) != eDigitalElementState::NONE)
            {
                button->SetDebugDrawColor(Color(0.0f, 1.0f, 0.0f, 1.0f));
            }
            else
            {
                button->SetDebugDrawColor(Color(1.0f, 0.0f, 0.0f, 1.0f));
            }
        }
    }

    return false;
}

void InputSystemTest::OnBeginFrame()
{
}

UIButton* InputSystemTest::CreateKeyboardUIButton(eInputElements key, WideString text, FTFont* font, float32 x, float32 y, float32 w, float32 h)
{
    UIButton* button = new UIButton(Rect(x, y, w, h));
    button->SetStateFont(0xFF, font);
    button->SetStateFontColor(0xFF, Color::White);
    button->SetStateText(0xFF, text);
    button->SetDebugDraw(true);

    keyboardButtons[static_cast<uint32>(key)] = button;
    AddControl(button);

    return button;
}

void InputSystemTest::CreateKeyboardUI()
{
    ScopedPtr<FTFont> font(FTFont::Create("~res:/Fonts/korinna.ttf"));

    const float32 keyboardButtonWidth = 30.0f;
    const float32 keyboardButtonHeight = 30.0f;

    float32 y = 20.0f;
    float32 x = 0.0f;
    CreateKeyboardUIButton(eInputElements::KB_GRAVE, L"`", font, x += keyboardButtonWidth + 1, y, keyboardButtonWidth, keyboardButtonHeight);
    CreateKeyboardUIButton(eInputElements::KB_KEY_1, L"1", font, x += keyboardButtonWidth + 1, y, keyboardButtonWidth, keyboardButtonHeight);
    CreateKeyboardUIButton(eInputElements::KB_KEY_2, L"2", font, x += keyboardButtonWidth + 1, y, keyboardButtonWidth, keyboardButtonHeight);
    CreateKeyboardUIButton(eInputElements::KB_KEY_3, L"3", font, x += keyboardButtonWidth + 1, y, keyboardButtonWidth, keyboardButtonHeight);
    CreateKeyboardUIButton(eInputElements::KB_KEY_4, L"4", font, x += keyboardButtonWidth + 1, y, keyboardButtonWidth, keyboardButtonHeight);
    CreateKeyboardUIButton(eInputElements::KB_KEY_5, L"5", font, x += keyboardButtonWidth + 1, y, keyboardButtonWidth, keyboardButtonHeight);
    CreateKeyboardUIButton(eInputElements::KB_KEY_6, L"6", font, x += keyboardButtonWidth + 1, y, keyboardButtonWidth, keyboardButtonHeight);
    CreateKeyboardUIButton(eInputElements::KB_KEY_7, L"7", font, x += keyboardButtonWidth + 1, y, keyboardButtonWidth, keyboardButtonHeight);
    CreateKeyboardUIButton(eInputElements::KB_KEY_8, L"8", font, x += keyboardButtonWidth + 1, y, keyboardButtonWidth, keyboardButtonHeight);
    CreateKeyboardUIButton(eInputElements::KB_KEY_9, L"9", font, x += keyboardButtonWidth + 1, y, keyboardButtonWidth, keyboardButtonHeight);
    CreateKeyboardUIButton(eInputElements::KB_KEY_0, L"0", font, x += keyboardButtonWidth + 1, y, keyboardButtonWidth, keyboardButtonHeight);
    CreateKeyboardUIButton(eInputElements::KB_MINUS, L"-", font, x += keyboardButtonWidth + 1, y, keyboardButtonWidth, keyboardButtonHeight);
    CreateKeyboardUIButton(eInputElements::KB_EQUALS, L"+", font, x += keyboardButtonWidth + 1, y, keyboardButtonWidth, keyboardButtonHeight);
    CreateKeyboardUIButton(eInputElements::KB_BACKSPACE, L"<-", font, x += keyboardButtonWidth + 1, y, keyboardButtonWidth, keyboardButtonHeight);

    y = 50.0f;
    x = 0.0f;
    CreateKeyboardUIButton(eInputElements::KB_TAB, L"TAB", font, x += keyboardButtonWidth + 1, y, keyboardButtonWidth, keyboardButtonHeight);
    CreateKeyboardUIButton(eInputElements::KB_KEY_Q, L"Q", font, x += keyboardButtonWidth + 1, y, keyboardButtonWidth, keyboardButtonHeight);
    CreateKeyboardUIButton(eInputElements::KB_KEY_W, L"W", font, x += keyboardButtonWidth + 1, y, keyboardButtonWidth, keyboardButtonHeight);
    CreateKeyboardUIButton(eInputElements::KB_KEY_E, L"E", font, x += keyboardButtonWidth + 1, y, keyboardButtonWidth, keyboardButtonHeight);
    CreateKeyboardUIButton(eInputElements::KB_KEY_R, L"R", font, x += keyboardButtonWidth + 1, y, keyboardButtonWidth, keyboardButtonHeight);
    CreateKeyboardUIButton(eInputElements::KB_KEY_T, L"T", font, x += keyboardButtonWidth + 1, y, keyboardButtonWidth, keyboardButtonHeight);
    CreateKeyboardUIButton(eInputElements::KB_KEY_Y, L"Y", font, x += keyboardButtonWidth + 1, y, keyboardButtonWidth, keyboardButtonHeight);
    CreateKeyboardUIButton(eInputElements::KB_KEY_U, L"U", font, x += keyboardButtonWidth + 1, y, keyboardButtonWidth, keyboardButtonHeight);
    CreateKeyboardUIButton(eInputElements::KB_KEY_I, L"I", font, x += keyboardButtonWidth + 1, y, keyboardButtonWidth, keyboardButtonHeight);
    CreateKeyboardUIButton(eInputElements::KB_KEY_O, L"O", font, x += keyboardButtonWidth + 1, y, keyboardButtonWidth, keyboardButtonHeight);
    CreateKeyboardUIButton(eInputElements::KB_KEY_P, L"P", font, x += keyboardButtonWidth + 1, y, keyboardButtonWidth, keyboardButtonHeight);
    CreateKeyboardUIButton(eInputElements::KB_LBRACKET, L"{", font, x += keyboardButtonWidth + 1, y, keyboardButtonWidth, keyboardButtonHeight);
    CreateKeyboardUIButton(eInputElements::KB_RBRACKET, L"}", font, x += keyboardButtonWidth + 1, y, keyboardButtonWidth, keyboardButtonHeight);

    y = 80.0f;
    x = 0.0f;
    CreateKeyboardUIButton(eInputElements::KB_CAPSLOCK, L"CAPS", font, x += keyboardButtonWidth + 1, y, keyboardButtonWidth, keyboardButtonHeight);
    CreateKeyboardUIButton(eInputElements::KB_KEY_A, L"A", font, x += keyboardButtonWidth + 1, y, keyboardButtonWidth, keyboardButtonHeight);
    CreateKeyboardUIButton(eInputElements::KB_KEY_S, L"S", font, x += keyboardButtonWidth + 1, y, keyboardButtonWidth, keyboardButtonHeight);
    CreateKeyboardUIButton(eInputElements::KB_KEY_D, L"D", font, x += keyboardButtonWidth + 1, y, keyboardButtonWidth, keyboardButtonHeight);
    CreateKeyboardUIButton(eInputElements::KB_KEY_F, L"F", font, x += keyboardButtonWidth + 1, y, keyboardButtonWidth, keyboardButtonHeight);
    CreateKeyboardUIButton(eInputElements::KB_KEY_G, L"G", font, x += keyboardButtonWidth + 1, y, keyboardButtonWidth, keyboardButtonHeight);
    CreateKeyboardUIButton(eInputElements::KB_KEY_H, L"H", font, x += keyboardButtonWidth + 1, y, keyboardButtonWidth, keyboardButtonHeight);
    CreateKeyboardUIButton(eInputElements::KB_KEY_J, L"J", font, x += keyboardButtonWidth + 1, y, keyboardButtonWidth, keyboardButtonHeight);
    CreateKeyboardUIButton(eInputElements::KB_KEY_K, L"K", font, x += keyboardButtonWidth + 1, y, keyboardButtonWidth, keyboardButtonHeight);
    CreateKeyboardUIButton(eInputElements::KB_KEY_L, L"L", font, x += keyboardButtonWidth + 1, y, keyboardButtonWidth, keyboardButtonHeight);
    CreateKeyboardUIButton(eInputElements::KB_SEMICOLON, L":", font, x += keyboardButtonWidth + 1, y, keyboardButtonWidth, keyboardButtonHeight);
    CreateKeyboardUIButton(eInputElements::KB_APOSTROPHE, L"\"", font, x += keyboardButtonWidth + 1, y, keyboardButtonWidth, keyboardButtonHeight);
    CreateKeyboardUIButton(eInputElements::KB_BACKSLASH, L"\\", font, x += keyboardButtonWidth + 1, y, keyboardButtonWidth, keyboardButtonHeight);

    y = 110.0f;
    x = 0.0f;
    CreateKeyboardUIButton(eInputElements::KB_LSHIFT, L"SHFT", font, x += keyboardButtonWidth + 1, y, keyboardButtonWidth, keyboardButtonHeight);
    CreateKeyboardUIButton(eInputElements::KB_NONUSBACKSLASH, L"\\", font, x += keyboardButtonWidth + 1, y, keyboardButtonWidth, keyboardButtonHeight);
    CreateKeyboardUIButton(eInputElements::KB_KEY_Z, L"Z", font, x += keyboardButtonWidth + 1, y, keyboardButtonWidth, keyboardButtonHeight);
    CreateKeyboardUIButton(eInputElements::KB_KEY_X, L"X", font, x += keyboardButtonWidth + 1, y, keyboardButtonWidth, keyboardButtonHeight);
    CreateKeyboardUIButton(eInputElements::KB_KEY_C, L"C", font, x += keyboardButtonWidth + 1, y, keyboardButtonWidth, keyboardButtonHeight);
    CreateKeyboardUIButton(eInputElements::KB_KEY_V, L"V", font, x += keyboardButtonWidth + 1, y, keyboardButtonWidth, keyboardButtonHeight);
    CreateKeyboardUIButton(eInputElements::KB_KEY_B, L"B", font, x += keyboardButtonWidth + 1, y, keyboardButtonWidth, keyboardButtonHeight);
    CreateKeyboardUIButton(eInputElements::KB_KEY_N, L"N", font, x += keyboardButtonWidth + 1, y, keyboardButtonWidth, keyboardButtonHeight);
    CreateKeyboardUIButton(eInputElements::KB_KEY_M, L"M", font, x += keyboardButtonWidth + 1, y, keyboardButtonWidth, keyboardButtonHeight);
    CreateKeyboardUIButton(eInputElements::KB_COMMA, L"<", font, x += keyboardButtonWidth + 1, y, keyboardButtonWidth, keyboardButtonHeight);
    CreateKeyboardUIButton(eInputElements::KB_PERIOD, L">", font, x += keyboardButtonWidth + 1, y, keyboardButtonWidth, keyboardButtonHeight);
    CreateKeyboardUIButton(eInputElements::KB_SLASH, L"?", font, x += keyboardButtonWidth + 1, y, keyboardButtonWidth, keyboardButtonHeight);
}

void InputSystemTest::CreateMouseUI()
{
    ScopedPtr<FTFont> font(FTFont::Create("~res:/Fonts/korinna.ttf"));

    UIButton* mouse = new UIButton(Rect(680, 10, 104, 140));
    mouse->SetStateFont(0xFF, font);
    mouse->SetStateFontColor(0xFF, Color::White);
    mouse->SetDebugDraw(true);
    AddControl(mouse);

    UIButton* mousePositionButton = new UIButton(Rect(680, 160, 104, 15));
    mousePositionButton->SetStateFont(0xFF, font);
    mousePositionButton->SetStateFontColor(0xFF, Color::White);
    mousePositionButton->SetDebugDraw(true);
    mouseButtons[static_cast<uint32>(eInputElements::MOUSE_POSITION)] = mousePositionButton;

    AnalogElementState mousePos = GetEngineContext()->deviceManager->GetMouse()->GetAnalogElementState(eInputElements::MOUSE_POSITION);
    std::wstringstream ss;
    ss << mousePos.x << L", " << mousePos.y;
    mousePositionButton->SetStateText(0xFF, ss.str());

    AddControl(mousePositionButton);

    UIButton* leftButton = new UIButton(Rect(700, 10, 30, 70));
    leftButton->SetStateFont(0xFF, font);
    leftButton->SetStateFontColor(0xFF, Color::White);
    leftButton->SetDebugDraw(true);
    mouseButtons[static_cast<uint32>(eInputElements::MOUSE_LBUTTON)] = leftButton;
    AddControl(leftButton);

    UIButton* rightButton = new UIButton(Rect(732, 10, 30, 70));
    rightButton->SetStateFont(0xFF, font);
    rightButton->SetStateFontColor(0xFF, Color::White);
    rightButton->SetDebugDraw(true);
    mouseButtons[static_cast<uint32>(eInputElements::MOUSE_RBUTTON)] = rightButton;
    AddControl(rightButton);
}

void InputSystemTest::CreateActionsUI()
{
    ScopedPtr<FTFont> font(FTFont::Create("~res:/Fonts/korinna.ttf"));

    //

    UIStaticText* staticText = new UIStaticText(Rect(10, 700, 100, 30));
    staticText->SetTextColor(Color::White);
    staticText->SetFont(font);
    staticText->SetMultiline(true);
    staticText->SetTextAlign(ALIGN_HCENTER | ALIGN_TOP);
    staticText->SetText(L"FIRE");
    AddControl(staticText);

    staticText = new UIStaticText(Rect(115, 700, 50, 30));
    staticText->SetTextColor(Color::White);
    staticText->SetFont(font);
    staticText->SetMultiline(true);
    staticText->SetTextAlign(ALIGN_HCENTER | ALIGN_TOP);
    staticText->SetText(L"0");
    AddControl(staticText);

    actionCounters[FIRE] = staticText;

    //

    staticText = new UIStaticText(Rect(130, 700, 100, 30));
    staticText->SetTextColor(Color::White);
    staticText->SetFont(font);
    staticText->SetMultiline(true);
    staticText->SetTextAlign(ALIGN_HCENTER | ALIGN_TOP);
    staticText->SetText(L"JUMP");
    AddControl(staticText);

    staticText = new UIStaticText(Rect(235, 700, 50, 30));
    staticText->SetTextColor(Color::White);
    staticText->SetFont(font);
    staticText->SetMultiline(true);
    staticText->SetTextAlign(ALIGN_HCENTER | ALIGN_TOP);
    staticText->SetText(L"0");
    AddControl(staticText);

    actionCounters[JUMP] = staticText;

    //

    staticText = new UIStaticText(Rect(350, 700, 100, 30));
    staticText->SetTextColor(Color::White);
    staticText->SetFont(font);
    staticText->SetMultiline(true);
    staticText->SetTextAlign(ALIGN_HCENTER | ALIGN_TOP);
    staticText->SetText(L"HIGH JUMP");
    AddControl(staticText);

    staticText = new UIStaticText(Rect(455, 700, 50, 30));
    staticText->SetTextColor(Color::White);
    staticText->SetFont(font);
    staticText->SetMultiline(true);
    staticText->SetTextAlign(ALIGN_HCENTER | ALIGN_TOP);
    staticText->SetText(L"0");
    AddControl(staticText);

    actionCounters[HIGH_JUMP] = staticText;

    //

    staticText = new UIStaticText(Rect(580, 700, 100, 30));
    staticText->SetTextColor(Color::White);
    staticText->SetFont(font);
    staticText->SetMultiline(true);
    staticText->SetTextAlign(ALIGN_HCENTER | ALIGN_TOP);
    staticText->SetText(L"ROTATE");
    AddControl(staticText);

    staticText = new UIStaticText(Rect(685, 700, 50, 30));
    staticText->SetTextColor(Color::White);
    staticText->SetFont(font);
    staticText->SetMultiline(true);
    staticText->SetTextAlign(ALIGN_HCENTER | ALIGN_TOP);
    staticText->SetText(L"0");
    AddControl(staticText);

    actionCounters[ROTATE_CAMERA] = staticText;

    //

    staticText = new UIStaticText(Rect(10, 580, 100, 30));
    staticText->SetTextColor(Color::White);
    staticText->SetFont(font);
    staticText->SetMultiline(true);
    staticText->SetTextAlign(ALIGN_HCENTER | ALIGN_TOP);
    staticText->SetText(L"AIM");
    AddControl(staticText);

    staticText = new UIStaticText(Rect(115, 580, 50, 30));
    staticText->SetTextColor(Color::White);
    staticText->SetFont(font);
    staticText->SetMultiline(true);
    staticText->SetTextAlign(ALIGN_HCENTER | ALIGN_TOP);
    staticText->SetText(L"0");
    AddControl(staticText);

    actionCounters[AIM] = staticText;

    //
}

void InputSystemTest::OnAction(DAVA::Action action)
{
    UIStaticText* staticTextCounter = actionCounters[action.actionId];
    int counter = std::atoi(UTF8Utils::EncodeToUTF8(staticTextCounter->GetText()).c_str()) + 1;
    staticTextCounter->SetText(UTF8Utils::EncodeToWideString(std::to_string(counter)));
}