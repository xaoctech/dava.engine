#include "Infrastructure/TestBed.h"
#include "Tests/InputSystemTest.h"
#include "Engine/Engine.h"
#include "DeviceManager/DeviceManager.h"
#include "Utils/UTF8Utils.h"
#include "Input/InputListener.h"

using namespace DAVA;

static const DAVA::FastName ACTION_1 = DAVA::FastName("ACTION 1");
static const DAVA::FastName ACTION_2 = DAVA::FastName("ACTION 2");
static const DAVA::FastName ACTION_3 = DAVA::FastName("ACTION 3");
static const DAVA::FastName ACTION_4 = DAVA::FastName("ACTION 4");
static const DAVA::FastName ACTION_5 = DAVA::FastName("ACTION 5");

InputSystemTest::InputSystemTest(TestBed& app)
    : BaseScreen(app, "InputSystemTest")
{
}

void InputSystemTest::LoadResources()
{
    BaseScreen::LoadResources();

    // Create UI
    CreateKeyboardUI(L"Scancode keyboard", 20.0f, 20.0f, false);
    CreateKeyboardUI(L"Virtual key keyboard", 20.0f, 250.0f, true);
    CreateMouseUI();
    CreateActionsUI();
    CreateInputListenerUI();

    // Subscribe to events
    rawInputToken = GetEngineContext()->inputSystem->AddHandler(eInputDeviceTypes::CLASS_ALL, MakeFunction(this, &InputSystemTest::OnInputEvent));
    actionTriggeredToken = GetEngineContext()->actionSystem->ActionTriggered.Connect(this, &InputSystemTest::OnAction);

    // Bind action set

    ActionSet set;

    DigitalBinding action1;
    action1.actionId = ACTION_1;
    action1.requiredStates[0].elementId = eInputElements::KB_W_VIRTUAL;
    action1.requiredStates[0].stateMask = eDigitalElementStates::PRESSED;
    set.digitalBindings.push_back(action1);

    DigitalBinding action2;
    action2.actionId = ACTION_2;
    action2.requiredStates[0].elementId = eInputElements::KB_SPACE_VIRTUAL;
    action2.requiredStates[0].stateMask = eDigitalElementStates::JUST_PRESSED;
    set.digitalBindings.push_back(action2);

    DigitalBinding action3;
    action3.actionId = ACTION_3;
    action3.requiredStates[0].elementId = eInputElements::KB_SPACE_VIRTUAL;
    action3.requiredStates[0].stateMask = eDigitalElementStates::JUST_PRESSED;
    action3.requiredStates[1].elementId = eInputElements::KB_LSHIFT_VIRTUAL;
    action3.requiredStates[1].stateMask = eDigitalElementStates::PRESSED;
    set.digitalBindings.push_back(action3);

    AnalogBinding action4;
    action4.actionId = ACTION_4;
    action4.analogElementId = eInputElements::MOUSE_POSITION;
    set.analogBindings.push_back(action4);

    AnalogBinding action5;
    action5.actionId = ACTION_5;
    action5.analogElementId = eInputElements::MOUSE_POSITION;
    action5.requiredDigitalElementStates[0].elementId = eInputElements::MOUSE_LBUTTON;
    action5.requiredDigitalElementStates[0].stateMask = eDigitalElementStates::PRESSED;
    action5.requiredDigitalElementStates[1].elementId = eInputElements::KB_LCTRL_VIRTUAL;
    action5.requiredDigitalElementStates[1].stateMask = eDigitalElementStates::PRESSED;
    set.analogBindings.push_back(action5);

    GetEngineContext()->actionSystem->BindSet(set, 1, 2);
}

void InputSystemTest::UnloadResources()
{
    GetEngineContext()->inputSystem->RemoveHandler(rawInputToken);
    GetEngineContext()->actionSystem->ActionTriggered.Disconnect(actionTriggeredToken);

    for (auto it = keyboardButtons.begin(); it != keyboardButtons.end(); ++it)
    {
        SafeRelease(it->second);
    }

    for (auto it = keyboardsHeaders.begin(); it != keyboardsHeaders.end(); ++it)
    {
        SafeRelease(*it);
    }

    for (auto it = mouseButtons.begin(); it != mouseButtons.end(); ++it)
    {
        SafeRelease(it->second);
    }

    for (auto it = actionCounters.begin(); it != actionCounters.end(); ++it)
    {
        SafeRelease(it->second);
    }

    SafeRelease(inputListenerDigitalSingleWithoutModifiersButton);
    SafeRelease(inputListenerDigitalSingleWithModifiersButton);
    SafeRelease(inputListenerDigitalMultipleAnyButton);
    SafeRelease(inputListenerAnalogButton);
    SafeRelease(inputListenerResultField);

    // TODO: unbind action set

    BaseScreen::UnloadResources();
}

