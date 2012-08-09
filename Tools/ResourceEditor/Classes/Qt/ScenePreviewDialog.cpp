#include "ScenePreviewDialog.h"

#include "../SceneEditor/ControlsFactory.h"
#include "../SceneEditor/ScenePreviewControl.h"

ScenePreviewDialog::ScenePreviewDialog()
    :   ExtendedDialog()
{
    draggableDialog->SetRect(DialogRect());
    
    fontLight = ControlsFactory::GetFontLight();
    fontDark = ControlsFactory::GetFontDark();
    
    preview = new ScenePreviewControl(Rect(0, 0, ControlsFactory::PREVIEW_PANEL_HEIGHT, ControlsFactory::PREVIEW_PANEL_HEIGHT));
    preview->SetDebugDraw(true);
    
    errorMessage = new UIStaticText(preview->GetRect());
    errorMessage->SetMultiline(true);
    errorMessage->SetAlign(ALIGN_HCENTER | ALIGN_VCENTER);
    errorMessage->SetFont(ControlsFactory::GetFontError());
    
    UIButton *b = ControlsFactory::CreateButton(Rect(0, ControlsFactory::PREVIEW_PANEL_HEIGHT,
                                                     ControlsFactory::PREVIEW_PANEL_HEIGHT, ControlsFactory::BUTTON_HEIGHT),
                                                LocalizedString(L"dialog.close"));
    b->AddEvent(UIControl::EVENT_TOUCH_UP_INSIDE, Message(this, &ScenePreviewDialog::OnClose));
    draggableDialog->AddControl(b);
    SafeRelease(b);
}
    
ScenePreviewDialog::~ScenePreviewDialog()
{
    SafeRelease(errorMessage);
    SafeRelease(preview);
}


void ScenePreviewDialog::Show(const String &scenePathname)
{
    if(!GetParent())
    {
        UIScreen *screen = UIScreenManager::Instance()->GetScreen();
        screen->AddControl(this);
    }
    
    
    //show preview
    if(errorMessage->GetParent())
    {
        draggableDialog->RemoveControl(errorMessage);
    }
    
    int32 error = preview->OpenScene(scenePathname);
    if(SceneFileV2::ERROR_NO_ERROR == error)
    {
        if(!preview->GetParent())
        {
            draggableDialog->AddControl(preview);
        }
    }
    else
    {
        switch (error)
        {
            case SceneFileV2::ERROR_FAILED_TO_CREATE_FILE:
            {
                errorMessage->SetText(LocalizedString(L"library.errormessage.failedtocreeatefile"));
                break;
            }
                
            case SceneFileV2::ERROR_FILE_WRITE_ERROR:
            {
                errorMessage->SetText(LocalizedString(L"library.errormessage.filewriteerror"));
                break;
            }
                
            case SceneFileV2::ERROR_VERSION_IS_TOO_OLD:
            {
                errorMessage->SetText(LocalizedString(L"library.errormessage.versionistooold"));
                break;
            }
                
            case ScenePreviewControl::ERROR_CANNOT_OPEN_FILE:
            {
                errorMessage->SetText(LocalizedString(L"library.errormessage.cannotopenfile"));
                break;
            }
                
            case ScenePreviewControl::ERROR_WRONG_EXTENSION:
            {
                errorMessage->SetText(LocalizedString(L"library.errormessage.wrongextension"));
                break;
            }
                
            default:
                errorMessage->SetText(LocalizedString(L"library.errormessage.unknownerror"));
                break;
        }
        
        draggableDialog->AddControl(errorMessage);
    }
}

void ScenePreviewDialog::OnClose(BaseObject *, void *, void *)
{
    preview->ReleaseScene();
    preview->RecreateScene();
    
    Close();
}


const Rect ScenePreviewDialog::DialogRect()
{
    int32 x = (GetRect().dx - ControlsFactory::PREVIEW_PANEL_HEIGHT);
    int32 h = ControlsFactory::PREVIEW_PANEL_HEIGHT + ControlsFactory::BUTTON_HEIGHT;
    int32 y = (GetRect().dy - h) / 2;
    
    return Rect(x, y, ControlsFactory::PREVIEW_PANEL_HEIGHT, h);
}
