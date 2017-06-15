#include "Tests/MicroWebBrowserTest.h"
#include "UI/Focus/UIFocusComponent.h"
#include "UI/Render/UIDebugRenderComponent.h"

MicroWebBrowserTest::MicroWebBrowserTest(TestBed& app)
    : BaseScreen(app, "MicroWebBrowserTest")
{
}

void MicroWebBrowserTest::LoadResources()
{
    ScopedPtr<Font> font(FTFont::Create("~res:/Fonts/korinna.ttf"));
    DVASSERT(font);
    font->SetSize(14);

    Rect screenRect = GetRect();

    Rect textFieldRect;
    textFieldRect.x = 10.0f;
    textFieldRect.y = 10.0f;
    textFieldRect.dx = screenRect.dx - 210.0f;
    textFieldRect.dy = 50.0f;

    textField.Set(new UITextField(textFieldRect));
    textField->SetFont(font);
    textField->GetOrCreateComponent<UIDebugRenderComponent>();
    textField->SetTextColor(Color(0.0, 1.0, 0.0, 1.0));
    textField->GetOrCreateComponent<UIFocusComponent>();
    textField->SetTextAlign(ALIGN_LEFT | ALIGN_VCENTER);
    textField->SetDelegate(this);
    AddControl(textField.Get());

    Rect loadPageRect = textFieldRect;
    loadPageRect.x += textFieldRect.dx + 10.0f;
    loadPageRect.dx = 200.0f;

    ScopedPtr<UIButton> loadPage(new UIButton(loadPageRect));
    loadPage->SetStateFont(0xFF, font);
    loadPage->SetStateFontColor(0xFF, Color::White);
    loadPage->SetStateText(0xFF, L"Load");
    loadPage->GetOrCreateComponent<UIDebugRenderComponent>();
    loadPage->AddEvent(UIButton::EVENT_TOUCH_DOWN, Message(this, &MicroWebBrowserTest::OnLoadPage));
    AddControl(loadPage);

    Rect webViewRect;
    webViewRect.x = textFieldRect.x;
    webViewRect.y = textFieldRect.y + textFieldRect.dy + 5.0f;
    webViewRect.dx = (loadPageRect.x + loadPageRect.dx) - webViewRect.x;
    webViewRect.dy = screenRect.dy * 0.8f;

    webView.Set(new UIWebView(webViewRect));
    webView->SetVisibilityFlag(true);
    webView->SetRenderToTexture(true);
    webView->GetOrCreateComponent<UIDebugRenderComponent>();
    webView->SetInputEnabled(true);
    webView->GetOrCreateComponent<UIFocusComponent>();
    AddControl(webView.Get());

    BaseScreen::LoadResources();
}

void MicroWebBrowserTest::UnloadResources()
{
    webView = nullptr;
    textField = nullptr;
    BaseScreen::UnloadResources();
}

void MicroWebBrowserTest::OnLoadPage(BaseObject* obj, void* data, void* callerData)
{
    WideString url = textField->GetText();
    if (!url.empty())
    {
        webView->OpenURL(UTF8Utils::EncodeToUTF8(url));
    }
}

void MicroWebBrowserTest::TextFieldShouldReturn(UITextField* /*textField*/)
{
    OnLoadPage(nullptr, nullptr, nullptr);
}