void InputSystemTest::CreateKeyboardUI(WideString header, float32 x, float32 y, bool forVirtualkeys)
{
    ScopedPtr<FTFont> font(FTFont::Create("~res:/Fonts/korinna.ttf"));

    const float32 keyboardButtonWidth = 25.0f;
    const float32 keyboardButtonHeight = 25.0f;

    const float32 headerHeight = 20.0f;

    font->SetSize(11);

    UIStaticText* headerText = new UIStaticText(Rect(x, y, 250, headerHeight));
    headerText->SetTextColor(Color::White);
    headerText->SetFont(font);
    headerText->SetTextAlign(ALIGN_LEFT | ALIGN_TOP);
    headerText->SetText(header);
    AddControl(headerText);

    keyboardsHeaders.push_back(headerText);

    y += headerHeight;

    const float32 initialX = x;
    const float32 initialY = y;

    float32 rightmostX = initialX;

    CreateKeyboardUIButton(GetVirtualOrScancodeInputElement(eInputElements::KB_ESCAPE, forVirtualkeys), L"ESC", font, &x, y, keyboardButtonWidth, keyboardButtonHeight);
    CreateKeyboardUIButton(GetVirtualOrScancodeInputElement(eInputElements::KB_F1, forVirtualkeys), L"F1", font, &x, y, keyboardButtonWidth, keyboardButtonHeight);
    CreateKeyboardUIButton(GetVirtualOrScancodeInputElement(eInputElements::KB_F2, forVirtualkeys), L"F2", font, &x, y, keyboardButtonWidth, keyboardButtonHeight);
    CreateKeyboardUIButton(GetVirtualOrScancodeInputElement(eInputElements::KB_F3, forVirtualkeys), L"F3", font, &x, y, keyboardButtonWidth, keyboardButtonHeight);
    CreateKeyboardUIButton(GetVirtualOrScancodeInputElement(eInputElements::KB_F4, forVirtualkeys), L"F4", font, &x, y, keyboardButtonWidth, keyboardButtonHeight);
    CreateKeyboardUIButton(GetVirtualOrScancodeInputElement(eInputElements::KB_F5, forVirtualkeys), L"F5", font, &x, y, keyboardButtonWidth, keyboardButtonHeight);
    CreateKeyboardUIButton(GetVirtualOrScancodeInputElement(eInputElements::KB_F6, forVirtualkeys), L"F6", font, &x, y, keyboardButtonWidth, keyboardButtonHeight);
    CreateKeyboardUIButton(GetVirtualOrScancodeInputElement(eInputElements::KB_F7, forVirtualkeys), L"F7", font, &x, y, keyboardButtonWidth, keyboardButtonHeight);
    CreateKeyboardUIButton(GetVirtualOrScancodeInputElement(eInputElements::KB_F8, forVirtualkeys), L"F8", font, &x, y, keyboardButtonWidth, keyboardButtonHeight);
    CreateKeyboardUIButton(GetVirtualOrScancodeInputElement(eInputElements::KB_F9, forVirtualkeys), L"F9", font, &x, y, keyboardButtonWidth, keyboardButtonHeight);
    CreateKeyboardUIButton(GetVirtualOrScancodeInputElement(eInputElements::KB_F10, forVirtualkeys), L"F10", font, &x, y, keyboardButtonWidth, keyboardButtonHeight);
    CreateKeyboardUIButton(GetVirtualOrScancodeInputElement(eInputElements::KB_F11, forVirtualkeys), L"F11", font, &x, y, keyboardButtonWidth, keyboardButtonHeight);
    CreateKeyboardUIButton(GetVirtualOrScancodeInputElement(eInputElements::KB_F12, forVirtualkeys), L"F12", font, &x, y, keyboardButtonWidth, keyboardButtonHeight);

    rightmostX = std::max(x, rightmostX);
    y += keyboardButtonHeight + 1.0f;
    x = initialX;

    CreateKeyboardUIButton(GetVirtualOrScancodeInputElement(eInputElements::KB_GRAVE, forVirtualkeys), L"`", font, &x, y, keyboardButtonWidth, keyboardButtonHeight);
    CreateKeyboardUIButton(GetVirtualOrScancodeInputElement(eInputElements::KB_1, forVirtualkeys), L"1", font, &x, y, keyboardButtonWidth, keyboardButtonHeight);
    CreateKeyboardUIButton(GetVirtualOrScancodeInputElement(eInputElements::KB_2, forVirtualkeys), L"2", font, &x, y, keyboardButtonWidth, keyboardButtonHeight);
    CreateKeyboardUIButton(GetVirtualOrScancodeInputElement(eInputElements::KB_3, forVirtualkeys), L"3", font, &x, y, keyboardButtonWidth, keyboardButtonHeight);
    CreateKeyboardUIButton(GetVirtualOrScancodeInputElement(eInputElements::KB_4, forVirtualkeys), L"4", font, &x, y, keyboardButtonWidth, keyboardButtonHeight);
    CreateKeyboardUIButton(GetVirtualOrScancodeInputElement(eInputElements::KB_5, forVirtualkeys), L"5", font, &x, y, keyboardButtonWidth, keyboardButtonHeight);
    CreateKeyboardUIButton(GetVirtualOrScancodeInputElement(eInputElements::KB_6, forVirtualkeys), L"6", font, &x, y, keyboardButtonWidth, keyboardButtonHeight);
    CreateKeyboardUIButton(GetVirtualOrScancodeInputElement(eInputElements::KB_7, forVirtualkeys), L"7", font, &x, y, keyboardButtonWidth, keyboardButtonHeight);
    CreateKeyboardUIButton(GetVirtualOrScancodeInputElement(eInputElements::KB_8, forVirtualkeys), L"8", font, &x, y, keyboardButtonWidth, keyboardButtonHeight);
    CreateKeyboardUIButton(GetVirtualOrScancodeInputElement(eInputElements::KB_9, forVirtualkeys), L"9", font, &x, y, keyboardButtonWidth, keyboardButtonHeight);
    CreateKeyboardUIButton(GetVirtualOrScancodeInputElement(eInputElements::KB_0, forVirtualkeys), L"0", font, &x, y, keyboardButtonWidth, keyboardButtonHeight);
    CreateKeyboardUIButton(GetVirtualOrScancodeInputElement(eInputElements::KB_MINUS, forVirtualkeys), L"-", font, &x, y, keyboardButtonWidth, keyboardButtonHeight);
    CreateKeyboardUIButton(GetVirtualOrScancodeInputElement(eInputElements::KB_EQUALS, forVirtualkeys), L"+", font, &x, y, keyboardButtonWidth, keyboardButtonHeight);
    CreateKeyboardUIButton(GetVirtualOrScancodeInputElement(eInputElements::KB_BACKSPACE, forVirtualkeys), L"<-", font, &x, y, keyboardButtonWidth, keyboardButtonHeight);

    rightmostX = std::max(x, rightmostX);
    y += keyboardButtonHeight + 1.0f;
    x = initialX;

    CreateKeyboardUIButton(GetVirtualOrScancodeInputElement(eInputElements::KB_TAB, forVirtualkeys), L"TAB", font, &x, y, keyboardButtonWidth, keyboardButtonHeight);
    CreateKeyboardUIButton(GetVirtualOrScancodeInputElement(eInputElements::KB_Q, forVirtualkeys), L"Q", font, &x, y, keyboardButtonWidth, keyboardButtonHeight);
    CreateKeyboardUIButton(GetVirtualOrScancodeInputElement(eInputElements::KB_W, forVirtualkeys), L"W", font, &x, y, keyboardButtonWidth, keyboardButtonHeight);
    CreateKeyboardUIButton(GetVirtualOrScancodeInputElement(eInputElements::KB_E, forVirtualkeys), L"E", font, &x, y, keyboardButtonWidth, keyboardButtonHeight);
    CreateKeyboardUIButton(GetVirtualOrScancodeInputElement(eInputElements::KB_R, forVirtualkeys), L"R", font, &x, y, keyboardButtonWidth, keyboardButtonHeight);
    CreateKeyboardUIButton(GetVirtualOrScancodeInputElement(eInputElements::KB_T, forVirtualkeys), L"T", font, &x, y, keyboardButtonWidth, keyboardButtonHeight);
    CreateKeyboardUIButton(GetVirtualOrScancodeInputElement(eInputElements::KB_Y, forVirtualkeys), L"Y", font, &x, y, keyboardButtonWidth, keyboardButtonHeight);
    CreateKeyboardUIButton(GetVirtualOrScancodeInputElement(eInputElements::KB_U, forVirtualkeys), L"U", font, &x, y, keyboardButtonWidth, keyboardButtonHeight);
    CreateKeyboardUIButton(GetVirtualOrScancodeInputElement(eInputElements::KB_I, forVirtualkeys), L"I", font, &x, y, keyboardButtonWidth, keyboardButtonHeight);
    CreateKeyboardUIButton(GetVirtualOrScancodeInputElement(eInputElements::KB_O, forVirtualkeys), L"O", font, &x, y, keyboardButtonWidth, keyboardButtonHeight);
    CreateKeyboardUIButton(GetVirtualOrScancodeInputElement(eInputElements::KB_P, forVirtualkeys), L"P", font, &x, y, keyboardButtonWidth, keyboardButtonHeight);
    CreateKeyboardUIButton(GetVirtualOrScancodeInputElement(eInputElements::KB_LBRACKET, forVirtualkeys), L"{", font, &x, y, keyboardButtonWidth, keyboardButtonHeight);
    CreateKeyboardUIButton(GetVirtualOrScancodeInputElement(eInputElements::KB_RBRACKET, forVirtualkeys), L"}", font, &x, y, keyboardButtonWidth, keyboardButtonHeight);
    CreateKeyboardUIButton(GetVirtualOrScancodeInputElement(eInputElements::KB_ENTER, forVirtualkeys), L"ENTER", font, &x, y, keyboardButtonWidth, keyboardButtonHeight);

    rightmostX = std::max(x, rightmostX);
    y += keyboardButtonHeight + 1.0f;
    x = initialX;

    CreateKeyboardUIButton(GetVirtualOrScancodeInputElement(eInputElements::KB_CAPSLOCK, forVirtualkeys), L"CAPS", font, &x, y, keyboardButtonWidth, keyboardButtonHeight);
    CreateKeyboardUIButton(GetVirtualOrScancodeInputElement(eInputElements::KB_A, forVirtualkeys), L"A", font, &x, y, keyboardButtonWidth, keyboardButtonHeight);
    CreateKeyboardUIButton(GetVirtualOrScancodeInputElement(eInputElements::KB_S, forVirtualkeys), L"S", font, &x, y, keyboardButtonWidth, keyboardButtonHeight);
    CreateKeyboardUIButton(GetVirtualOrScancodeInputElement(eInputElements::KB_D, forVirtualkeys), L"D", font, &x, y, keyboardButtonWidth, keyboardButtonHeight);
    CreateKeyboardUIButton(GetVirtualOrScancodeInputElement(eInputElements::KB_F, forVirtualkeys), L"F", font, &x, y, keyboardButtonWidth, keyboardButtonHeight);
    CreateKeyboardUIButton(GetVirtualOrScancodeInputElement(eInputElements::KB_G, forVirtualkeys), L"G", font, &x, y, keyboardButtonWidth, keyboardButtonHeight);
    CreateKeyboardUIButton(GetVirtualOrScancodeInputElement(eInputElements::KB_H, forVirtualkeys), L"H", font, &x, y, keyboardButtonWidth, keyboardButtonHeight);
    CreateKeyboardUIButton(GetVirtualOrScancodeInputElement(eInputElements::KB_J, forVirtualkeys), L"J", font, &x, y, keyboardButtonWidth, keyboardButtonHeight);
    CreateKeyboardUIButton(GetVirtualOrScancodeInputElement(eInputElements::KB_K, forVirtualkeys), L"K", font, &x, y, keyboardButtonWidth, keyboardButtonHeight);
    CreateKeyboardUIButton(GetVirtualOrScancodeInputElement(eInputElements::KB_L, forVirtualkeys), L"L", font, &x, y, keyboardButtonWidth, keyboardButtonHeight);
    CreateKeyboardUIButton(GetVirtualOrScancodeInputElement(eInputElements::KB_SEMICOLON, forVirtualkeys), L":", font, &x, y, keyboardButtonWidth, keyboardButtonHeight);
    CreateKeyboardUIButton(GetVirtualOrScancodeInputElement(eInputElements::KB_APOSTROPHE, forVirtualkeys), L"\"", font, &x, y, keyboardButtonWidth, keyboardButtonHeight);
    CreateKeyboardUIButton(GetVirtualOrScancodeInputElement(eInputElements::KB_BACKSLASH, forVirtualkeys), L"\\", font, &x, y, keyboardButtonWidth, keyboardButtonHeight);

    rightmostX = std::max(x, rightmostX);
    y += keyboardButtonHeight + 1.0f;
    x = initialX;

    CreateKeyboardUIButton(GetVirtualOrScancodeInputElement(eInputElements::KB_LSHIFT, forVirtualkeys), L"SHFT", font, &x, y, keyboardButtonWidth, keyboardButtonHeight);
    CreateKeyboardUIButton(GetVirtualOrScancodeInputElement(eInputElements::KB_NONUSBACKSLASH, forVirtualkeys), L"\\", font, &x, y, keyboardButtonWidth, keyboardButtonHeight);
    CreateKeyboardUIButton(GetVirtualOrScancodeInputElement(eInputElements::KB_Z, forVirtualkeys), L"Z", font, &x, y, keyboardButtonWidth, keyboardButtonHeight);
    CreateKeyboardUIButton(GetVirtualOrScancodeInputElement(eInputElements::KB_X, forVirtualkeys), L"X", font, &x, y, keyboardButtonWidth, keyboardButtonHeight);
    CreateKeyboardUIButton(GetVirtualOrScancodeInputElement(eInputElements::KB_C, forVirtualkeys), L"C", font, &x, y, keyboardButtonWidth, keyboardButtonHeight);
    CreateKeyboardUIButton(GetVirtualOrScancodeInputElement(eInputElements::KB_V, forVirtualkeys), L"V", font, &x, y, keyboardButtonWidth, keyboardButtonHeight);
    CreateKeyboardUIButton(GetVirtualOrScancodeInputElement(eInputElements::KB_B, forVirtualkeys), L"B", font, &x, y, keyboardButtonWidth, keyboardButtonHeight);
    CreateKeyboardUIButton(GetVirtualOrScancodeInputElement(eInputElements::KB_N, forVirtualkeys), L"N", font, &x, y, keyboardButtonWidth, keyboardButtonHeight);
    CreateKeyboardUIButton(GetVirtualOrScancodeInputElement(eInputElements::KB_M, forVirtualkeys), L"M", font, &x, y, keyboardButtonWidth, keyboardButtonHeight);
    CreateKeyboardUIButton(GetVirtualOrScancodeInputElement(eInputElements::KB_COMMA, forVirtualkeys), L"<", font, &x, y, keyboardButtonWidth, keyboardButtonHeight);
    CreateKeyboardUIButton(GetVirtualOrScancodeInputElement(eInputElements::KB_PERIOD, forVirtualkeys), L">", font, &x, y, keyboardButtonWidth, keyboardButtonHeight);
    CreateKeyboardUIButton(GetVirtualOrScancodeInputElement(eInputElements::KB_SLASH, forVirtualkeys), L"?", font, &x, y, keyboardButtonWidth, keyboardButtonHeight);
    CreateKeyboardUIButton(GetVirtualOrScancodeInputElement(eInputElements::KB_RSHIFT, forVirtualkeys), L"Shift", font, &x, y, keyboardButtonWidth, keyboardButtonHeight);

    rightmostX = std::max(x, rightmostX);
    y += keyboardButtonHeight + 1.0f;
    x = initialX;

    CreateKeyboardUIButton(GetVirtualOrScancodeInputElement(eInputElements::KB_LCTRL, forVirtualkeys), L"CTRL", font, &x, y, keyboardButtonWidth, keyboardButtonHeight);
    CreateKeyboardUIButton(GetVirtualOrScancodeInputElement(eInputElements::KB_LWIN, forVirtualkeys), L"WIN", font, &x, y, keyboardButtonWidth, keyboardButtonHeight);
    CreateKeyboardUIButton(GetVirtualOrScancodeInputElement(eInputElements::KB_LALT, forVirtualkeys), L"ALT", font, &x, y, keyboardButtonWidth, keyboardButtonHeight);
    CreateKeyboardUIButton(GetVirtualOrScancodeInputElement(eInputElements::KB_SPACE, forVirtualkeys), L"SPACE", font, &x, y, keyboardButtonWidth * 4, keyboardButtonHeight);
    CreateKeyboardUIButton(GetVirtualOrScancodeInputElement(eInputElements::KB_RALT, forVirtualkeys), L"ALT", font, &x, y, keyboardButtonWidth, keyboardButtonHeight);
    CreateKeyboardUIButton(GetVirtualOrScancodeInputElement(eInputElements::KB_RWIN, forVirtualkeys), L"WIN", font, &x, y, keyboardButtonWidth, keyboardButtonHeight);
    CreateKeyboardUIButton(GetVirtualOrScancodeInputElement(eInputElements::KB_MENU, forVirtualkeys), L"MENU", font, &x, y, keyboardButtonWidth, keyboardButtonHeight);
    CreateKeyboardUIButton(GetVirtualOrScancodeInputElement(eInputElements::KB_RCTRL, forVirtualkeys), L"CTRL", font, &x, y, keyboardButtonWidth, keyboardButtonHeight);

    const float32 middleSectionStartX = rightmostX + keyboardButtonWidth;

    y = initialY;
    x = middleSectionStartX;
    CreateKeyboardUIButton(GetVirtualOrScancodeInputElement(eInputElements::KB_PRINTSCREEN, forVirtualkeys), L"PSCR", font, &x, y, keyboardButtonWidth, keyboardButtonHeight);
    CreateKeyboardUIButton(GetVirtualOrScancodeInputElement(eInputElements::KB_SCROLLLOCK, forVirtualkeys), L"SCROLLLOCK", font, &x, y, keyboardButtonWidth, keyboardButtonHeight);
    CreateKeyboardUIButton(GetVirtualOrScancodeInputElement(eInputElements::KB_PAUSE, forVirtualkeys), L"PAUSE", font, &x, y, keyboardButtonWidth, keyboardButtonHeight);

    rightmostX = std::max(x, rightmostX);
    y += keyboardButtonHeight + 1.0f;
    x = middleSectionStartX;
    CreateKeyboardUIButton(GetVirtualOrScancodeInputElement(eInputElements::KB_INSERT, forVirtualkeys), L"INSERT", font, &x, y, keyboardButtonWidth, keyboardButtonHeight);
    CreateKeyboardUIButton(GetVirtualOrScancodeInputElement(eInputElements::KB_HOME, forVirtualkeys), L"HOME", font, &x, y, keyboardButtonWidth, keyboardButtonHeight);
    CreateKeyboardUIButton(GetVirtualOrScancodeInputElement(eInputElements::KB_PAGEUP, forVirtualkeys), L"PGUP", font, &x, y, keyboardButtonWidth, keyboardButtonHeight);

    rightmostX = std::max(x, rightmostX);
    y += keyboardButtonHeight + 1.0f;
    x = middleSectionStartX;
    CreateKeyboardUIButton(GetVirtualOrScancodeInputElement(eInputElements::KB_DELETE, forVirtualkeys), L"DEL", font, &x, y, keyboardButtonWidth, keyboardButtonHeight);
    CreateKeyboardUIButton(GetVirtualOrScancodeInputElement(eInputElements::KB_END, forVirtualkeys), L"END", font, &x, y, keyboardButtonWidth, keyboardButtonHeight);
    CreateKeyboardUIButton(GetVirtualOrScancodeInputElement(eInputElements::KB_PAGEDOWN, forVirtualkeys), L"PGDOWN", font, &x, y, keyboardButtonWidth, keyboardButtonHeight);

    rightmostX = std::max(x, rightmostX);
    y += 2.0f * (keyboardButtonHeight + 1.0f);
    x = middleSectionStartX + keyboardButtonWidth + 1.0f;
    CreateKeyboardUIButton(GetVirtualOrScancodeInputElement(eInputElements::KB_UP, forVirtualkeys), L"UP", font, &x, y, keyboardButtonWidth, keyboardButtonHeight);

    rightmostX = std::max(x, rightmostX);
    y += keyboardButtonHeight + 1.0f;
    x = middleSectionStartX;
    CreateKeyboardUIButton(GetVirtualOrScancodeInputElement(eInputElements::KB_LEFT, forVirtualkeys), L"LEFT", font, &x, y, keyboardButtonWidth, keyboardButtonHeight);
    CreateKeyboardUIButton(GetVirtualOrScancodeInputElement(eInputElements::KB_DOWN, forVirtualkeys), L"DOWN", font, &x, y, keyboardButtonWidth, keyboardButtonHeight);
    CreateKeyboardUIButton(GetVirtualOrScancodeInputElement(eInputElements::KB_RIGHT, forVirtualkeys), L"RIGHT", font, &x, y, keyboardButtonWidth, keyboardButtonHeight);

    const float32 numpadSectionStartX = rightmostX + keyboardButtonWidth;

    y = initialY;
    x = numpadSectionStartX;
    CreateKeyboardUIButton(GetVirtualOrScancodeInputElement(eInputElements::KB_NUMLOCK, forVirtualkeys), L"NUMLOCK", font, &x, y, keyboardButtonWidth, keyboardButtonHeight);
    CreateKeyboardUIButton(GetVirtualOrScancodeInputElement(eInputElements::KB_DIVIDE, forVirtualkeys), L"/", font, &x, y, keyboardButtonWidth, keyboardButtonHeight);
    CreateKeyboardUIButton(GetVirtualOrScancodeInputElement(eInputElements::KB_MULTIPLY, forVirtualkeys), L"*", font, &x, y, keyboardButtonWidth, keyboardButtonHeight);
    CreateKeyboardUIButton(GetVirtualOrScancodeInputElement(eInputElements::KB_NUMPAD_MINUS, forVirtualkeys), L"-", font, &x, y, keyboardButtonWidth, keyboardButtonHeight);

    rightmostX = std::max(x, rightmostX);
    y += keyboardButtonHeight + 1.0f;
    x = numpadSectionStartX;
    CreateKeyboardUIButton(GetVirtualOrScancodeInputElement(eInputElements::KB_NUMPAD_7, forVirtualkeys), L"7", font, &x, y, keyboardButtonWidth, keyboardButtonHeight);
    CreateKeyboardUIButton(GetVirtualOrScancodeInputElement(eInputElements::KB_NUMPAD_8, forVirtualkeys), L"8", font, &x, y, keyboardButtonWidth, keyboardButtonHeight);
    CreateKeyboardUIButton(GetVirtualOrScancodeInputElement(eInputElements::KB_NUMPAD_9, forVirtualkeys), L"9", font, &x, y, keyboardButtonWidth, keyboardButtonHeight);
    CreateKeyboardUIButton(GetVirtualOrScancodeInputElement(eInputElements::KB_NUMPAD_PLUS, forVirtualkeys), L"+", font, &x, y, keyboardButtonWidth, keyboardButtonHeight);

    rightmostX = std::max(x, rightmostX);
    y += keyboardButtonHeight + 1.0f;
    x = numpadSectionStartX;
    CreateKeyboardUIButton(GetVirtualOrScancodeInputElement(eInputElements::KB_NUMPAD_4, forVirtualkeys), L"4", font, &x, y, keyboardButtonWidth, keyboardButtonHeight);
    CreateKeyboardUIButton(GetVirtualOrScancodeInputElement(eInputElements::KB_NUMPAD_5, forVirtualkeys), L"5", font, &x, y, keyboardButtonWidth, keyboardButtonHeight);
    CreateKeyboardUIButton(GetVirtualOrScancodeInputElement(eInputElements::KB_NUMPAD_6, forVirtualkeys), L"6", font, &x, y, keyboardButtonWidth, keyboardButtonHeight);

    rightmostX = std::max(x, rightmostX);
    y += keyboardButtonHeight + 1.0f;
    x = numpadSectionStartX;
    CreateKeyboardUIButton(GetVirtualOrScancodeInputElement(eInputElements::KB_NUMPAD_1, forVirtualkeys), L"1", font, &x, y, keyboardButtonWidth, keyboardButtonHeight);
    CreateKeyboardUIButton(GetVirtualOrScancodeInputElement(eInputElements::KB_NUMPAD_2, forVirtualkeys), L"2", font, &x, y, keyboardButtonWidth, keyboardButtonHeight);
    CreateKeyboardUIButton(GetVirtualOrScancodeInputElement(eInputElements::KB_NUMPAD_3, forVirtualkeys), L"3", font, &x, y, keyboardButtonWidth, keyboardButtonHeight);
    CreateKeyboardUIButton(GetVirtualOrScancodeInputElement(eInputElements::KB_NUMPAD_ENTER, forVirtualkeys), L"ENTER", font, &x, y, keyboardButtonWidth, keyboardButtonHeight);

    rightmostX = std::max(x, rightmostX);
    y += keyboardButtonHeight + 1.0f;
    x = numpadSectionStartX;
    CreateKeyboardUIButton(GetVirtualOrScancodeInputElement(eInputElements::KB_NUMPAD_0, forVirtualkeys), L"0", font, &x, y, keyboardButtonWidth, keyboardButtonHeight);
    CreateKeyboardUIButton(GetVirtualOrScancodeInputElement(eInputElements::KB_NUMPAD_DELETE, forVirtualkeys), L"DEL", font, &x, y, keyboardButtonWidth, keyboardButtonHeight);
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

    float32 y = 450.0f;
    const float32 yDelta = 60.0f;

    //

    UIStaticText* staticText = new UIStaticText(Rect(10, y, 200, 30));
    staticText->SetTextColor(Color::White);
    staticText->SetFont(font);
    staticText->SetMultiline(true);
    staticText->SetTextAlign(ALIGN_LEFT | ALIGN_TOP);
    staticText->SetText(L"Action 1 (W pressed):");
    AddControl(staticText);

    staticText = new UIStaticText(Rect(215, y, 50, 30));
    staticText->SetTextColor(Color::White);
    staticText->SetFont(font);
    staticText->SetMultiline(true);
    staticText->SetTextAlign(ALIGN_HCENTER | ALIGN_TOP);
    staticText->SetText(L"0");
    AddControl(staticText);

    actionCounters[ACTION_1] = staticText;

    y += yDelta;

    //

    staticText = new UIStaticText(Rect(10, y, 200, 30));
    staticText->SetTextColor(Color::White);
    staticText->SetFont(font);
    staticText->SetMultiline(true);
    staticText->SetTextAlign(ALIGN_LEFT | ALIGN_TOP);
    staticText->SetText(L"Action 2 (Space just pressed):");
    AddControl(staticText);

    staticText = new UIStaticText(Rect(215, y, 50, 30));
    staticText->SetTextColor(Color::White);
    staticText->SetFont(font);
    staticText->SetMultiline(true);
    staticText->SetTextAlign(ALIGN_HCENTER | ALIGN_TOP);
    staticText->SetText(L"0");
    AddControl(staticText);

    actionCounters[ACTION_2] = staticText;

    y += yDelta;

    //

    staticText = new UIStaticText(Rect(10, y, 200, 30));
    staticText->SetTextColor(Color::White);
    staticText->SetFont(font);
    staticText->SetMultiline(true);
    staticText->SetTextAlign(ALIGN_LEFT | ALIGN_TOP);
    staticText->SetText(L"Action 3 (Left Shift pressed, Space just pressed):");
    AddControl(staticText);

    staticText = new UIStaticText(Rect(215, y, 50, 30));
    staticText->SetTextColor(Color::White);
    staticText->SetFont(font);
    staticText->SetMultiline(true);
    staticText->SetTextAlign(ALIGN_HCENTER | ALIGN_TOP);
    staticText->SetText(L"0");
    AddControl(staticText);

    actionCounters[ACTION_3] = staticText;

    y += yDelta;

    //

    staticText = new UIStaticText(Rect(10, y, 200, 30));
    staticText->SetTextColor(Color::White);
    staticText->SetFont(font);
    staticText->SetMultiline(true);
    staticText->SetTextAlign(ALIGN_LEFT | ALIGN_TOP);
    staticText->SetText(L"Action 4 (Mouse Cursor):");
    AddControl(staticText);

    staticText = new UIStaticText(Rect(215, y, 50, 30));
    staticText->SetTextColor(Color::White);
    staticText->SetFont(font);
    staticText->SetMultiline(true);
    staticText->SetTextAlign(ALIGN_HCENTER | ALIGN_TOP);
    staticText->SetText(L"0");
    AddControl(staticText);

    actionCounters[ACTION_4] = staticText;

    y += yDelta;

    //

    staticText = new UIStaticText(Rect(10, y, 200, 30));
    staticText->SetTextColor(Color::White);
    staticText->SetFont(font);
    staticText->SetMultiline(true);
    staticText->SetTextAlign(ALIGN_LEFT | ALIGN_TOP);
    staticText->SetText(L"Action 5 (Mouse Cursor, Left Ctrl pressed, Mouse Left Button pressed):");
    AddControl(staticText);

    staticText = new UIStaticText(Rect(215, y, 50, 30));
    staticText->SetTextColor(Color::White);
    staticText->SetFont(font);
    staticText->SetMultiline(true);
    staticText->SetTextAlign(ALIGN_HCENTER | ALIGN_TOP);
    staticText->SetText(L"0");
    AddControl(staticText);

    actionCounters[ACTION_5] = staticText;

    y += yDelta;

    //
}

