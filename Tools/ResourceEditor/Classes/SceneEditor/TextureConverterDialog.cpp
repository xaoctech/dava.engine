#include "TextureConverterDialog.h"
#include "ControlsFactory.h"

#include "TextureConverterCell.h"

#include "ErrorNotifier.h"
#include "PVRConverter.h"

#include "UIZoomControl.h"

TextureConverterDialog::TextureConverterDialog(const Rect &rect, bool rectInAbsoluteCoordinates)
    :   UIControl(rect, rectInAbsoluteCoordinates)
{
    selectedItem = -1;
    
    workingScene = NULL;
    
    ControlsFactory::CustomizePanelControl(this);
    
    textureList = new UIList(Rect(0, 0, ControlsFactory::TEXTURE_PREVIEW_WIDTH, rect.dy), 
                             UIList::ORIENTATION_VERTICAL);
    ControlsFactory::SetScrollbar(textureList);
    textureList->SetDelegate(this);
    AddControl(textureList);
    
    float32 closeButtonSide = ControlsFactory::BUTTON_HEIGHT;
    closeButtonTop = ControlsFactory::CreateCloseWindowButton(Rect(rect.dx - closeButtonSide, 0, closeButtonSide, closeButtonSide));
    closeButtonTop->AddEvent(UIControl::EVENT_TOUCH_UP_INSIDE, Message(this, &TextureConverterDialog::OnCancel));
    AddControl(closeButtonTop);
    

    int32 x = rect.dx - ControlsFactory::BUTTON_WIDTH;
    convertButton = ControlsFactory::CreateButton(Vector2(x, 
                                                          rect.dy - ControlsFactory::BUTTON_HEIGHT), 
                                                  LocalizedString(L"textureconverter.convert"));
    convertButton->AddEvent(UIControl::EVENT_TOUCH_UP_INSIDE, Message(this, &TextureConverterDialog::OnConvert));
    
    AddControl(convertButton);
    
    AddLine(Rect(ControlsFactory::TEXTURE_PREVIEW_WIDTH, 0, 1, rect.dy));
    
    
    dstPreview = new UIControl(Rect(0, 0, 100, 100));
    dstPreview->SetInputEnabled(false);
    dstPreview->GetBackground()->SetDrawType(UIControlBackground::DRAW_SCALE_PROPORTIONAL);
    srcPreview = new UIControl(Rect(0, 0, 100, 100));
    srcPreview->SetInputEnabled(false);
    srcPreview->GetBackground()->SetDrawType(UIControlBackground::DRAW_SCALE_PROPORTIONAL);

    int32 width = rect.dx - ControlsFactory::TEXTURE_PREVIEW_WIDTH;
    int32 height = (rect.dy - ControlsFactory::BUTTON_HEIGHT*2);
    
    if(width < height)
    {
        width = Min(width, height/2);
        --width;

        Rect srcRect(ControlsFactory::TEXTURE_PREVIEW_WIDTH, ControlsFactory::BUTTON_HEIGHT, width, width);
        Rect dstRect(ControlsFactory::TEXTURE_PREVIEW_WIDTH, ControlsFactory::BUTTON_HEIGHT + width + 1, width, width);
        
        srcZoomPreview = new UIZoomControl(srcRect);
        dstZoomPreview = new UIZoomControl(dstRect);
        
        AddLine(Rect(ControlsFactory::TEXTURE_PREVIEW_WIDTH+width, ControlsFactory::BUTTON_HEIGHT, 1, width * 2));
        AddLine(Rect(ControlsFactory::TEXTURE_PREVIEW_WIDTH, ControlsFactory::BUTTON_HEIGHT + width, width, 1));
        AddLine(Rect(ControlsFactory::TEXTURE_PREVIEW_WIDTH, ControlsFactory::BUTTON_HEIGHT + width*2, width, 1));
    }
    else 
    {
        height = Min(height, width/2);
        --height;
        Rect srcRect(ControlsFactory::TEXTURE_PREVIEW_WIDTH, ControlsFactory::BUTTON_HEIGHT, height, height);
        Rect dstRect(ControlsFactory::TEXTURE_PREVIEW_WIDTH + height + 1, ControlsFactory::BUTTON_HEIGHT, height, height);
        
        srcZoomPreview = new UIZoomControl(srcRect);
        dstZoomPreview = new UIZoomControl(dstRect);
        
        AddLine(Rect(ControlsFactory::TEXTURE_PREVIEW_WIDTH, ControlsFactory::BUTTON_HEIGHT + height, height*2, 1));
        
        AddLine(Rect(ControlsFactory::TEXTURE_PREVIEW_WIDTH + height, ControlsFactory::BUTTON_HEIGHT, 1, height));
        AddLine(Rect(ControlsFactory::TEXTURE_PREVIEW_WIDTH + height*2, ControlsFactory::BUTTON_HEIGHT, 1, height));
    }
    
    srcZoomPreview->AddControl(srcPreview);
    dstZoomPreview->AddControl(dstPreview);

    zoomSlider = new UISlider(Rect(ControlsFactory::TEXTURE_PREVIEW_WIDTH, rect.dy - ControlsFactory::BUTTON_HEIGHT, 
                                   rect.dx - ControlsFactory::TEXTURE_PREVIEW_WIDTH - ControlsFactory::BUTTON_WIDTH, 
                                   ControlsFactory::BUTTON_HEIGHT));
    zoomSlider->AddEvent(UIControl::EVENT_VALUE_CHANGED, Message(this, &TextureConverterDialog::OnZoomChanged));
    zoomSlider->SetMinSprite("~res:/Gfx/LandscapeEditor/Tools/polzunok", 1);
    zoomSlider->SetMinDrawType(UIControlBackground::DRAW_STRETCH_HORIZONTAL);
    zoomSlider->SetMinLeftRightStretchCap(5);
    zoomSlider->SetMaxSprite("~res:/Gfx/LandscapeEditor/Tools/polzunok", 0);
    zoomSlider->SetMaxDrawType(UIControlBackground::DRAW_STRETCH_HORIZONTAL);
    zoomSlider->SetMaxLeftRightStretchCap(5);
    zoomSlider->SetThumbSprite("~res:/Gfx/LandscapeEditor/Tools/polzunokCenter", 0);
    AddControl(zoomSlider);
    
    formatDialog = new TextureFormatDialog(this);
    
    lastActiveZoomControl = NULL;
}

