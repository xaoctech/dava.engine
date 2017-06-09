#include "Infrastructure/TestBed.h"
#include "Tests/InputSystemTest.h"

#include <Engine/Engine.h>
#include <DeviceManager/DeviceManager.h>
#include <Input/InputBindingListener.h>
#include <Input/Keyboard.h>
#include <Utils/UTF8Utils.h>
#include <UI/Render/UIDebugRenderComponent.h>

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
    CreateKeyboardUI(L"Scancode keyboard", 20.0f, 20.0f);
    CreateMouseUI();
    CreateTouchUI();
    CreateActionsUI();
    CreateInputListenerUI();

    // Subscribe to events
    rawInputToken = GetEngineContext()->inputSystem->AddHandler(eInputDeviceTypes::CLASS_ALL, MakeFunction(this, &InputSystemTest::OnInputEvent));
    GetEngineContext()->actionSystem->ActionTriggered.Connect(this, &InputSystemTest::OnAction);
    Engine::Instance()->update.Connect(this, &InputSystemTest::OnUpdate);

    // Bind action set

    ActionSet set;

    DigitalBinding action1;
    action1.actionId = ACTION_1;
    action1.digitalElements[0] = eInputElements::KB_W;
    action1.digitalStates[0] = DigitalElementState::Pressed();
    set.digitalBindings.push_back(action1);

    DigitalBinding action2;
    action2.actionId = ACTION_2;
    action2.digitalElements[0] = eInputElements::KB_SPACE;
    action2.digitalStates[0] = DigitalElementState::JustPressed();
    set.digitalBindings.push_back(action2);

    DigitalBinding action3;
    action3.actionId = ACTION_3;
    action3.digitalElements[0] = eInputElements::KB_SPACE;
    action3.digitalStates[0] = DigitalElementState::JustPressed();
    action3.digitalElements[1] = eInputElements::KB_LSHIFT;
    action3.digitalStates[1] = DigitalElementState::Pressed();
    set.digitalBindings.push_back(action3);

    AnalogBinding action4;
    action4.actionId = ACTION_4;
    action4.analogElementId = eInputElements::MOUSE_POSITION;
    set.analogBindings.push_back(action4);

    AnalogBinding action5;
    action5.actionId = ACTION_5;
    action5.analogElementId = eInputElements::MOUSE_POSITION;
    action5.digitalElements[0] = eInputElements::MOUSE_LBUTTON;
    action5.digitalStates[0] = DigitalElementState::Pressed();
    action5.digitalElements[1] = eInputElements::KB_LCTRL;
    action5.digitalStates[1] = DigitalElementState::Pressed();
    set.analogBindings.push_back(action5);

    GetEngineContext()->actionSystem->BindSet(set, 1, 2);
}

void InputSystemTest::UnloadResources()
{
    GetEngineContext()->inputSystem->RemoveHandler(rawInputToken);
    GetEngineContext()->actionSystem->ActionTriggered.Disconnect(this);
    Engine::Instance()->update.Disconnect(this);

    for (auto it = keyboardButtons.begin(); it != keyboardButtons.end(); ++it)
    {
        SafeRelease(it->second);
    }

    for (auto it = keyboardsHeaders.begin(); it != keyboardsHeaders.end(); ++it)
    {
        SafeRelease(*it);
    }

    SafeRelease(mouseHeader);
    SafeRelease(mouseBody);
    for (auto it = mouseButtons.begin(); it != mouseButtons.end(); ++it)
    {
        SafeRelease(it->second);
    }

    SafeRelease(touchHeader);
    for (auto it = touchClickButtons.begin(); it != touchClickButtons.end(); ++it)
    {
        SafeRelease(it->second);
    }
    for (auto it = touchMoveButtons.begin(); it != touchMoveButtons.end(); ++it)
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

    GetEngineContext()->actionSystem->UnbindAllSets();

    BaseScreen::UnloadResources();
}