void InputSystemTest::CreateInputListenerUI()
{
    ScopedPtr<FTFont> font(FTFont::Create("~res:/Fonts/korinna.ttf"));

    inputListenerDigitalSingleWithoutModifiersButton = new UIButton(Rect(680, 450, 250, 30));
    inputListenerDigitalSingleWithoutModifiersButton->SetStateFont(0xFF, font);
    inputListenerDigitalSingleWithoutModifiersButton->SetStateFontColor(0xFF, Color::White);
    inputListenerDigitalSingleWithoutModifiersButton->SetDebugDraw(true);
    inputListenerDigitalSingleWithoutModifiersButton->SetStateText(0xFF, L"Listen: digital single without modifiers");
    inputListenerDigitalSingleWithoutModifiersButton->AddEvent(UIButton::EVENT_TOUCH_UP_INSIDE, Message(this, &InputSystemTest::OnInputListenerButtonPressed));
    AddControl(inputListenerDigitalSingleWithoutModifiersButton);

    inputListenerDigitalSingleWithModifiersButton = new UIButton(Rect(680, 490, 250, 30));
    inputListenerDigitalSingleWithModifiersButton->SetStateFont(0xFF, font);
    inputListenerDigitalSingleWithModifiersButton->SetStateFontColor(0xFF, Color::White);
    inputListenerDigitalSingleWithModifiersButton->SetDebugDraw(true);
    inputListenerDigitalSingleWithModifiersButton->SetStateText(0xFF, L"Listen: digital single with modifiers");
    inputListenerDigitalSingleWithModifiersButton->AddEvent(UIButton::EVENT_TOUCH_UP_INSIDE, Message(this, &InputSystemTest::OnInputListenerButtonPressed));
    AddControl(inputListenerDigitalSingleWithModifiersButton);

    inputListenerDigitalMultipleAnyButton = new UIButton(Rect(680, 530, 250, 30));
    inputListenerDigitalMultipleAnyButton->SetStateFont(0xFF, font);
    inputListenerDigitalMultipleAnyButton->SetStateFontColor(0xFF, Color::White);
    inputListenerDigitalMultipleAnyButton->SetDebugDraw(true);
    inputListenerDigitalMultipleAnyButton->SetStateText(0xFF, L"Listen: digital multiple any");
    inputListenerDigitalMultipleAnyButton->AddEvent(UIButton::EVENT_TOUCH_UP_INSIDE, Message(this, &InputSystemTest::OnInputListenerButtonPressed));
    AddControl(inputListenerDigitalMultipleAnyButton);

    inputListenerAnalogButton = new UIButton(Rect(680, 570, 250, 30));
    inputListenerAnalogButton->SetStateFont(0xFF, font);
    inputListenerAnalogButton->SetStateFontColor(0xFF, Color::White);
    inputListenerAnalogButton->SetDebugDraw(true);
    inputListenerAnalogButton->SetStateText(0xFF, L"Listen: analog");
    inputListenerAnalogButton->AddEvent(UIButton::EVENT_TOUCH_UP_INSIDE, Message(this, &InputSystemTest::OnInputListenerButtonPressed));
    AddControl(inputListenerAnalogButton);

    inputListenerResultField = new UIStaticText(Rect(680, 610, 250, 30));
    inputListenerResultField->SetTextColor(Color::White);
    inputListenerResultField->SetFont(font);
    inputListenerResultField->SetMultiline(true);
    inputListenerResultField->SetTextAlign(ALIGN_HCENTER | ALIGN_VCENTER);
    inputListenerResultField->SetText(L"Listened input will be shown here");
    AddControl(inputListenerResultField);
}