void TextureConverterDialog::AddLine(const DAVA::Rect &lineRect)
{
    UIControl *line =  ControlsFactory::CreateLine(lineRect, Color(0.2f, 0.2f, 0.2f, 0.8f));
    AddControl(line);
    SafeRelease(line);
}

TextureConverterDialog::~TextureConverterDialog()
{
    SafeRelease(zoomSlider);
    SafeRelease(formatDialog);
    
    SafeRelease(dstZoomPreview);
    SafeRelease(srcZoomPreview);
    SafeRelease(dstPreview);
    SafeRelease(srcPreview);
  
    ReleaseTextures();
    SafeRelease(workingScene);
    
    SafeRelease(convertButton);
    
    SafeRelease(closeButtonTop);
    SafeRelease(textureList);
}

int32 TextureConverterDialog::ElementsCount(UIList * list)
{
    return textures.size();
}

UIListCell *TextureConverterDialog::CellAtIndex(UIList *list, int32 index)
{
    TextureConverterCell *c = (TextureConverterCell *)list->GetReusableCell("TextureConverter cell"); 
    if(!c)
    {
        c = new TextureConverterCell(Rect(0, 0, list->GetSize().dx, 10), "TextureConverter cell");
    }
    
    c->SetSelected(false, false);
    c->SetTexture(GetWorkingTexturePath(GetTextureForIndex(index)->relativePathname));
    return c;
}

int32 TextureConverterDialog::CellHeight(UIList * list, int32 index)
{
    return TextureConverterCell::GetCellHeight();
}

