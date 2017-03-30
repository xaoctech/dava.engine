#include "Infrastructure/TestBed.h"
#include "Tests/InputSystemTest.h"
#include "Engine/Engine.h"
#include "DeviceManager/DeviceManager.h"
#include "Utils/UTF8Utils.h"

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

    SafeRelease(inputListenerStartButton);
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

    CreateKeyboardUIButton(GetVirtualOrScancodeInputElement(eInputElements::KB_ESCAPE_SCANCODE, forVirtualkeys), L"ESC", font, &x, y, keyboardButtonWidth, keyboardButtonHeight);
    CreateKeyboardUIButton(GetVirtualOrScancodeInputElement(eInputElements::KB_F1_SCANCODE, forVirtualkeys), L"F1", font, &x, y, keyboardButtonWidth, keyboardButtonHeight);
    CreateKeyboardUIButton(GetVirtualOrScancodeInputElement(eInputElements::KB_F2_SCANCODE, forVirtualkeys), L"F2", font, &x, y, keyboardButtonWidth, keyboardButtonHeight);
    CreateKeyboardUIButton(GetVirtualOrScancodeInputElement(eInputElements::KB_F3_SCANCODE, forVirtualkeys), L"F3", font, &x, y, keyboardButtonWidth, keyboardButtonHeight);
    CreateKeyboardUIButton(GetVirtualOrScancodeInputElement(eInputElements::KB_F4_SCANCODE, forVirtualkeys), L"F4", font, &x, y, keyboardButtonWidth, keyboardButtonHeight);
    CreateKeyboardUIButton(GetVirtualOrScancodeInputElement(eInputElements::KB_F5_SCANCODE, forVirtualkeys), L"F5", font, &x, y, keyboardButtonWidth, keyboardButtonHeight);
    CreateKeyboardUIButton(GetVirtualOrScancodeInputElement(eInputElements::KB_F6_SCANCODE, forVirtualkeys), L"F6", font, &x, y, keyboardButtonWidth, keyboardButtonHeight);
    CreateKeyboardUIButton(GetVirtualOrScancodeInputElement(eInputElements::KB_F7_SCANCODE, forVirtualkeys), L"F7", font, &x, y, keyboardButtonWidth, keyboardButtonHeight);
    CreateKeyboardUIButton(GetVirtualOrScancodeInputElement(eInputElements::KB_F8_SCANCODE, forVirtualkeys), L"F8", font, &x, y, keyboardButtonWidth, keyboardButtonHeight);
    CreateKeyboardUIButton(GetVirtualOrScancodeInputElement(eInputElements::KB_F9_SCANCODE, forVirtualkeys), L"F9", font, &x, y, keyboardButtonWidth, keyboardButtonHeight);
    CreateKeyboardUIButton(GetVirtualOrScancodeInputElement(eInputElements::KB_F10_SCANCODE, forVirtualkeys), L"F10", font, &x, y, keyboardButtonWidth, keyboardButtonHeight);
    CreateKeyboardUIButton(GetVirtualOrScancodeInputElement(eInputElements::KB_F11_SCANCODE, forVirtualkeys), L"F11", font, &x, y, keyboardButtonWidth, keyboardButtonHeight);
    CreateKeyboardUIButton(GetVirtualOrScancodeInputElement(eInputElements::KB_F12_SCANCODE, forVirtualkeys), L"F12", font, &x, y, keyboardButtonWidth, keyboardButtonHeight);

    rightmostX = std::max(x, rightmostX);
    y += keyboardButtonHeight + 1.0f;
    x = initialX;

    CreateKeyboardUIButton(GetVirtualOrScancodeInputElement(eInputElements::KB_GRAVE_SCANCODE, forVirtualkeys), L"`", font, &x, y, keyboardButtonWidth, keyboardButtonHeight);
    CreateKeyboardUIButton(GetVirtualOrScancodeInputElement(eInputElements::KB_1_SCANCODE, forVirtualkeys), L"1", font, &x, y, keyboardButtonWidth, keyboardButtonHeight);
    CreateKeyboardUIButton(GetVirtualOrScancodeInputElement(eInputElements::KB_2_SCANCODE, forVirtualkeys), L"2", font, &x, y, keyboardButtonWidth, keyboardButtonHeight);
    CreateKeyboardUIButton(GetVirtualOrScancodeInputElement(eInputElements::KB_3_SCANCODE, forVirtualkeys), L"3", font, &x, y, keyboardButtonWidth, keyboardButtonHeight);
    CreateKeyboardUIButton(GetVirtualOrScancodeInputElement(eInputElements::KB_4_SCANCODE, forVirtualkeys), L"4", font, &x, y, keyboardButtonWidth, keyboardButtonHeight);
    CreateKeyboardUIButton(GetVirtualOrScancodeInputElement(eInputElements::KB_5_SCANCODE, forVirtualkeys), L"5", font, &x, y, keyboardButtonWidth, keyboardButtonHeight);
    CreateKeyboardUIButton(GetVirtualOrScancodeInputElement(eInputElements::KB_6_SCANCODE, forVirtualkeys), L"6", font, &x, y, keyboardButtonWidth, keyboardButtonHeight);
    CreateKeyboardUIButton(GetVirtualOrScancodeInputElement(eInputElements::KB_7_SCANCODE, forVirtualkeys), L"7", font, &x, y, keyboardButtonWidth, keyboardButtonHeight);
    CreateKeyboardUIButton(GetVirtualOrScancodeInputElement(eInputElements::KB_8_SCANCODE, forVirtualkeys), L"8", font, &x, y, keyboardButtonWidth, keyboardButtonHeight);
    CreateKeyboardUIButton(GetVirtualOrScancodeInputElement(eInputElements::KB_9_SCANCODE, forVirtualkeys), L"9", font, &x, y, keyboardButtonWidth, keyboardButtonHeight);
    CreateKeyboardUIButton(GetVirtualOrScancodeInputElement(eInputElements::KB_0_SCANCODE, forVirtualkeys), L"0", font, &x, y, keyboardButtonWidth, keyboardButtonHeight);
    CreateKeyboardUIButton(GetVirtualOrScancodeInputElement(eInputElements::KB_MINUS_SCANCODE, forVirtualkeys), L"-", font, &x, y, keyboardButtonWidth, keyboardButtonHeight);
    CreateKeyboardUIButton(GetVirtualOrScancodeInputElement(eInputElements::KB_EQUALS_SCANCODE, forVirtualkeys), L"+", font, &x, y, keyboardButtonWidth, keyboardButtonHeight);
    CreateKeyboardUIButton(GetVirtualOrScancodeInputElement(eInputElements::KB_BACKSPACE_SCANCODE, forVirtualkeys), L"<-", font, &x, y, keyboardButtonWidth, keyboardButtonHeight);

    rightmostX = std::max(x, rightmostX);
    y += keyboardButtonHeight + 1.0f;
    x = initialX;

    CreateKeyboardUIButton(GetVirtualOrScancodeInputElement(eInputElements::KB_TAB_SCANCODE, forVirtualkeys), L"TAB", font, &x, y, keyboardButtonWidth, keyboardButtonHeight);
    CreateKeyboardUIButton(GetVirtualOrScancodeInputElement(eInputElements::KB_Q_SCANCODE, forVirtualkeys), L"Q", font, &x, y, keyboardButtonWidth, keyboardButtonHeight);
    CreateKeyboardUIButton(GetVirtualOrScancodeInputElement(eInputElements::KB_W_SCANCODE, forVirtualkeys), L"W", font, &x, y, keyboardButtonWidth, keyboardButtonHeight);
    CreateKeyboardUIButton(GetVirtualOrScancodeInputElement(eInputElements::KB_E_SCANCODE, forVirtualkeys), L"E", font, &x, y, keyboardButtonWidth, keyboardButtonHeight);
    CreateKeyboardUIButton(GetVirtualOrScancodeInputElement(eInputElements::KB_R_SCANCODE, forVirtualkeys), L"R", font, &x, y, keyboardButtonWidth, keyboardButtonHeight);
    CreateKeyboardUIButton(GetVirtualOrScancodeInputElement(eInputElements::KB_T_SCANCODE, forVirtualkeys), L"T", font, &x, y, keyboardButtonWidth, keyboardButtonHeight);
    CreateKeyboardUIButton(GetVirtualOrScancodeInputElement(eInputElements::KB_Y_SCANCODE, forVirtualkeys), L"Y", font, &x, y, keyboardButtonWidth, keyboardButtonHeight);
    CreateKeyboardUIButton(GetVirtualOrScancodeInputElement(eInputElements::KB_U_SCANCODE, forVirtualkeys), L"U", font, &x, y, keyboardButtonWidth, keyboardButtonHeight);
    CreateKeyboardUIButton(GetVirtualOrScancodeInputElement(eInputElements::KB_I_SCANCODE, forVirtualkeys), L"I", font, &x, y, keyboardButtonWidth, keyboardButtonHeight);
    CreateKeyboardUIButton(GetVirtualOrScancodeInputElement(eInputElements::KB_O_SCANCODE, forVirtualkeys), L"O", font, &x, y, keyboardButtonWidth, keyboardButtonHeight);
    CreateKeyboardUIButton(GetVirtualOrScancodeInputElement(eInputElements::KB_P_SCANCODE, forVirtualkeys), L"P", font, &x, y, keyboardButtonWidth, keyboardButtonHeight);
    CreateKeyboardUIButton(GetVirtualOrScancodeInputElement(eInputElements::KB_LBRACKET_SCANCODE, forVirtualkeys), L"{", font, &x, y, keyboardButtonWidth, keyboardButtonHeight);
    CreateKeyboardUIButton(GetVirtualOrScancodeInputElement(eInputElements::KB_RBRACKET_SCANCODE, forVirtualkeys), L"}", font, &x, y, keyboardButtonWidth, keyboardButtonHeight);
    CreateKeyboardUIButton(GetVirtualOrScancodeInputElement(eInputElements::KB_ENTER_SCANCODE, forVirtualkeys), L"ENTER", font, &x, y, keyboardButtonWidth, keyboardButtonHeight);

    rightmostX = std::max(x, rightmostX);
    y += keyboardButtonHeight + 1.0f;
    x = initialX;

    CreateKeyboardUIButton(GetVirtualOrScancodeInputElement(eInputElements::KB_CAPSLOCK_SCANCODE, forVirtualkeys), L"CAPS", font, &x, y, keyboardButtonWidth, keyboardButtonHeight);
    CreateKeyboardUIButton(GetVirtualOrScancodeInputElement(eInputElements::KB_A_SCANCODE, forVirtualkeys), L"A", font, &x, y, keyboardButtonWidth, keyboardButtonHeight);
    CreateKeyboardUIButton(GetVirtualOrScancodeInputElement(eInputElements::KB_S_SCANCODE, forVirtualkeys), L"S", font, &x, y, keyboardButtonWidth, keyboardButtonHeight);
    CreateKeyboardUIButton(GetVirtualOrScancodeInputElement(eInputElements::KB_D_SCANCODE, forVirtualkeys), L"D", font, &x, y, keyboardButtonWidth, keyboardButtonHeight);
    CreateKeyboardUIButton(GetVirtualOrScancodeInputElement(eInputElements::KB_F_SCANCODE, forVirtualkeys), L"F", font, &x, y, keyboardButtonWidth, keyboardButtonHeight);
    CreateKeyboardUIButton(GetVirtualOrScancodeInputElement(eInputElements::KB_G_SCANCODE, forVirtualkeys), L"G", font, &x, y, keyboardButtonWidth, keyboardButtonHeight);
    CreateKeyboardUIButton(GetVirtualOrScancodeInputElement(eInputElements::KB_H_SCANCODE, forVirtualkeys), L"H", font, &x, y, keyboardButtonWidth, keyboardButtonHeight);
    CreateKeyboardUIButton(GetVirtualOrScancodeInputElement(eInputElements::KB_J_SCANCODE, forVirtualkeys), L"J", font, &x, y, keyboardButtonWidth, keyboardButtonHeight);
    CreateKeyboardUIButton(GetVirtualOrScancodeInputElement(eInputElements::KB_K_SCANCODE, forVirtualkeys), L"K", font, &x, y, keyboardButtonWidth, keyboardButtonHeight);
    CreateKeyboardUIButton(GetVirtualOrScancodeInputElement(eInputElements::KB_L_SCANCODE, forVirtualkeys), L"L", font, &x, y, keyboardButtonWidth, keyboardButtonHeight);
    CreateKeyboardUIButton(GetVirtualOrScancodeInputElement(eInputElements::KB_SEMICOLON_SCANCODE, forVirtualkeys), L":", font, &x, y, keyboardButtonWidth, keyboardButtonHeight);
    CreateKeyboardUIButton(GetVirtualOrScancodeInputElement(eInputElements::KB_APOSTROPHE_SCANCODE, forVirtualkeys), L"\"", font, &x, y, keyboardButtonWidth, keyboardButtonHeight);
    CreateKeyboardUIButton(GetVirtualOrScancodeInputElement(eInputElements::KB_BACKSLASH_SCANCODE, forVirtualkeys), L"\\", font, &x, y, keyboardButtonWidth, keyboardButtonHeight);

    rightmostX = std::max(x, rightmostX);
    y += keyboardButtonHeight + 1.0f;
    x = initialX;

    CreateKeyboardUIButton(GetVirtualOrScancodeInputElement(eInputElements::KB_LSHIFT_SCANCODE, forVirtualkeys), L"SHFT", font, &x, y, keyboardButtonWidth, keyboardButtonHeight);
    CreateKeyboardUIButton(GetVirtualOrScancodeInputElement(eInputElements::KB_NONUSBACKSLASH_SCANCODE, forVirtualkeys), L"\\", font, &x, y, keyboardButtonWidth, keyboardButtonHeight);
    CreateKeyboardUIButton(GetVirtualOrScancodeInputElement(eInputElements::KB_Z_SCANCODE, forVirtualkeys), L"Z", font, &x, y, keyboardButtonWidth, keyboardButtonHeight);
    CreateKeyboardUIButton(GetVirtualOrScancodeInputElement(eInputElements::KB_X_SCANCODE, forVirtualkeys), L"X", font, &x, y, keyboardButtonWidth, keyboardButtonHeight);
    CreateKeyboardUIButton(GetVirtualOrScancodeInputElement(eInputElements::KB_C_SCANCODE, forVirtualkeys), L"C", font, &x, y, keyboardButtonWidth, keyboardButtonHeight);
    CreateKeyboardUIButton(GetVirtualOrScancodeInputElement(eInputElements::KB_V_SCANCODE, forVirtualkeys), L"V", font, &x, y, keyboardButtonWidth, keyboardButtonHeight);
    CreateKeyboardUIButton(GetVirtualOrScancodeInputElement(eInputElements::KB_B_SCANCODE, forVirtualkeys), L"B", font, &x, y, keyboardButtonWidth, keyboardButtonHeight);
    CreateKeyboardUIButton(GetVirtualOrScancodeInputElement(eInputElements::KB_N_SCANCODE, forVirtualkeys), L"N", font, &x, y, keyboardButtonWidth, keyboardButtonHeight);
    CreateKeyboardUIButton(GetVirtualOrScancodeInputElement(eInputElements::KB_M_SCANCODE, forVirtualkeys), L"M", font, &x, y, keyboardButtonWidth, keyboardButtonHeight);
    CreateKeyboardUIButton(GetVirtualOrScancodeInputElement(eInputElements::KB_COMMA_SCANCODE, forVirtualkeys), L"<", font, &x, y, keyboardButtonWidth, keyboardButtonHeight);
    CreateKeyboardUIButton(GetVirtualOrScancodeInputElement(eInputElements::KB_PERIOD_SCANCODE, forVirtualkeys), L">", font, &x, y, keyboardButtonWidth, keyboardButtonHeight);
    CreateKeyboardUIButton(GetVirtualOrScancodeInputElement(eInputElements::KB_SLASH_SCANCODE, forVirtualkeys), L"?", font, &x, y, keyboardButtonWidth, keyboardButtonHeight);
    CreateKeyboardUIButton(GetVirtualOrScancodeInputElement(eInputElements::KB_RSHIFT_SCANCODE, forVirtualkeys), L"Shift", font, &x, y, keyboardButtonWidth, keyboardButtonHeight);

    rightmostX = std::max(x, rightmostX);
    y += keyboardButtonHeight + 1.0f;
    x = initialX;

    CreateKeyboardUIButton(GetVirtualOrScancodeInputElement(eInputElements::KB_LCTRL_SCANCODE, forVirtualkeys), L"CTRL", font, &x, y, keyboardButtonWidth, keyboardButtonHeight);
    CreateKeyboardUIButton(GetVirtualOrScancodeInputElement(eInputElements::KB_LWIN_SCANCODE, forVirtualkeys), L"WIN", font, &x, y, keyboardButtonWidth, keyboardButtonHeight);
    CreateKeyboardUIButton(GetVirtualOrScancodeInputElement(eInputElements::KB_LALT_SCANCODE, forVirtualkeys), L"ALT", font, &x, y, keyboardButtonWidth, keyboardButtonHeight);
    CreateKeyboardUIButton(GetVirtualOrScancodeInputElement(eInputElements::KB_SPACE_SCANCODE, forVirtualkeys), L"SPACE", font, &x, y, keyboardButtonWidth * 4, keyboardButtonHeight);
    CreateKeyboardUIButton(GetVirtualOrScancodeInputElement(eInputElements::KB_RALT_SCANCODE, forVirtualkeys), L"ALT", font, &x, y, keyboardButtonWidth, keyboardButtonHeight);
    CreateKeyboardUIButton(GetVirtualOrScancodeInputElement(eInputElements::KB_RWIN_SCANCODE, forVirtualkeys), L"WIN", font, &x, y, keyboardButtonWidth, keyboardButtonHeight);
    CreateKeyboardUIButton(GetVirtualOrScancodeInputElement(eInputElements::KB_MENU_SCANCODE, forVirtualkeys), L"MENU", font, &x, y, keyboardButtonWidth, keyboardButtonHeight);
    CreateKeyboardUIButton(GetVirtualOrScancodeInputElement(eInputElements::KB_RCTRL_SCANCODE, forVirtualkeys), L"CTRL", font, &x, y, keyboardButtonWidth, keyboardButtonHeight);

    const float32 middleSectionStartX = rightmostX + keyboardButtonWidth;

    y = initialY;
    x = middleSectionStartX;
    CreateKeyboardUIButton(GetVirtualOrScancodeInputElement(eInputElements::KB_PRINTSCREEN_SCANCODE, forVirtualkeys), L"PSCR", font, &x, y, keyboardButtonWidth, keyboardButtonHeight);
    CreateKeyboardUIButton(GetVirtualOrScancodeInputElement(eInputElements::KB_SCROLLLOCK_SCANCODE, forVirtualkeys), L"SCROLLLOCK", font, &x, y, keyboardButtonWidth, keyboardButtonHeight);
    CreateKeyboardUIButton(GetVirtualOrScancodeInputElement(eInputElements::KB_PAUSE_SCANCODE, forVirtualkeys), L"PAUSE", font, &x, y, keyboardButtonWidth, keyboardButtonHeight);

    rightmostX = std::max(x, rightmostX);
    y += keyboardButtonHeight + 1.0f;
    x = middleSectionStartX;
    CreateKeyboardUIButton(GetVirtualOrScancodeInputElement(eInputElements::KB_INSERT_SCANCODE, forVirtualkeys), L"INSERT", font, &x, y, keyboardButtonWidth, keyboardButtonHeight);
    CreateKeyboardUIButton(GetVirtualOrScancodeInputElement(eInputElements::KB_HOME_SCANCODE, forVirtualkeys), L"HOME", font, &x, y, keyboardButtonWidth, keyboardButtonHeight);
    CreateKeyboardUIButton(GetVirtualOrScancodeInputElement(eInputElements::KB_PAGEUP_SCANCODE, forVirtualkeys), L"PGUP", font, &x, y, keyboardButtonWidth, keyboardButtonHeight);

    rightmostX = std::max(x, rightmostX);
    y += keyboardButtonHeight + 1.0f;
    x = middleSectionStartX;
    CreateKeyboardUIButton(GetVirtualOrScancodeInputElement(eInputElements::KB_DELETE_SCANCODE, forVirtualkeys), L"DEL", font, &x, y, keyboardButtonWidth, keyboardButtonHeight);
    CreateKeyboardUIButton(GetVirtualOrScancodeInputElement(eInputElements::KB_END_SCANCODE, forVirtualkeys), L"END", font, &x, y, keyboardButtonWidth, keyboardButtonHeight);
    CreateKeyboardUIButton(GetVirtualOrScancodeInputElement(eInputElements::KB_PAGEDOWN_SCANCODE, forVirtualkeys), L"PGDOWN", font, &x, y, keyboardButtonWidth, keyboardButtonHeight);

    rightmostX = std::max(x, rightmostX);
    y += 2.0f * (keyboardButtonHeight + 1.0f);
    x = middleSectionStartX + keyboardButtonWidth + 1.0f;
    CreateKeyboardUIButton(GetVirtualOrScancodeInputElement(eInputElements::KB_UP_SCANCODE, forVirtualkeys), L"UP", font, &x, y, keyboardButtonWidth, keyboardButtonHeight);

    rightmostX = std::max(x, rightmostX);
    y += keyboardButtonHeight + 1.0f;
    x = middleSectionStartX;
    CreateKeyboardUIButton(GetVirtualOrScancodeInputElement(eInputElements::KB_LEFT_SCANCODE, forVirtualkeys), L"LEFT", font, &x, y, keyboardButtonWidth, keyboardButtonHeight);
    CreateKeyboardUIButton(GetVirtualOrScancodeInputElement(eInputElements::KB_DOWN_SCANCODE, forVirtualkeys), L"DOWN", font, &x, y, keyboardButtonWidth, keyboardButtonHeight);
    CreateKeyboardUIButton(GetVirtualOrScancodeInputElement(eInputElements::KB_RIGHT_SCANCODE, forVirtualkeys), L"RIGHT", font, &x, y, keyboardButtonWidth, keyboardButtonHeight);

    const float32 numpadSectionStartX = rightmostX + keyboardButtonWidth;

    y = initialY;
    x = numpadSectionStartX;
    CreateKeyboardUIButton(GetVirtualOrScancodeInputElement(eInputElements::KB_NUMLOCK_SCANCODE, forVirtualkeys), L"NUMLOCK", font, &x, y, keyboardButtonWidth, keyboardButtonHeight);
    CreateKeyboardUIButton(GetVirtualOrScancodeInputElement(eInputElements::KB_DIVIDE_SCANCODE, forVirtualkeys), L"/", font, &x, y, keyboardButtonWidth, keyboardButtonHeight);
    CreateKeyboardUIButton(GetVirtualOrScancodeInputElement(eInputElements::KB_MULTIPLY_SCANCODE, forVirtualkeys), L"*", font, &x, y, keyboardButtonWidth, keyboardButtonHeight);
    CreateKeyboardUIButton(GetVirtualOrScancodeInputElement(eInputElements::KB_NUMPAD_MINUS_SCANCODE, forVirtualkeys), L"-", font, &x, y, keyboardButtonWidth, keyboardButtonHeight);

    rightmostX = std::max(x, rightmostX);
    y += keyboardButtonHeight + 1.0f;
    x = numpadSectionStartX;
    CreateKeyboardUIButton(GetVirtualOrScancodeInputElement(eInputElements::KB_NUMPAD_7_SCANCODE, forVirtualkeys), L"7", font, &x, y, keyboardButtonWidth, keyboardButtonHeight);
    CreateKeyboardUIButton(GetVirtualOrScancodeInputElement(eInputElements::KB_NUMPAD_8_SCANCODE, forVirtualkeys), L"8", font, &x, y, keyboardButtonWidth, keyboardButtonHeight);
    CreateKeyboardUIButton(GetVirtualOrScancodeInputElement(eInputElements::KB_NUMPAD_9_SCANCODE, forVirtualkeys), L"9", font, &x, y, keyboardButtonWidth, keyboardButtonHeight);
    CreateKeyboardUIButton(GetVirtualOrScancodeInputElement(eInputElements::KB_NUMPAD_PLUS_SCANCODE, forVirtualkeys), L"+", font, &x, y, keyboardButtonWidth, keyboardButtonHeight);

    rightmostX = std::max(x, rightmostX);
    y += keyboardButtonHeight + 1.0f;
    x = numpadSectionStartX;
    CreateKeyboardUIButton(GetVirtualOrScancodeInputElement(eInputElements::KB_NUMPAD_4_SCANCODE, forVirtualkeys), L"4", font, &x, y, keyboardButtonWidth, keyboardButtonHeight);
    CreateKeyboardUIButton(GetVirtualOrScancodeInputElement(eInputElements::KB_NUMPAD_5_SCANCODE, forVirtualkeys), L"5", font, &x, y, keyboardButtonWidth, keyboardButtonHeight);
    CreateKeyboardUIButton(GetVirtualOrScancodeInputElement(eInputElements::KB_NUMPAD_6_SCANCODE, forVirtualkeys), L"6", font, &x, y, keyboardButtonWidth, keyboardButtonHeight);

    rightmostX = std::max(x, rightmostX);
    y += keyboardButtonHeight + 1.0f;
    x = numpadSectionStartX;
    CreateKeyboardUIButton(GetVirtualOrScancodeInputElement(eInputElements::KB_NUMPAD_1_SCANCODE, forVirtualkeys), L"1", font, &x, y, keyboardButtonWidth, keyboardButtonHeight);
    CreateKeyboardUIButton(GetVirtualOrScancodeInputElement(eInputElements::KB_NUMPAD_2_SCANCODE, forVirtualkeys), L"2", font, &x, y, keyboardButtonWidth, keyboardButtonHeight);
    CreateKeyboardUIButton(GetVirtualOrScancodeInputElement(eInputElements::KB_NUMPAD_3_SCANCODE, forVirtualkeys), L"3", font, &x, y, keyboardButtonWidth, keyboardButtonHeight);
    CreateKeyboardUIButton(GetVirtualOrScancodeInputElement(eInputElements::KB_NUMPAD_ENTER_SCANCODE, forVirtualkeys), L"ENTER", font, &x, y, keyboardButtonWidth, keyboardButtonHeight);

    rightmostX = std::max(x, rightmostX);
    y += keyboardButtonHeight + 1.0f;
    x = numpadSectionStartX;
    CreateKeyboardUIButton(GetVirtualOrScancodeInputElement(eInputElements::KB_NUMPAD_0_SCANCODE, forVirtualkeys), L"0", font, &x, y, keyboardButtonWidth, keyboardButtonHeight);
    CreateKeyboardUIButton(GetVirtualOrScancodeInputElement(eInputElements::KB_NUMPAD_DELETE_SCANCODE, forVirtualkeys), L"DEL", font, &x, y, keyboardButtonWidth, keyboardButtonHeight);
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

    inputListenerStartButton = new UIButton(Rect(680, 450, 150, 30));
    inputListenerStartButton->SetStateFont(0xFF, font);
    inputListenerStartButton->SetStateFontColor(0xFF, Color::White);
    inputListenerStartButton->SetDebugDraw(true);
    inputListenerStartButton->SetStateText(0xFF, L"Start listening for input");
    inputListenerStartButton->AddEvent(UIButton::EVENT_TOUCH_UP_INSIDE, Message(this, &InputSystemTest::OnInputListenerButtonPressed));
    AddControl(inputListenerStartButton);

    inputListenerResultField = new UIStaticText(Rect(680, 500, 150, 30));
    inputListenerResultField->SetTextColor(Color::White);
    inputListenerResultField->SetFont(font);
    inputListenerResultField->SetMultiline(true);
    inputListenerResultField->SetTextAlign(ALIGN_LEFT | ALIGN_TOP);
    inputListenerResultField->SetText(L"");
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
        eInputElements virtualCounterpart = GetEngineContext()->deviceManager->GetKeyboard()->ConvertScancodeToVirtual(event.elementId);

        UIButton* scancodeButton = keyboardButtons[event.elementId];
        UIButton* virtualButton = keyboardButtons[virtualCounterpart];

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
    GetEngineContext()->actionSystem->GetUserInput(MakeFunction(this, &InputSystemTest::OnInputListeningEnded));
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