void InputSystemTest::CreateKeyboardUI(WideString header, float32 x, float32 y)
{
    ScopedPtr<FTFont> font(FTFont::Create("~res:/Fonts/DejaVuSans.ttf"));

    const float32 keyboardButtonWidth = 20.0f;
    const float32 keyboardButtonHeight = 20.0f;

    const float32 headerHeight = 15.0f;

    font->SetSize(10);

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

    CreateKeyboardUIButton(eInputElements::KB_ESCAPE, L"ESC", font, &x, y, keyboardButtonWidth, keyboardButtonHeight);
    CreateKeyboardUIButton(eInputElements::KB_F1, L"F1", font, &x, y, keyboardButtonWidth, keyboardButtonHeight);
    CreateKeyboardUIButton(eInputElements::KB_F2, L"F2", font, &x, y, keyboardButtonWidth, keyboardButtonHeight);
    CreateKeyboardUIButton(eInputElements::KB_F3, L"F3", font, &x, y, keyboardButtonWidth, keyboardButtonHeight);
    CreateKeyboardUIButton(eInputElements::KB_F4, L"F4", font, &x, y, keyboardButtonWidth, keyboardButtonHeight);
    CreateKeyboardUIButton(eInputElements::KB_F5, L"F5", font, &x, y, keyboardButtonWidth, keyboardButtonHeight);
    CreateKeyboardUIButton(eInputElements::KB_F6, L"F6", font, &x, y, keyboardButtonWidth, keyboardButtonHeight);
    CreateKeyboardUIButton(eInputElements::KB_F7, L"F7", font, &x, y, keyboardButtonWidth, keyboardButtonHeight);
    CreateKeyboardUIButton(eInputElements::KB_F8, L"F8", font, &x, y, keyboardButtonWidth, keyboardButtonHeight);
    CreateKeyboardUIButton(eInputElements::KB_F9, L"F9", font, &x, y, keyboardButtonWidth, keyboardButtonHeight);
    CreateKeyboardUIButton(eInputElements::KB_F10, L"F10", font, &x, y, keyboardButtonWidth, keyboardButtonHeight);
    CreateKeyboardUIButton(eInputElements::KB_F11, L"F11", font, &x, y, keyboardButtonWidth, keyboardButtonHeight);
    CreateKeyboardUIButton(eInputElements::KB_F12, L"F12", font, &x, y, keyboardButtonWidth, keyboardButtonHeight);

    rightmostX = std::max(x, rightmostX);
    y += keyboardButtonHeight + 1.0f;
    x = initialX;

    CreateKeyboardUIButton(eInputElements::KB_GRAVE, L"`", font, &x, y, keyboardButtonWidth, keyboardButtonHeight);
    CreateKeyboardUIButton(eInputElements::KB_1, L"1", font, &x, y, keyboardButtonWidth, keyboardButtonHeight);
    CreateKeyboardUIButton(eInputElements::KB_2, L"2", font, &x, y, keyboardButtonWidth, keyboardButtonHeight);
    CreateKeyboardUIButton(eInputElements::KB_3, L"3", font, &x, y, keyboardButtonWidth, keyboardButtonHeight);
    CreateKeyboardUIButton(eInputElements::KB_4, L"4", font, &x, y, keyboardButtonWidth, keyboardButtonHeight);
    CreateKeyboardUIButton(eInputElements::KB_5, L"5", font, &x, y, keyboardButtonWidth, keyboardButtonHeight);
    CreateKeyboardUIButton(eInputElements::KB_6, L"6", font, &x, y, keyboardButtonWidth, keyboardButtonHeight);
    CreateKeyboardUIButton(eInputElements::KB_7, L"7", font, &x, y, keyboardButtonWidth, keyboardButtonHeight);
    CreateKeyboardUIButton(eInputElements::KB_8, L"8", font, &x, y, keyboardButtonWidth, keyboardButtonHeight);
    CreateKeyboardUIButton(eInputElements::KB_9, L"9", font, &x, y, keyboardButtonWidth, keyboardButtonHeight);
    CreateKeyboardUIButton(eInputElements::KB_0, L"0", font, &x, y, keyboardButtonWidth, keyboardButtonHeight);
    CreateKeyboardUIButton(eInputElements::KB_MINUS, L"-", font, &x, y, keyboardButtonWidth, keyboardButtonHeight);
    CreateKeyboardUIButton(eInputElements::KB_EQUALS, L"+", font, &x, y, keyboardButtonWidth, keyboardButtonHeight);
    CreateKeyboardUIButton(eInputElements::KB_BACKSPACE, L"<-", font, &x, y, keyboardButtonWidth, keyboardButtonHeight);

    rightmostX = std::max(x, rightmostX);
    y += keyboardButtonHeight + 1.0f;
    x = initialX;

    CreateKeyboardUIButton(eInputElements::KB_TAB, L"TAB", font, &x, y, keyboardButtonWidth, keyboardButtonHeight);
    CreateKeyboardUIButton(eInputElements::KB_Q, L"Q", font, &x, y, keyboardButtonWidth, keyboardButtonHeight);
    CreateKeyboardUIButton(eInputElements::KB_W, L"W", font, &x, y, keyboardButtonWidth, keyboardButtonHeight);
    CreateKeyboardUIButton(eInputElements::KB_E, L"E", font, &x, y, keyboardButtonWidth, keyboardButtonHeight);
    CreateKeyboardUIButton(eInputElements::KB_R, L"R", font, &x, y, keyboardButtonWidth, keyboardButtonHeight);
    CreateKeyboardUIButton(eInputElements::KB_T, L"T", font, &x, y, keyboardButtonWidth, keyboardButtonHeight);
    CreateKeyboardUIButton(eInputElements::KB_Y, L"Y", font, &x, y, keyboardButtonWidth, keyboardButtonHeight);
    CreateKeyboardUIButton(eInputElements::KB_U, L"U", font, &x, y, keyboardButtonWidth, keyboardButtonHeight);
    CreateKeyboardUIButton(eInputElements::KB_I, L"I", font, &x, y, keyboardButtonWidth, keyboardButtonHeight);
    CreateKeyboardUIButton(eInputElements::KB_O, L"O", font, &x, y, keyboardButtonWidth, keyboardButtonHeight);
    CreateKeyboardUIButton(eInputElements::KB_P, L"P", font, &x, y, keyboardButtonWidth, keyboardButtonHeight);
    CreateKeyboardUIButton(eInputElements::KB_LBRACKET, L"{", font, &x, y, keyboardButtonWidth, keyboardButtonHeight);
    CreateKeyboardUIButton(eInputElements::KB_RBRACKET, L"}", font, &x, y, keyboardButtonWidth, keyboardButtonHeight);
    CreateKeyboardUIButton(eInputElements::KB_ENTER, L"ENTER", font, &x, y, keyboardButtonWidth, keyboardButtonHeight);

    rightmostX = std::max(x, rightmostX);
    y += keyboardButtonHeight + 1.0f;
    x = initialX;

    CreateKeyboardUIButton(eInputElements::KB_CAPSLOCK, L"CAPS", font, &x, y, keyboardButtonWidth, keyboardButtonHeight);
    CreateKeyboardUIButton(eInputElements::KB_A, L"A", font, &x, y, keyboardButtonWidth, keyboardButtonHeight);
    CreateKeyboardUIButton(eInputElements::KB_S, L"S", font, &x, y, keyboardButtonWidth, keyboardButtonHeight);
    CreateKeyboardUIButton(eInputElements::KB_D, L"D", font, &x, y, keyboardButtonWidth, keyboardButtonHeight);
    CreateKeyboardUIButton(eInputElements::KB_F, L"F", font, &x, y, keyboardButtonWidth, keyboardButtonHeight);
    CreateKeyboardUIButton(eInputElements::KB_G, L"G", font, &x, y, keyboardButtonWidth, keyboardButtonHeight);
    CreateKeyboardUIButton(eInputElements::KB_H, L"H", font, &x, y, keyboardButtonWidth, keyboardButtonHeight);
    CreateKeyboardUIButton(eInputElements::KB_J, L"J", font, &x, y, keyboardButtonWidth, keyboardButtonHeight);
    CreateKeyboardUIButton(eInputElements::KB_K, L"K", font, &x, y, keyboardButtonWidth, keyboardButtonHeight);
    CreateKeyboardUIButton(eInputElements::KB_L, L"L", font, &x, y, keyboardButtonWidth, keyboardButtonHeight);
    CreateKeyboardUIButton(eInputElements::KB_SEMICOLON, L":", font, &x, y, keyboardButtonWidth, keyboardButtonHeight);
    CreateKeyboardUIButton(eInputElements::KB_APOSTROPHE, L"\"", font, &x, y, keyboardButtonWidth, keyboardButtonHeight);
    CreateKeyboardUIButton(eInputElements::KB_BACKSLASH, L"\\", font, &x, y, keyboardButtonWidth, keyboardButtonHeight);

    rightmostX = std::max(x, rightmostX);
    y += keyboardButtonHeight + 1.0f;
    x = initialX;

    CreateKeyboardUIButton(eInputElements::KB_LSHIFT, L"SHFT", font, &x, y, keyboardButtonWidth, keyboardButtonHeight);
    CreateKeyboardUIButton(eInputElements::KB_NONUSBACKSLASH, L"\\", font, &x, y, keyboardButtonWidth, keyboardButtonHeight);
    CreateKeyboardUIButton(eInputElements::KB_Z, L"Z", font, &x, y, keyboardButtonWidth, keyboardButtonHeight);
    CreateKeyboardUIButton(eInputElements::KB_X, L"X", font, &x, y, keyboardButtonWidth, keyboardButtonHeight);
    CreateKeyboardUIButton(eInputElements::KB_C, L"C", font, &x, y, keyboardButtonWidth, keyboardButtonHeight);
    CreateKeyboardUIButton(eInputElements::KB_V, L"V", font, &x, y, keyboardButtonWidth, keyboardButtonHeight);
    CreateKeyboardUIButton(eInputElements::KB_B, L"B", font, &x, y, keyboardButtonWidth, keyboardButtonHeight);
    CreateKeyboardUIButton(eInputElements::KB_N, L"N", font, &x, y, keyboardButtonWidth, keyboardButtonHeight);
    CreateKeyboardUIButton(eInputElements::KB_M, L"M", font, &x, y, keyboardButtonWidth, keyboardButtonHeight);
    CreateKeyboardUIButton(eInputElements::KB_COMMA, L"<", font, &x, y, keyboardButtonWidth, keyboardButtonHeight);
    CreateKeyboardUIButton(eInputElements::KB_PERIOD, L">", font, &x, y, keyboardButtonWidth, keyboardButtonHeight);
    CreateKeyboardUIButton(eInputElements::KB_SLASH, L"?", font, &x, y, keyboardButtonWidth, keyboardButtonHeight);
    CreateKeyboardUIButton(eInputElements::KB_RSHIFT, L"Shift", font, &x, y, keyboardButtonWidth, keyboardButtonHeight);

    rightmostX = std::max(x, rightmostX);
    y += keyboardButtonHeight + 1.0f;
    x = initialX;

    CreateKeyboardUIButton(eInputElements::KB_LCTRL, L"CTRL", font, &x, y, keyboardButtonWidth, keyboardButtonHeight);
    CreateKeyboardUIButton(eInputElements::KB_LWIN, L"WIN", font, &x, y, keyboardButtonWidth, keyboardButtonHeight);
    CreateKeyboardUIButton(eInputElements::KB_LALT, L"ALT", font, &x, y, keyboardButtonWidth, keyboardButtonHeight);
    CreateKeyboardUIButton(eInputElements::KB_SPACE, L"SPACE", font, &x, y, keyboardButtonWidth * 4, keyboardButtonHeight);
    CreateKeyboardUIButton(eInputElements::KB_RALT, L"ALT", font, &x, y, keyboardButtonWidth, keyboardButtonHeight);
    CreateKeyboardUIButton(eInputElements::KB_RWIN, L"WIN", font, &x, y, keyboardButtonWidth, keyboardButtonHeight);
    CreateKeyboardUIButton(eInputElements::KB_MENU, L"MENU", font, &x, y, keyboardButtonWidth, keyboardButtonHeight);
    CreateKeyboardUIButton(eInputElements::KB_RCTRL, L"CTRL", font, &x, y, keyboardButtonWidth, keyboardButtonHeight);

    const float32 middleSectionStartX = rightmostX + keyboardButtonWidth;

    y = initialY;
    x = middleSectionStartX;
    CreateKeyboardUIButton(eInputElements::KB_PRINTSCREEN, L"PSCR", font, &x, y, keyboardButtonWidth, keyboardButtonHeight);
    CreateKeyboardUIButton(eInputElements::KB_SCROLLLOCK, L"SCROLLLOCK", font, &x, y, keyboardButtonWidth, keyboardButtonHeight);
    CreateKeyboardUIButton(eInputElements::KB_PAUSE, L"PAUSE", font, &x, y, keyboardButtonWidth, keyboardButtonHeight);

    rightmostX = std::max(x, rightmostX);
    y += keyboardButtonHeight + 1.0f;
    x = middleSectionStartX;
    CreateKeyboardUIButton(eInputElements::KB_INSERT, L"INSERT", font, &x, y, keyboardButtonWidth, keyboardButtonHeight);
    CreateKeyboardUIButton(eInputElements::KB_HOME, L"HOME", font, &x, y, keyboardButtonWidth, keyboardButtonHeight);
    CreateKeyboardUIButton(eInputElements::KB_PAGEUP, L"PGUP", font, &x, y, keyboardButtonWidth, keyboardButtonHeight);

    rightmostX = std::max(x, rightmostX);
    y += keyboardButtonHeight + 1.0f;
    x = middleSectionStartX;
    CreateKeyboardUIButton(eInputElements::KB_DELETE, L"DEL", font, &x, y, keyboardButtonWidth, keyboardButtonHeight);
    CreateKeyboardUIButton(eInputElements::KB_END, L"END", font, &x, y, keyboardButtonWidth, keyboardButtonHeight);
    CreateKeyboardUIButton(eInputElements::KB_PAGEDOWN, L"PGDOWN", font, &x, y, keyboardButtonWidth, keyboardButtonHeight);

    rightmostX = std::max(x, rightmostX);
    y += 2.0f * (keyboardButtonHeight + 1.0f);
    x = middleSectionStartX + keyboardButtonWidth + 1.0f;
    CreateKeyboardUIButton(eInputElements::KB_UP, L"UP", font, &x, y, keyboardButtonWidth, keyboardButtonHeight);

    rightmostX = std::max(x, rightmostX);
    y += keyboardButtonHeight + 1.0f;
    x = middleSectionStartX;
    CreateKeyboardUIButton(eInputElements::KB_LEFT, L"LEFT", font, &x, y, keyboardButtonWidth, keyboardButtonHeight);
    CreateKeyboardUIButton(eInputElements::KB_DOWN, L"DOWN", font, &x, y, keyboardButtonWidth, keyboardButtonHeight);
    CreateKeyboardUIButton(eInputElements::KB_RIGHT, L"RIGHT", font, &x, y, keyboardButtonWidth, keyboardButtonHeight);

    const float32 numpadSectionStartX = rightmostX + keyboardButtonWidth;

    y = initialY;
    x = numpadSectionStartX;
    CreateKeyboardUIButton(eInputElements::KB_NUMLOCK, L"NUMLOCK", font, &x, y, keyboardButtonWidth, keyboardButtonHeight);
    CreateKeyboardUIButton(eInputElements::KB_DIVIDE, L"/", font, &x, y, keyboardButtonWidth, keyboardButtonHeight);
    CreateKeyboardUIButton(eInputElements::KB_MULTIPLY, L"*", font, &x, y, keyboardButtonWidth, keyboardButtonHeight);
    CreateKeyboardUIButton(eInputElements::KB_NUMPAD_MINUS, L"-", font, &x, y, keyboardButtonWidth, keyboardButtonHeight);

    rightmostX = std::max(x, rightmostX);
    y += keyboardButtonHeight + 1.0f;
    x = numpadSectionStartX;
    CreateKeyboardUIButton(eInputElements::KB_NUMPAD_7, L"7", font, &x, y, keyboardButtonWidth, keyboardButtonHeight);
    CreateKeyboardUIButton(eInputElements::KB_NUMPAD_8, L"8", font, &x, y, keyboardButtonWidth, keyboardButtonHeight);
    CreateKeyboardUIButton(eInputElements::KB_NUMPAD_9, L"9", font, &x, y, keyboardButtonWidth, keyboardButtonHeight);
    CreateKeyboardUIButton(eInputElements::KB_NUMPAD_PLUS, L"+", font, &x, y, keyboardButtonWidth, keyboardButtonHeight);

    rightmostX = std::max(x, rightmostX);
    y += keyboardButtonHeight + 1.0f;
    x = numpadSectionStartX;
    CreateKeyboardUIButton(eInputElements::KB_NUMPAD_4, L"4", font, &x, y, keyboardButtonWidth, keyboardButtonHeight);
    CreateKeyboardUIButton(eInputElements::KB_NUMPAD_5, L"5", font, &x, y, keyboardButtonWidth, keyboardButtonHeight);
    CreateKeyboardUIButton(eInputElements::KB_NUMPAD_6, L"6", font, &x, y, keyboardButtonWidth, keyboardButtonHeight);

    rightmostX = std::max(x, rightmostX);
    y += keyboardButtonHeight + 1.0f;
    x = numpadSectionStartX;
    CreateKeyboardUIButton(eInputElements::KB_NUMPAD_1, L"1", font, &x, y, keyboardButtonWidth, keyboardButtonHeight);
    CreateKeyboardUIButton(eInputElements::KB_NUMPAD_2, L"2", font, &x, y, keyboardButtonWidth, keyboardButtonHeight);
    CreateKeyboardUIButton(eInputElements::KB_NUMPAD_3, L"3", font, &x, y, keyboardButtonWidth, keyboardButtonHeight);
    CreateKeyboardUIButton(eInputElements::KB_NUMPAD_ENTER, L"ENTER", font, &x, y, keyboardButtonWidth, keyboardButtonHeight);

    rightmostX = std::max(x, rightmostX);
    y += keyboardButtonHeight + 1.0f;
    x = numpadSectionStartX;
    CreateKeyboardUIButton(eInputElements::KB_NUMPAD_0, L"0", font, &x, y, keyboardButtonWidth, keyboardButtonHeight);
    CreateKeyboardUIButton(eInputElements::KB_NUMPAD_DELETE, L"DEL", font, &x, y, keyboardButtonWidth, keyboardButtonHeight);
}

