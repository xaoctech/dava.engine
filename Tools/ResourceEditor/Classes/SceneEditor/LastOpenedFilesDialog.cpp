#include "LastOpenedFilesDialog.h"

#include "ControlsFactory.h"

#include "EditorSettings.h"

LastOpenedFilesDialog::LastOpenedFilesDialog(LastOpenedFilesDialogDelegate *newDelegate)
    :   ExtendedDialog()
    ,   delegate(newDelegate)
{
    draggableDialog->SetRect(DialogRect());
    
    Rect rect = DialogRect();
    
    filesList = new UIList(Rect(0, ControlsFactory::OFFSET, 
                                rect.dx, CellHeight(NULL, 0) * EditorSettings::RESENT_FILES_COUNT), //maxlist height 
                           UIList::ORIENTATION_VERTICAL);
    
    ControlsFactory::SetScrollbar(filesList);
    filesList->SetDelegate(this);
    draggableDialog->AddControl(filesList);
    
    float32 buttonX = (rect.dx - ControlsFactory::BUTTON_WIDTH) / 2;
    float32 buttonY = rect.dy - ControlsFactory::BUTTON_HEIGHT;
    closeButton = ControlsFactory::CreateButton(Vector2(buttonX, buttonY), LocalizedString(L"dialog.close"));
    closeButton->AddEvent(UIControl::EVENT_TOUCH_UP_INSIDE, Message(this, &LastOpenedFilesDialog::OnCancel));
    draggableDialog->AddControl(closeButton);
}

LastOpenedFilesDialog::~LastOpenedFilesDialog()
{
    SafeRelease(closeButton);
    SafeRelease(filesList);
}

int32 LastOpenedFilesDialog::ElementsCount(UIList * list)
{
    return EditorSettings::Instance()->GetLastOpenedCount();
}

UIListCell *LastOpenedFilesDialog::CellAtIndex(UIList *list, int32 index)
{
    UIListCell *c = list->GetReusableCell("LastOpenedFiles cell"); 
    if(!c)
    {
        c = new UIListCell(Rect(0, 0, list->GetRect().dx, ControlsFactory::BUTTON_HEIGHT), "LastOpenedFiles cell");
    }

    ControlsFactory::CustomizeMenuPopupCell(c, StringToWString(EditorSettings::Instance()->GetLastOpenedFile(index)));
    
    return c;
}

int32 LastOpenedFilesDialog::CellHeight(UIList * list, int32 index)
{
    return ControlsFactory::BUTTON_HEIGHT;
}

void LastOpenedFilesDialog::Show()
{
    if(!GetParent())
    {
        filesList->Refresh();
        
        Rect r = DialogRect();
        draggableDialog->SetRect(r);
        
        Vector2 pos = closeButton->GetPosition();
        pos.y = r.dy - ControlsFactory::BUTTON_HEIGHT;
        closeButton->SetPosition(pos);
        
        UIScreen *screen = UIScreenManager::Instance()->GetScreen();
        screen->AddControl(this);
    }
}


const Rect LastOpenedFilesDialog::DialogRect()
{
    Rect baseRect(GetRect().dx/8, GetRect().dy/4, GetRect().dx * 3 / 4, GetRect().dy/2);
    
    if(EditorSettings::Instance()->GetLastOpenedCount())
    {
        int32 height = (EditorSettings::Instance()->GetLastOpenedCount()) * CellHeight(NULL, 0) 
                            + ControlsFactory::OFFSET
                            + ControlsFactory::BUTTON_HEIGHT;
        
        if(height< baseRect.dy)
        {
            baseRect.dy = height;
            baseRect.y = (GetRect().dy - baseRect.dy) / 2;
        }
    }
    
    return baseRect;
}

void LastOpenedFilesDialog::OnCancel(BaseObject * owner, void * userData, void * callerData)
{
    Close();
}

void LastOpenedFilesDialog::OnCellSelected(UIList *forList, UIListCell *selectedCell)
{
    if(delegate)
    {
        int32 index = selectedCell->GetIndex();
        delegate->OnLastFileSelected(EditorSettings::Instance()->GetLastOpenedFile(index));
    }
    Close();
}