void InputSystemTest::CreateKeyboardUIButton(eInputElements key, WideString text, FTFont* font, float32* x, float32 y, float32 w, float32 h)
{
    UIButton* button = new UIButton(Rect(*x, y, w, h));
    button->SetStateFont(0xFF, font);
    button->SetStateFontColor(0xFF, Color::White);
    button->SetStateText(0xFF, text);
    button->SetDebugDraw(true);

    keyboardButtons[static_cast<uint32>(key)] = button;
    AddControl(button);

    *x = *x + w + 1.0f;
}

eInputElements InputSystemTest::GetVirtualOrScancodeInputElement(eInputElements scancodeElement, bool convertToVirtualCounterpart)
{
    if (convertToVirtualCounterpart)
    {
        return static_cast<eInputElements>(scancodeElement - eInputElements::KB_COUNT_VIRTUAL);
    }
    else
    {
        return scancodeElement;
    }
}

void InputSystemTest::HighlightDigitalButton(DAVA::UIButton* button, DAVA::eDigitalElementStates state)
{
    if (button == nullptr)
    {
        return;
    }

    if ((state & eDigitalElementStates::PRESSED) != eDigitalElementStates::NONE)
    {
        button->SetDebugDrawColor(Color(0.0f, 1.0f, 0.0f, 1.0f));
    }
    else
    {
        button->SetDebugDrawColor(Color(1.0f, 0.0f, 0.0f, 1.0f));
    }
}