void TextureConverterDialog::Show(Scene * scene)
{
    if(!GetParent())
    {
        SafeRelease(workingScene);
        workingScene = SafeRetain(scene);
        
        EnumerateTextures();
        
        selectedItem = -1;
        textureList->Refresh();
        
        UIScreen *screen = UIScreenManager::Instance()->GetScreen();
        screen->AddControl(this);
    }
}


void TextureConverterDialog::OnCancel(BaseObject * owner, void * userData, void * callerData)
{
    if(srcZoomPreview->GetParent())
    {
        RemoveControl(srcZoomPreview);
    }
    if(dstZoomPreview->GetParent())
    {
        RemoveControl(dstZoomPreview);
    }

    SafeRelease(workingScene);
    ReleaseTextures();

    if(GetParent())
    {
        GetParent()->RemoveControl(this);
    }
}

void TextureConverterDialog::EnumerateTextures()
{
    if(workingScene)
    {
        EnumerateTexturesFromMaterials();
        EnumerateTexturesFromNodes(workingScene);
    }
}

void TextureConverterDialog::EnumerateTexturesFromMaterials()
{
    Vector<Material *> materials;
    workingScene->GetDataNodes(materials);
    
    for(int32 iMat = 0; iMat < materials.size(); ++iMat)
    {
        for(int32 iTex = 0; iTex < Material::TEXTURE_COUNT; ++iTex)
        {
            Texture *t = materials[iMat]->textures[iTex];
            if(t)
            {
                textures.insert(SafeRetain(t));
            }
        }
    }
}

void TextureConverterDialog::EnumerateTexturesFromNodes(SceneNode * node)
{
    int32 count = node->GetChildrenCount();
    for(int32 iChild = 0; iChild < count; ++iChild)
    {
        SceneNode *child = node->GetChild(iChild);
        
        LandscapeNode *landscape = dynamic_cast<LandscapeNode*>(child);
        if (landscape) 
        {
            for(int32 iTex = 0; iTex < LandscapeNode::TEXTURE_COUNT; ++iTex)
            {
                Texture *t = landscape->GetTexture((LandscapeNode::eTextureLevel)iTex);
                if(t)
                {
                    textures.insert(SafeRetain(t));
                }
            }
        }
        
        EnumerateTexturesFromNodes(child);
    }
}

void TextureConverterDialog::RestoreTextures(Texture *t, const String &newTexturePath)
{
    RestoreTexturesFromMaterials(t, newTexturePath);
    RestoreTexturesFromNodes(t, newTexturePath, workingScene);
}

void TextureConverterDialog::RestoreTexturesFromMaterials(Texture *t, const String &newTexturePath)
{
    Vector<Material *> materials;
    workingScene->GetDataNodes(materials);
    
    for(int32 iMat = 0; iMat < materials.size(); ++iMat)
    {
        for(int32 iTex = 0; iTex < Material::TEXTURE_COUNT; ++iTex)
        {
            Texture *tex = materials[iMat]->textures[iTex];
            if(t == tex)
            {
                materials[iMat]->SetTexture((Material::eTextureLevel)iTex, newTexturePath);
            }
        }
    }

}

void TextureConverterDialog::RestoreTexturesFromNodes(Texture *t, const String &newTexturePath, SceneNode * node)
{
    int32 count = node->GetChildrenCount();
    for(int32 iChild = 0; iChild < count; ++iChild)
    {
        SceneNode *child = node->GetChild(iChild);
        
        LandscapeNode *landscape = dynamic_cast<LandscapeNode*>(child);
        if (landscape) 
        {
            for(int32 iTex = 0; iTex < LandscapeNode::TEXTURE_COUNT; ++iTex)
            {
                Texture *tex = landscape->GetTexture((LandscapeNode::eTextureLevel)iTex);
                if(t == tex)
                {
                    landscape->SetTexture((LandscapeNode::eTextureLevel)iTex, newTexturePath);
                }
            }
        }
        
        RestoreTexturesFromNodes(t, newTexturePath, child);
    }
}