void InputSystemTest::CreateMouseUI()
{
    ScopedPtr<FTFont> font(FTFont::Create("~res:/Fonts/DejaVuSans.ttf"));
    font->SetSize(10);

    const float32 x = 530;

    mouseHeader = new UIStaticText(Rect(x, 20, 250, 15));
    mouseHeader->SetTextColor(Color::White);
    mouseHeader->SetFont(font);
    mouseHeader->SetTextAlign(ALIGN_LEFT | ALIGN_TOP);
    mouseHeader->SetText(L"Mouse");
    AddControl(mouseHeader);

    mouseBody = new UIButton(Rect(x, 35, 84, 100));
    mouseBody->SetStateFont(0xFF, font);
    mouseBody->SetStateFontColor(0xFF, Color::White);
    mouseBody->GetOrCreateComponent<UIDebugRenderComponent>();
    AddControl(mouseBody);

    UIButton* mousePositionButton = new UIButton(Rect(x, 140, 84, 15));
    mousePositionButton->SetStateFont(0xFF, font);
    mousePositionButton->SetStateFontColor(0xFF, Color::White);
    mousePositionButton->GetOrCreateComponent<UIDebugRenderComponent>();
    mouseButtons[static_cast<uint32>(eInputElements::MOUSE_POSITION)] = mousePositionButton;
    AddControl(mousePositionButton);

    UIButton* leftButton = new UIButton(Rect(x + 15.0f, 35, 20, 70));
    leftButton->SetStateFont(0xFF, font);
    leftButton->SetStateFontColor(0xFF, Color::White);
    leftButton->GetOrCreateComponent<UIDebugRenderComponent>();
    mouseButtons[static_cast<uint32>(eInputElements::MOUSE_LBUTTON)] = leftButton;
    AddControl(leftButton);

    UIButton* rightButton = new UIButton(Rect(x + 45.0f, 35, 20, 70));
    rightButton->SetStateFont(0xFF, font);
    rightButton->SetStateFontColor(0xFF, Color::White);
    rightButton->GetOrCreateComponent<UIDebugRenderComponent>();
    mouseButtons[static_cast<uint32>(eInputElements::MOUSE_RBUTTON)] = rightButton;
    AddControl(rightButton);
}

