#include "Infrastructure/TestBed.h"
#include "Tests/InputSystemTest.h"
#include "Engine/Engine.h"
#include "DeviceManager/DeviceManager.h"

using namespace DAVA;

InputSystemTest::InputSystemTest(TestBed& app)
    : BaseScreen(app, "InputSystemTest")
{
}

void InputSystemTest::LoadResources()
{
    BaseScreen::LoadResources();
	
	CreateKeyboardUI();
	CreateMouseUI();

	Engine::Instance()->update.Connect(this, &InputSystemTest::OnUpdate);
	GetEngineContext()->inputSystem->AddInputEventHandler(MakeFunction(this, &InputSystemTest::OnInputEvent));
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
		button = keyboardButtons[event.controlId];
	}
	else if (event.deviceType == MouseInputDevice::TYPE)
	{
		button = mouseButtons[event.controlId];
	}
	 
	if (button != nullptr)
	{
		if (event.controlId == MouseInputDevice::MOUSE)
		{
			std::wstringstream ss;
			ss << event.analogState.x << L", " << event.analogState.y;
			button->SetStateText(0xFF, ss.str());
		}
		else
		{
			if (event.digitalState.IsPressed())
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

void InputSystemTest::OnUpdate(DAVA::float32 delta)
{
}

UIButton* InputSystemTest::CreateKeyboardUIButton(eKeyboardKey key, WideString text, FTFont* font, float32 x, float32 y, float32 w, float32 h)
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
	CreateKeyboardUIButton(eKeyboardKey::GRAVE, L"`", font, x += keyboardButtonWidth + 1, y, keyboardButtonWidth, keyboardButtonHeight);
	CreateKeyboardUIButton(eKeyboardKey::KEY_1, L"1", font, x += keyboardButtonWidth + 1, y, keyboardButtonWidth, keyboardButtonHeight);
	CreateKeyboardUIButton(eKeyboardKey::KEY_2, L"2", font, x += keyboardButtonWidth + 1, y, keyboardButtonWidth, keyboardButtonHeight);
	CreateKeyboardUIButton(eKeyboardKey::KEY_3, L"3", font, x += keyboardButtonWidth + 1, y, keyboardButtonWidth, keyboardButtonHeight);
	CreateKeyboardUIButton(eKeyboardKey::KEY_4, L"4", font, x += keyboardButtonWidth + 1, y, keyboardButtonWidth, keyboardButtonHeight);
	CreateKeyboardUIButton(eKeyboardKey::KEY_5, L"5", font, x += keyboardButtonWidth + 1, y, keyboardButtonWidth, keyboardButtonHeight);
	CreateKeyboardUIButton(eKeyboardKey::KEY_6, L"6", font, x += keyboardButtonWidth + 1, y, keyboardButtonWidth, keyboardButtonHeight);
	CreateKeyboardUIButton(eKeyboardKey::KEY_7, L"7", font, x += keyboardButtonWidth + 1, y, keyboardButtonWidth, keyboardButtonHeight);
	CreateKeyboardUIButton(eKeyboardKey::KEY_8, L"8", font, x += keyboardButtonWidth + 1, y, keyboardButtonWidth, keyboardButtonHeight);
	CreateKeyboardUIButton(eKeyboardKey::KEY_9, L"9", font, x += keyboardButtonWidth + 1, y, keyboardButtonWidth, keyboardButtonHeight);
	CreateKeyboardUIButton(eKeyboardKey::KEY_0, L"0", font, x += keyboardButtonWidth + 1, y, keyboardButtonWidth, keyboardButtonHeight);
	CreateKeyboardUIButton(eKeyboardKey::MINUS, L"-", font, x += keyboardButtonWidth + 1, y, keyboardButtonWidth, keyboardButtonHeight);
	CreateKeyboardUIButton(eKeyboardKey::EQUALS, L"+", font, x += keyboardButtonWidth + 1, y, keyboardButtonWidth, keyboardButtonHeight);
	CreateKeyboardUIButton(eKeyboardKey::BACKSPACE, L"<-", font, x += keyboardButtonWidth + 1, y, keyboardButtonWidth, keyboardButtonHeight);

	y = 50.0f;
	x = 0.0f;
	CreateKeyboardUIButton(eKeyboardKey::TAB, L"TAB", font, x += keyboardButtonWidth + 1, y, keyboardButtonWidth, keyboardButtonHeight);
	CreateKeyboardUIButton(eKeyboardKey::KEY_Q, L"Q", font, x += keyboardButtonWidth + 1, y, keyboardButtonWidth, keyboardButtonHeight);
	CreateKeyboardUIButton(eKeyboardKey::KEY_W, L"W", font, x += keyboardButtonWidth + 1, y, keyboardButtonWidth, keyboardButtonHeight);
	CreateKeyboardUIButton(eKeyboardKey::KEY_E, L"E", font, x += keyboardButtonWidth + 1, y, keyboardButtonWidth, keyboardButtonHeight);
	CreateKeyboardUIButton(eKeyboardKey::KEY_R, L"R", font, x += keyboardButtonWidth + 1, y, keyboardButtonWidth, keyboardButtonHeight);
	CreateKeyboardUIButton(eKeyboardKey::KEY_T, L"T", font, x += keyboardButtonWidth + 1, y, keyboardButtonWidth, keyboardButtonHeight);
	CreateKeyboardUIButton(eKeyboardKey::KEY_Y, L"Y", font, x += keyboardButtonWidth + 1, y, keyboardButtonWidth, keyboardButtonHeight);
	CreateKeyboardUIButton(eKeyboardKey::KEY_U, L"U", font, x += keyboardButtonWidth + 1, y, keyboardButtonWidth, keyboardButtonHeight);
	CreateKeyboardUIButton(eKeyboardKey::KEY_I, L"I", font, x += keyboardButtonWidth + 1, y, keyboardButtonWidth, keyboardButtonHeight);
	CreateKeyboardUIButton(eKeyboardKey::KEY_O, L"O", font, x += keyboardButtonWidth + 1, y, keyboardButtonWidth, keyboardButtonHeight);
	CreateKeyboardUIButton(eKeyboardKey::KEY_P, L"P", font, x += keyboardButtonWidth + 1, y, keyboardButtonWidth, keyboardButtonHeight);
	CreateKeyboardUIButton(eKeyboardKey::LBRACKET, L"{", font, x += keyboardButtonWidth + 1, y, keyboardButtonWidth, keyboardButtonHeight);
	CreateKeyboardUIButton(eKeyboardKey::RBRACKET, L"}", font, x += keyboardButtonWidth + 1, y, keyboardButtonWidth, keyboardButtonHeight);

	y = 80.0f;
	x = 0.0f;
	CreateKeyboardUIButton(eKeyboardKey::CAPSLOCK, L"CAPS", font, x += keyboardButtonWidth + 1, y, keyboardButtonWidth, keyboardButtonHeight);
	CreateKeyboardUIButton(eKeyboardKey::KEY_A, L"A", font, x += keyboardButtonWidth + 1, y, keyboardButtonWidth, keyboardButtonHeight);
	CreateKeyboardUIButton(eKeyboardKey::KEY_S, L"S", font, x += keyboardButtonWidth + 1, y, keyboardButtonWidth, keyboardButtonHeight);
	CreateKeyboardUIButton(eKeyboardKey::KEY_D, L"D", font, x += keyboardButtonWidth + 1, y, keyboardButtonWidth, keyboardButtonHeight);
	CreateKeyboardUIButton(eKeyboardKey::KEY_F, L"F", font, x += keyboardButtonWidth + 1, y, keyboardButtonWidth, keyboardButtonHeight);
	CreateKeyboardUIButton(eKeyboardKey::KEY_G, L"G", font, x += keyboardButtonWidth + 1, y, keyboardButtonWidth, keyboardButtonHeight);
	CreateKeyboardUIButton(eKeyboardKey::KEY_H, L"H", font, x += keyboardButtonWidth + 1, y, keyboardButtonWidth, keyboardButtonHeight);
	CreateKeyboardUIButton(eKeyboardKey::KEY_J, L"J", font, x += keyboardButtonWidth + 1, y, keyboardButtonWidth, keyboardButtonHeight);
	CreateKeyboardUIButton(eKeyboardKey::KEY_K, L"K", font, x += keyboardButtonWidth + 1, y, keyboardButtonWidth, keyboardButtonHeight);
	CreateKeyboardUIButton(eKeyboardKey::KEY_L, L"L", font, x += keyboardButtonWidth + 1, y, keyboardButtonWidth, keyboardButtonHeight);
	CreateKeyboardUIButton(eKeyboardKey::SEMICOLON, L":", font, x += keyboardButtonWidth + 1, y, keyboardButtonWidth, keyboardButtonHeight);
	CreateKeyboardUIButton(eKeyboardKey::APOSTROPHE, L"\"", font, x += keyboardButtonWidth + 1, y, keyboardButtonWidth, keyboardButtonHeight);
	CreateKeyboardUIButton(eKeyboardKey::BACKSLASH, L"\\", font, x += keyboardButtonWidth + 1, y, keyboardButtonWidth, keyboardButtonHeight);

	y = 110.0f;
	x = 0.0f;
	CreateKeyboardUIButton(eKeyboardKey::LSHIFT, L"SHFT", font, x += keyboardButtonWidth + 1, y, keyboardButtonWidth, keyboardButtonHeight);
	CreateKeyboardUIButton(eKeyboardKey::NONUSBACKSLASH, L"\\", font, x += keyboardButtonWidth + 1, y, keyboardButtonWidth, keyboardButtonHeight);
	CreateKeyboardUIButton(eKeyboardKey::KEY_Z, L"Z", font, x += keyboardButtonWidth + 1, y, keyboardButtonWidth, keyboardButtonHeight);
	CreateKeyboardUIButton(eKeyboardKey::KEY_X, L"X", font, x += keyboardButtonWidth + 1, y, keyboardButtonWidth, keyboardButtonHeight);
	CreateKeyboardUIButton(eKeyboardKey::KEY_C, L"C", font, x += keyboardButtonWidth + 1, y, keyboardButtonWidth, keyboardButtonHeight);
	CreateKeyboardUIButton(eKeyboardKey::KEY_V, L"V", font, x += keyboardButtonWidth + 1, y, keyboardButtonWidth, keyboardButtonHeight);
	CreateKeyboardUIButton(eKeyboardKey::KEY_B, L"B", font, x += keyboardButtonWidth + 1, y, keyboardButtonWidth, keyboardButtonHeight);
	CreateKeyboardUIButton(eKeyboardKey::KEY_N, L"N", font, x += keyboardButtonWidth + 1, y, keyboardButtonWidth, keyboardButtonHeight);
	CreateKeyboardUIButton(eKeyboardKey::KEY_M, L"M", font, x += keyboardButtonWidth + 1, y, keyboardButtonWidth, keyboardButtonHeight);
	CreateKeyboardUIButton(eKeyboardKey::COMMA, L"<", font, x += keyboardButtonWidth + 1, y, keyboardButtonWidth, keyboardButtonHeight);
	CreateKeyboardUIButton(eKeyboardKey::PERIOD, L">", font, x += keyboardButtonWidth + 1, y, keyboardButtonWidth, keyboardButtonHeight);
	CreateKeyboardUIButton(eKeyboardKey::SLASH, L"?", font, x += keyboardButtonWidth + 1, y, keyboardButtonWidth, keyboardButtonHeight);
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
	mouseButtons[static_cast<uint32>(MouseInputDevice::MOUSE)] = mousePositionButton;

	AnalogControlState mousePos = GetEngineContext()->deviceManager->GetMouse()->GetAnalogControlState(MouseInputDevice::MOUSE);
	std::wstringstream ss;
	ss << mousePos.x << L", " << mousePos.y;
	mousePositionButton->SetStateText(0xFF, ss.str());

	AddControl(mousePositionButton);

	UIButton* leftButton = new UIButton(Rect(700, 10, 30, 70));
	leftButton->SetStateFont(0xFF, font);
	leftButton->SetStateFontColor(0xFF, Color::White);
	leftButton->SetDebugDraw(true);
	mouseButtons[static_cast<uint32>(MouseInputDevice::LEFT_BUTTON)] = leftButton;
	AddControl(leftButton);

	UIButton* rightButton = new UIButton(Rect(732, 10, 30, 70));
	rightButton->SetStateFont(0xFF, font);
	rightButton->SetStateFontColor(0xFF, Color::White);
	rightButton->SetDebugDraw(true);
	mouseButtons[static_cast<uint32>(MouseInputDevice::RIGHT_BUTTON)] = rightButton;
	AddControl(rightButton);
}