void TextureConverterDialog::OnCellSelected(UIList *forList, UIListCell *selectedCell)
{
    selectedItem = selectedCell->GetIndex();
    SetupTexturePreview();
    
    //set selections
    List<UIControl*> children = forList->GetVisibleCells();
    List<UIControl*>::iterator endIt = children.end();
    for(List<UIControl*>::iterator it = children.begin(); it != endIt; ++it)
    {
        UIControl *ctrl = (*it);
        ctrl->SetSelected(false, false);
    }
    
    selectedCell->SetSelected(true, false);
}

void TextureConverterDialog::OnConvert(DAVA::BaseObject *owner, void *userData, void *callerData)
{
    if(-1 == selectedItem)
    {
        ErrorNotifier::Instance()->ShowError("Texture not selected.");
    }
    else 
    {
        formatDialog->Show();
    }
}

void TextureConverterDialog::OnFormatSelected(int32 newFormat, bool generateMimpaps)
{
    Texture *t = GetTextureForIndex(selectedItem);
     
    String newName = PVRConverter::Instance()->ConvertPngToPvr(t->relativePathname, newFormat, generateMimpaps);
    RestoreTextures(t, newName);
    
    SetupTexturePreview();
}


Texture *TextureConverterDialog::GetTextureForIndex(int32 index)
{
    Set<Texture *>::iterator it = textures.begin();
    Set<Texture *>::iterator endIt = textures.end();
    
    for(int32 i = 0; it != endIt; ++it, ++i)
    {
        if(index == i)
        {
            return (*it);
        }
    }
    
    return NULL;
}

void TextureConverterDialog::ReleaseTextures()
{
    Set<Texture *>::iterator it = textures.begin();
    Set<Texture *>::iterator endIt = textures.end();
    
    for(int32 i = 0; it != endIt; ++it, ++i)
    {
        Texture *t = (*it);
        SafeRelease(t);
    }
    textures.clear();
}

void TextureConverterDialog::SetupTexturePreview()
{
    srcOffsetPrev = Vector2(0, 0);
    dstOffsetPrev = Vector2(0, 0);

    Texture *srcTexture = NULL;
    Texture *dstTexture = NULL;
    
    Texture *workingTexture = GetTextureForIndex(selectedItem);
    String workingTexturePath = GetWorkingTexturePath(workingTexture->relativePathname);

    bool isEnabled = Image::IsAlphaPremultiplicationEnabled();
//    bool isMipmaps = Texture::IsMipmapGenerationEnabled();

//    if(workingTexture->isAlphaPremultiplied)
    {
        Image::EnableAlphaPremultiplication(false);
    }
//    if(workingTexture->isMimMapTexture)
    {
        Texture::DisableMipmapGeneration();   
    }

    if(FileSystem::GetExtension(workingTexturePath) == ".png")
    {
//        srcTexture = Texture::CreateFromFile(workingTexturePath);
//
//        String dstPath = FileSystem::ReplaceExtension(workingTexturePath, ".pvr");
//        dstTexture = Texture::CreateFromFile(dstPath);
        srcTexture = CreateFromImage(workingTexturePath);
        
        String dstPath = FileSystem::ReplaceExtension(workingTexturePath, ".pvr.png");
        dstTexture = CreateFromImage(dstPath);
    }
    else 
    {
//        String srcPath = FileSystem::ReplaceExtension(workingTexturePath, ".png");
//        srcTexture = Texture::CreateFromFile(srcPath);
//
//        dstTexture = Texture::CreateFromFile(workingTexturePath);

        
        String srcPath = FileSystem::ReplaceExtension(workingTexturePath, ".png");
        srcTexture = CreateFromImage(srcPath);
        
        String dstPath = FileSystem::ReplaceExtension(workingTexturePath, ".pvr.png");
        dstTexture = CreateFromImage(dstPath);
    }
    
//    if(!isMipmaps && workingTexture->isMimMapTexture)
//    if(isMipmaps)
    {
        Texture::EnableMipmapGeneration();
    }
//    if(workingTexture->isAlphaPremultiplied)
    {
        Image::EnableAlphaPremultiplication(isEnabled);
    }
    
    
    SetupZoomedPreview(srcTexture, srcPreview, srcZoomPreview);
    SetupZoomedPreview(dstTexture, dstPreview, dstZoomPreview);
    
    SafeRelease(srcTexture);
    SafeRelease(dstTexture);
}