void InputSystemTest::CreateTouchUI()
{
    ScopedPtr<FTFont> font(FTFont::Create("~res:/Fonts/DejaVuSans.ttf"));
    font->SetSize(10);

    float32 x = 530.0f;
    float32 y = 180.0f;

    touchHeader = new UIStaticText(Rect(x, y, 250, 15));
    touchHeader->SetTextColor(Color::White);
    touchHeader->SetFont(font);
    touchHeader->SetTextAlign(ALIGN_LEFT | ALIGN_TOP);
    touchHeader->SetText(L"Touch");
    AddControl(touchHeader);

    y += 17.0f;

    const float32 clickButtonSizeX = 22.8f;
    const float32 clickButtonSizeY = 20.0f;
    for (size_t i = 0; i < INPUT_ELEMENTS_TOUCH_CLICK_COUNT; ++i)
    {
        std::wstringstream ss;
        ss << i;

        UIButton* touchClickButton = new UIButton(Rect(x, y, clickButtonSizeX, clickButtonSizeY));
        touchClickButton->SetStateFont(0xFF, font);
        touchClickButton->SetStateFontColor(0xFF, Color::White);
        touchClickButton->GetOrCreateComponent<UIDebugRenderComponent>();
        touchClickButton->SetStateText(0xFF, ss.str());
        touchClickButtons[static_cast<uint32>(eInputElements::TOUCH_FIRST_CLICK + i)] = touchClickButton;
        AddControl(touchClickButton);

        UIButton* touchMoveButton = new UIButton(Rect(x, y + clickButtonSizeY + 1.0f, clickButtonSizeX, clickButtonSizeY * 2.0f));
        touchMoveButton->SetStateFont(0xFF, font);
        touchMoveButton->SetStateFontColor(0xFF, Color::White);
        touchMoveButton->GetOrCreateComponent<UIDebugRenderComponent>();
        touchMoveButton->SetStateText(0xFF, L"0\n0");
        touchMoveButton->SetStateTextMultiline(0xFF, true);
        touchMoveButtons[static_cast<uint32>(eInputElements::TOUCH_FIRST_POSITION + i)] = touchMoveButton;
        AddControl(touchMoveButton);

        x += clickButtonSizeX + 1.0f;
    }
}