bool InputSystemTest::OnInputEvent(InputEvent const& event)
{
    if (event.deviceType == eInputDeviceTypes::KEYBOARD)
    {
        eInputElements scancodeCounterpart = GetEngineContext()->deviceManager->GetKeyboard()->ConvertVirtualToScancode(event.elementId);

        UIButton* virtualButton = keyboardButtons[event.elementId];
        UIButton* scancodeButton = keyboardButtons[scancodeCounterpart];

        HighlightDigitalButton(scancodeButton, event.digitalState);
        HighlightDigitalButton(virtualButton, event.digitalState);
    }
    else if (event.deviceType == eInputDeviceTypes::MOUSE)
    {
        UIButton* mouseButton = mouseButtons[event.elementId];

        if (event.elementId == eInputElements::MOUSE_POSITION)
        {
            std::wstringstream ss;
            ss << event.analogState.x << L", " << event.analogState.y;
            mouseButton->SetStateText(0xFF, ss.str());
        }
        else
        {
            HighlightDigitalButton(mouseButton, event.digitalState);
        }
    }

    InputListener* inputListener = GetEngineContext()->inputListener;
    if (event.elementId == eInputElements::KB_ESCAPE_VIRTUAL && inputListener->IsListening())
    {
        inputListener->StopListening();
        inputListenerResultField->SetText(L"Stopped listening");
    }

    return false;
}