Texture * TextureConverterDialog::CreateFromImage(const String &fileName)
{
	Image * image = Image::CreateFromFile(fileName);
	if (!image)
	{
		return 0;
	}
	
	RenderManager::Instance()->LockNonMain();
	Texture * texture = Texture::CreateFromData((Texture::PixelFormat)image->GetPixelFormat(), image->GetData(), image->GetWidth(), image->GetHeight());
	RenderManager::Instance()->UnlockNonMain();
	texture->relativePathname = fileName;
//    texture->isAlphaPremultiplied = image->isAlphaPremultiplied;
	
	SafeRelease(image);
	
	return texture;
}

void TextureConverterDialog::SetupZoomedPreview(Texture *tex, UIControl *preview, UIZoomControl *zoomControl)
{
    if(tex)
    {
        Sprite *sprite = Sprite::CreateFromTexture(tex, 0, 0, tex->width, tex->height);
        preview->SetSprite(sprite, 0);
        Vector2 texSize(tex->width, tex->height);
        preview->SetSize(texSize);
        zoomControl->SetContentSize(texSize);
        zoomControl->SetOffset(Vector2(0, 0));
        float32 minScale = zoomControl->GetSize().x / tex->width;
        zoomControl->SetScales(minScale, 10.f);
        zoomControl->SetScale(5.f);
        
        zoomSlider->SetValue(5.f);
        zoomSlider->SetMinMaxValue(minScale, 10.f);

        if(!zoomControl->GetParent())
        {
            AddControl(zoomControl);
        }
    }
    else 
    {
        if(zoomControl->GetParent())
        {
            RemoveControl(zoomControl);
        }
    }
}


void TextureConverterDialog::OnZoomChanged(DAVA::BaseObject *object, void *userData, void *callerData)
{
    float32 scale = zoomSlider->GetValue();
    if(dstZoomPreview->GetParent())
    {
        dstZoomPreview->SetScale(scale);
    }
    if(srcZoomPreview->GetParent())
    {
        srcZoomPreview->SetScale(scale);
    }
}

void TextureConverterDialog::Update(float32 timeElapsed)
{
    if(srcZoomPreview->GetParent() && dstZoomPreview->GetParent())
    {
        Vector2 srcOffset = srcZoomPreview->GetOffset();
        Vector2 dstOffset = dstZoomPreview->GetOffset();

        if(srcZoomPreview->IsScrolling())
        {
            lastActiveZoomControl = srcZoomPreview;
            
            srcOffsetPrev = srcOffset;
            dstOffsetPrev = srcOffset;
            dstZoomPreview->SetOffset(srcOffset);
        }
        else if(dstZoomPreview->IsScrolling())
        {
            lastActiveZoomControl = dstZoomPreview;
            
            srcOffsetPrev = dstOffset;
            dstOffsetPrev = dstOffset;
            srcZoomPreview->SetOffset(dstOffset);
        }
        else if(lastActiveZoomControl)
        {
            Vector2 offset = lastActiveZoomControl->GetOffset();
            srcZoomPreview->SetOffset(offset);
            dstZoomPreview->SetOffset(offset);
            lastActiveZoomControl = NULL;
        }
    }
    
    UIControl::Update(timeElapsed);
}


String TextureConverterDialog::GetWorkingTexturePath(const String &relativeTexturePath)
{
    String textureWorkingPath = relativeTexturePath;
    String::size_type pos = textureWorkingPath.find(".pvr.png");
    if(String::npos != pos)
    {
        textureWorkingPath = FileSystem::ReplaceExtension(textureWorkingPath, "");
    }

    return textureWorkingPath;
}