void InputSystemTest::CreateActionsUI()
{
    ScopedPtr<FTFont> font(FTFont::Create("~res:/Fonts/DejaVuSans.ttf"));
    font->SetSize(9.0f);

    float32 y = 370.0f;
    const float32 yDelta = 30.0f;

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
    ScopedPtr<FTFont> font(FTFont::Create("~res:/Fonts/DejaVuSans.ttf"));
    font->SetSize(11.0f);

    const float32 x = 530.0f;
    float32 y = 370.0f;
    const float32 yDelta = 30.0f;

    inputListenerDigitalSingleWithoutModifiersButton = new UIButton(Rect(x, y, 200, 30));
    inputListenerDigitalSingleWithoutModifiersButton->SetStateFont(0xFF, font);
    inputListenerDigitalSingleWithoutModifiersButton->SetStateFontColor(0xFF, Color::White);
    inputListenerDigitalSingleWithoutModifiersButton->GetOrCreateComponent<UIDebugRenderComponent>();
    inputListenerDigitalSingleWithoutModifiersButton->SetStateText(0xFF, L"Listen: digital single without modifiers");
    inputListenerDigitalSingleWithoutModifiersButton->AddEvent(UIButton::EVENT_TOUCH_UP_INSIDE, Message(this, &InputSystemTest::OnInputListenerButtonPressed));
    AddControl(inputListenerDigitalSingleWithoutModifiersButton);

    y += yDelta;

    inputListenerDigitalSingleWithModifiersButton = new UIButton(Rect(x, y, 200, 30));
    inputListenerDigitalSingleWithModifiersButton->SetStateFont(0xFF, font);
    inputListenerDigitalSingleWithModifiersButton->SetStateFontColor(0xFF, Color::White);
    inputListenerDigitalSingleWithModifiersButton->GetOrCreateComponent<UIDebugRenderComponent>();
    inputListenerDigitalSingleWithModifiersButton->SetStateText(0xFF, L"Listen: digital single with modifiers");
    inputListenerDigitalSingleWithModifiersButton->AddEvent(UIButton::EVENT_TOUCH_UP_INSIDE, Message(this, &InputSystemTest::OnInputListenerButtonPressed));
    AddControl(inputListenerDigitalSingleWithModifiersButton);

    y += yDelta;

    inputListenerDigitalMultipleAnyButton = new UIButton(Rect(x, y, 200, 30));
    inputListenerDigitalMultipleAnyButton->SetStateFont(0xFF, font);
    inputListenerDigitalMultipleAnyButton->SetStateFontColor(0xFF, Color::White);
    inputListenerDigitalMultipleAnyButton->GetOrCreateComponent<UIDebugRenderComponent>();
    inputListenerDigitalMultipleAnyButton->SetStateText(0xFF, L"Listen: digital multiple any");
    inputListenerDigitalMultipleAnyButton->AddEvent(UIButton::EVENT_TOUCH_UP_INSIDE, Message(this, &InputSystemTest::OnInputListenerButtonPressed));
    AddControl(inputListenerDigitalMultipleAnyButton);

    y += yDelta;

    inputListenerAnalogButton = new UIButton(Rect(x, y, 200, 30));
    inputListenerAnalogButton->SetStateFont(0xFF, font);
    inputListenerAnalogButton->SetStateFontColor(0xFF, Color::White);
    inputListenerAnalogButton->GetOrCreateComponent<UIDebugRenderComponent>();
    inputListenerAnalogButton->SetStateText(0xFF, L"Listen: analog");
    inputListenerAnalogButton->AddEvent(UIButton::EVENT_TOUCH_UP_INSIDE, Message(this, &InputSystemTest::OnInputListenerButtonPressed));
    AddControl(inputListenerAnalogButton);

    y += yDelta;

    inputListenerResultField = new UIStaticText(Rect(x, y, 200, 30));
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
    button->GetOrCreateComponent<UIDebugRenderComponent>();

    keyboardButtons[static_cast<uint32>(key)] = button;
    AddControl(button);

    *x = *x + w + 1.0f;
}

