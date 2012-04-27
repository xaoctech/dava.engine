#include "TextureTrianglesDialog.h"
#include "ControlsFactory.h"
    
TextureTrianglesDialog::TextureTrianglesDialog()
    :   ExtendedDialog()
{
    workingPolygonGroup = NULL;
    previewSprite = NULL;

    Rect rect = DialogRect();
    draggableDialog->SetRect(rect);

    rect.x = rect.y = 0;
    
    Vector<String> comboboxItems;
    comboboxItems.push_back("0");
    comboboxItems.push_back("1");
    comboboxItems.push_back("2");
    comboboxItems.push_back("3");
    
    Rect comboRect = rect;
    comboRect.dy = ControlsFactory::BUTTON_HEIGHT;
    combobox = new ComboBox(comboRect, this, comboboxItems);
    draggableDialog->AddControl(combobox);
    
    Rect previewRect = rect;
    previewRect.y = ControlsFactory::BUTTON_HEIGHT;
    previewRect.dy -= ControlsFactory::BUTTON_HEIGHT * 2;
    texturePreview = new UIControl(previewRect);
    texturePreview->SetInputEnabled(false);
    draggableDialog->AddControl(texturePreview);
    
    float32 buttonX = (rect.dx - ControlsFactory::BUTTON_WIDTH) / 2;
    float32 buttonY = rect.dy - ControlsFactory::BUTTON_HEIGHT;
    UIButton *btnCancel = ControlsFactory::CreateButton(Vector2(buttonX, buttonY), LocalizedString(L"dialog.close"));
    btnCancel->AddEvent(UIControl::EVENT_TOUCH_UP_INSIDE, Message(this, &TextureTrianglesDialog::OnClose));
    draggableDialog->AddControl(btnCancel);
    SafeRelease(btnCancel);
}

TextureTrianglesDialog::~TextureTrianglesDialog()
{
    SafeRelease(workingPolygonGroup);
    SafeRelease(previewSprite);
    SafeRelease(texturePreview);
    SafeRelease(combobox);
}

const Rect TextureTrianglesDialog::DialogRect()
{
    Rect rect = GetRect();
    
    if(rect.dx < rect.dy)
    {
        rect.dy = rect.dx + ControlsFactory::BUTTON_HEIGHT * 2;
    }
    else
    {
        rect.dx = rect.dy - ControlsFactory::BUTTON_HEIGHT * 2;
    }
    
    rect.y = (GetRect().dy - rect.dy) / 2;
    rect.x = (GetRect().dx - rect.dx) / 2;
    
    return rect;
}

void TextureTrianglesDialog::Close()
{
    SafeRelease(workingPolygonGroup);
    
    ExtendedDialog::Close();
}


void TextureTrianglesDialog::OnClose(BaseObject * owner, void * userData, void * callerData)
{
    Close();
}


void TextureTrianglesDialog::Show(PolygonGroup *polygonGroup)
{
    if(!GetParent())
    {
        InitWithData(polygonGroup);
        
        UIScreenManager::Instance()->GetScreen()->AddControl(this);
    }
}

void TextureTrianglesDialog::InitWithData(PolygonGroup *pg)
{
    SafeRelease(workingPolygonGroup);
    workingPolygonGroup = SafeRetain(pg);
    
    Vector<String> comboboxItems;
    if(workingPolygonGroup)
    {
        for(int32 i = 0; i < workingPolygonGroup->textureCoordCount; ++i)
        {
            comboboxItems.push_back(Format("%d", i));
        }
    }
    else
    {
        comboboxItems.push_back("No coordinates");
    }
    combobox->SetNewItemsSet(comboboxItems);
    
    combobox->SetSelectedIndex(0, true);
}

void TextureTrianglesDialog::ConvertCoordinates(Vector2 &coordinates, const Vector2 &boxSize)
{
    coordinates.x = (coordinates.x + 0.5f) * boxSize.x;
    coordinates.y = (coordinates.y + 0.5f) * boxSize.y;
}


void TextureTrianglesDialog::FillRenderTarget(int32 textureIndex)
{
    SafeRelease(previewSprite);
    
    if(workingPolygonGroup)
    {
        const int32 IndexFlags[] = {EVF_TEXCOORD0, EVF_TEXCOORD1, EVF_TEXCOORD2, EVF_TEXCOORD3};
        
        if(workingPolygonGroup->vertexFormat & IndexFlags[textureIndex])
        {
            Vector2 usedSize = texturePreview->GetSize();
            previewSprite = Sprite::CreateAsRenderTarget(usedSize.x, usedSize.y, FORMAT_RGBA8888);
            
            RenderManager::Instance()->SetRenderTarget(previewSprite);
            RenderManager::Instance()->SetColor(Color::Black());
            RenderHelper::Instance()->FillRect(Rect(0, 0, usedSize.x, usedSize.y));
            
            usedSize.x = Min(usedSize.x, usedSize.y);
            usedSize.y = Min(usedSize.x, usedSize.y);
            
            usedSize /= 2.0f;
            
            RenderManager::Instance()->SetColor(Color(1.f, 0.f, 0.f, 1.f));
            Vector2 lt(0, 0), lb(0, 1), rt(1, 0), rb(1, 1);
            ConvertCoordinates(lt, usedSize);
            ConvertCoordinates(lb, usedSize);
            ConvertCoordinates(rt, usedSize);
            ConvertCoordinates(rb, usedSize);

            
            RenderHelper::Instance()->DrawLine(lt, lb);
            RenderHelper::Instance()->DrawLine(lt, rt);
            RenderHelper::Instance()->DrawLine(lb, rb);
            RenderHelper::Instance()->DrawLine(rt, rb);
            
            
            RenderManager::Instance()->SetColor(Color(0.f, 1.f, 0.f, 1.f));
            int32 indexCount = workingPolygonGroup->GetIndexCount();
            for(int32 i = 0; i < indexCount - 3; i += 3)
            {
                int32 index0 = 0, index1 = 0, index2 = 0;
                workingPolygonGroup->GetIndex(i + 0, index0);
                workingPolygonGroup->GetIndex(i + 1, index1);
                workingPolygonGroup->GetIndex(i + 2, index2);
                
                Vector2 v0, v1, v2;
                workingPolygonGroup->GetTexcoord(textureIndex, index0, v0);
                workingPolygonGroup->GetTexcoord(textureIndex, index1, v1);
                workingPolygonGroup->GetTexcoord(textureIndex, index2, v2);
             
                ConvertCoordinates(v0, usedSize);
                ConvertCoordinates(v1, usedSize);
                ConvertCoordinates(v2, usedSize);

                RenderHelper::Instance()->DrawLine(v0, v1);
                RenderHelper::Instance()->DrawLine(v1, v2);
                RenderHelper::Instance()->DrawLine(v0, v2);
            }
            
            
            RenderManager::Instance()->RestoreRenderTarget();
        }
    }
    
    texturePreview->SetSprite(previewSprite, 0);
}

void TextureTrianglesDialog::OnItemSelected(ComboBox *forComboBox, const String &itemKey, int itemIndex)
{
    FillRenderTarget(itemIndex);
}