void InputSystemTest::OnAction(DAVA::Action action)
{
    UIStaticText* staticTextCounter = actionCounters[action.actionId];
    int counter = std::atoi(UTF8Utils::EncodeToUTF8(staticTextCounter->GetText()).c_str()) + 1;
    staticTextCounter->SetText(UTF8Utils::EncodeToWideString(std::to_string(counter)));
}

void InputSystemTest::OnInputListenerButtonPressed(DAVA::BaseObject* sender, void* data, void* callerData)
{
    DAVA::eInputListenerModes mode;
    if (sender == inputListenerDigitalSingleWithoutModifiersButton)
    {
        mode = DAVA::eInputListenerModes::DIGITAL_SINGLE_WITHOUT_MODIFIERS;
    }
    else if (sender == inputListenerDigitalSingleWithModifiersButton)
    {
        mode = DAVA::eInputListenerModes::DIGITAL_SINGLE_WITH_MODIFIERS;
    }
    else if (sender == inputListenerDigitalMultipleAnyButton)
    {
        mode = DAVA::eInputListenerModes::DIGITAL_MULTIPLE_ANY;
    }
    else
    {
        mode = DAVA::eInputListenerModes::ANALOG;
    }

    GetEngineContext()->inputListener->Listen(mode, MakeFunction(this, &InputSystemTest::OnInputListeningEnded));
    inputListenerResultField->SetText(L"Listening...");
}

void InputSystemTest::OnInputListeningEnded(DAVA::Vector<DAVA::eInputElements> input)
{
    std::wstringstream ss;
    for (size_t i = 0; i < input.size(); ++i)
    {
        DAVA::InputElementInfo info = GetInputElementInfo(input[i]);
        ss << info.name.c_str();

        if (i != input.size() - 1)
        {
            ss << " + ";
        }
    }

    inputListenerResultField->SetText(ss.str());
}