void InputSystemTest::HighlightDigitalButton(DAVA::UIButton* button, DAVA::DigitalElementState state)
{
    if (button == nullptr)
    {
        return;
    }

    if (state.IsPressed())
    {
        button->GetOrCreateComponent<UIDebugRenderComponent>()->SetDrawColor(Color(0.0f, 1.0f, 0.0f, 1.0f));
    }
    else
    {
        button->GetOrCreateComponent<UIDebugRenderComponent>()->SetDrawColor(Color(1.0f, 0.0f, 0.0f, 1.0f));
    }
}

bool InputSystemTest::OnInputEvent(InputEvent const& event)
{
    if (event.deviceType == eInputDeviceTypes::KEYBOARD)
    {
        UIButton* scancodeButton = keyboardButtons[event.elementId];

        HighlightDigitalButton(scancodeButton, event.digitalState);
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
    else if (event.deviceType == eInputDeviceTypes::TOUCH_SURFACE)
    {
        if (IsTouchClickInputElement(event.elementId))
        {
            UIButton* touchButton = touchClickButtons[event.elementId];
            HighlightDigitalButton(touchButton, event.digitalState);
        }

        // Position will be changed in Update
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
    DAVA::eInputBindingListenerModes mode;
    if (sender == inputListenerDigitalSingleWithoutModifiersButton)
    {
        mode = DAVA::eInputBindingListenerModes::DIGITAL_SINGLE_WITHOUT_MODIFIERS;
    }
    else if (sender == inputListenerDigitalSingleWithModifiersButton)
    {
        mode = DAVA::eInputBindingListenerModes::DIGITAL_SINGLE_WITH_MODIFIERS;
    }
    else if (sender == inputListenerDigitalMultipleAnyButton)
    {
        mode = DAVA::eInputBindingListenerModes::DIGITAL_MULTIPLE_ANY;
    }
    else
    {
        mode = DAVA::eInputBindingListenerModes::ANALOG;
    }

    GetEngineContext()->inputListener->Listen(mode, MakeFunction(this, &InputSystemTest::OnInputListeningEnded));
    inputListenerResultField->SetText(L"Listening...");
}

void InputSystemTest::OnInputListeningEnded(bool cancelled, DAVA::Vector<DAVA::InputEvent> input)
{
    if (cancelled)
    {
        inputListenerResultField->SetText(L"Stopped listening");
    }
    else
    {
        // Combine input elements into a string

        std::stringstream stringStream;
        for (size_t i = 0; i < input.size(); ++i)
        {
            if (input[i].deviceType == eInputDeviceTypes::KEYBOARD)
            {
                String repr = GetEngineContext()->deviceManager->GetKeyboard()->TranslateElementToUTF8String(input[i].elementId);
                stringStream << repr;
            }
            else
            {
                String repr = GetInputElementInfo(input[i].elementId).name;
                stringStream << repr.c_str();
            }

            if (i != input.size() - 1)
            {
                stringStream << " + ";
            }
        }

        inputListenerResultField->SetUtf8Text(stringStream.str());
    }
}

void InputSystemTest::OnUpdate(float32 delta)
{
    TouchScreen* touch = GetEngineContext()->deviceManager->GetTouchScreen();

    if (touch != nullptr)
    {
        for (size_t i = 0; i < INPUT_ELEMENTS_TOUCH_POSITION_COUNT; ++i)
        {
            eInputElements elementId = static_cast<eInputElements>(eInputElements::TOUCH_FIRST_POSITION + i);

            UIButton* touchButton = touchMoveButtons[elementId];

            AnalogElementState state = touch->GetAnalogElementState(elementId);

            std::wstringstream ss;
            ss << static_cast<int>(state.x) << "\n" << static_cast<int>(state.y);
            touchButton->SetStateText(0xFF, ss.str());
        }
    }
}