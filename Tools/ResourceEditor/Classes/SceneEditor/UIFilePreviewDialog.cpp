#include "UIFilePreviewDialog.h"

#pragma mark  --UIFilePreviewDialog
UIFilePreviewDialog::UIFilePreviewDialog(const String &_fontPath)
    : UIFileSystemDialog(_fontPath)
{
    Rect rect = GetRect();

    preview = new UIControl(Rect(rect.dx, 0, PREVIEW_WIDTH, rect.dy));
    preview->GetBackground()->SetDrawType(UIControlBackground::DRAW_SCALE_PROPORTIONAL);
    
    rect.dx += PREVIEW_WIDTH;
    rect.x -= PREVIEW_WIDTH/2;
    
    SetRect(rect);
}


UIFilePreviewDialog::~UIFilePreviewDialog()
{
    SafeRelease(preview);
}

void UIFilePreviewDialog::UpdatePreview(int32 unitIndex)
{
    Texture *tex = NULL;
    Sprite *sprite = NULL;
    
    if((0 <= unitIndex) && unitIndex < fileUnits.size())
    {
        int32 fileIndex = fileUnits[unitIndex].indexInFileList;
        if((0 <= fileIndex) && fileIndex < files->GetCount())
        {
            String fileName = files->GetPathname(fileIndex);
            tex = Texture::CreateFromFile(fileName);
            if(tex)
            {
                sprite = Sprite::CreateFromTexture(tex, 0, 0, tex->GetWidth(), tex->GetHeight());
            }
        }
    }
    
    if(sprite)
    {
        preview->SetSprite(sprite, 0);
        if(!preview->GetParent())
        {
            AddControl(preview);
        }
    }
    else 
    {
        if(preview->GetParent())
        {
            RemoveControl(preview);
        }
    }
    
    SafeRelease(sprite);
    SafeRelease(tex);
}

#pragma mark  --UIListDelegate
UIListCell *UIFilePreviewDialog::CellAtIndex(UIList *forList, int32 index)
{
    UIListCell *c = UIFileSystemDialog::CellAtIndex(forList, index);
    
    if(index == lastSelectedIndex)
    {
        UpdatePreview(index);
    }
    
    return c;
}

void UIFilePreviewDialog::OnCellSelected(DAVA::UIList *forList, DAVA::UIListCell *selectedCell)
{
    UIFileSystemDialog::OnCellSelected(forList, selectedCell);

    UpdatePreview(selectedCell->GetIndex());
}

