#include "LandscapeEditor.h"

#include "PaintTool.h"
#include "HeightmapNode.h"
#include "EditorSettings.h"
#include "EditorScene.h"
#include "ErrorNotifier.h"

#include "EditorBodyControl.h"

#pragma mark --LandscapeEditor
LandscapeEditor::LandscapeEditor(LandscapeEditorDelegate *newDelegate, EditorBodyControl *parentControl)
    :   delegate(newDelegate)
    ,   state(ELE_NONE)
    ,   workingScene(NULL)
    ,   parent(parentControl)
{
	wasTileMaskToolUpdate = false;
    tileMaskEditorShader = new Shader();
	tileMaskEditorShader->LoadFromYaml("~res:/Shaders/Landscape/tilemask-editor.shader");
	tileMaskEditorShader->Recompile();

    fileSystemDialogOpMode = DIALOG_OPERATION_NONE;
    fileSystemDialog = new UIFileSystemDialog("~res:/Fonts/MyriadPro-Regular.otf");
    fileSystemDialog->SetDelegate(this);

    KeyedArchive *keyedArchieve = EditorSettings::Instance()->GetSettings();
    String path = keyedArchieve->GetString("3dDataSourcePath", "/");
    if(path.length())
    {
        fileSystemDialog->SetCurrentDir(path);   
    }

    workingLandscape = NULL;
    savedTexture = NULL;
    maskSprite = NULL;
	oldMaskSprite = NULL;
	toolSprite = NULL;
    currentTool = NULL;
    heightmapNode = NULL;
    settings = NULL;
    
    //init draw params
    srcBlendMode = BLEND_SRC_ALPHA;
    dstBlendMode = BLEND_ONE;
    paintColor = Color(1.f, 1.f, 1.f, 1.0f);
}

LandscapeEditor::~LandscapeEditor()
{
    SafeRelease(heightmapNode);
    SafeRetain(workingLandscape);
    SafeRelease(workingScene);
    
    SafeRelease(tileMaskEditorShader);

    SafeRelease(fileSystemDialog);
    
    SafeRelease(maskSprite);
	SafeRelease(oldMaskSprite);
	SafeRelease(toolSprite);
    SafeRelease(savedTexture);
}


void LandscapeEditor::Draw(const DAVA::UIGeometricData &geometricData)
{
    if(wasTileMaskToolUpdate)
	{
		UpdateTileMask();
        
		RenderManager::Instance()->SetRenderTarget(toolSprite);
		RenderManager::Instance()->ClearWithColor(0.f, 0.f, 0.f, 0.f);
		RenderManager::Instance()->RestoreRenderTarget();
		
		wasTileMaskToolUpdate = false;
	}
}


bool LandscapeEditor::SetScene(EditorScene *newScene)
{
    SafeRelease(workingScene);
    
    workingLandscape = SafeRetain(newScene->GetLandScape(newScene));
    if(!workingLandscape)
    {
        ErrorNotifier::Instance()->ShowError("No landscape at level.");
        return false;
    }
    else if(LandscapeNode::RENDERING_MODE_TILE_MASK_SHADER != workingLandscape->GetRenderingMode()) 
    {
        ErrorNotifier::Instance()->ShowError("Rendering mode is not RENDERING_MODE_TILE_MASK_SHADER.");
        return false;
    }
    
    workingScene = SafeRetain(newScene);
    return true;
}

void LandscapeEditor::SetPaintTool(PaintTool *newTool)
{
    currentTool = newTool;
}

void LandscapeEditor::SetSettings(LandscapeEditorSettings *newSettings)
{
    settings = newSettings;
}

LandscapeNode *LandscapeEditor::GetLandscape()
{
    return workingLandscape;
}

bool LandscapeEditor::IsActive()
{
    return (ELE_NONE != state);
}

void LandscapeEditor::Toggle()
{
    if(ELE_ACTIVE == state)
    {
        state = ELE_CLOSING;
        
        SaveNewMask();
    }
    else if(ELE_NONE == state)
    {
        SafeRelease(heightmapNode);
        heightmapNode = new HeightmapNode(workingScene);
        workingScene->AddNode(heightmapNode);
        
        isPaintActive = false;
        
        state = ELE_ACTIVE;
        
        if(delegate)
        {
            delegate->LandscapeEditorStarted();
        }

        CreateMaskTexture();
    }
}

void LandscapeEditor::Close()
{
    workingLandscape->SetTexture(LandscapeNode::TEXTURE_TILE_MASK, savedTexture);
    SafeRelease(workingLandscape);

    workingScene->RemoveNode(heightmapNode);
    SafeRelease(heightmapNode);

    SafeRelease(workingScene);
    
    SafeRelease(savedTexture);
    SafeRelease(maskSprite);
	SafeRelease(oldMaskSprite);
	SafeRelease(toolSprite);
    
    currentTool = NULL;
    state = ELE_NONE;
    
    if(delegate)
    {
        delegate->LandscapeEditorFinished();
    }
}


void LandscapeEditor::CreateMaskTexture()
{
    SafeRelease(savedTexture);
    savedTexture = SafeRetain(workingLandscape->GetTexture(LandscapeNode::TEXTURE_TILE_MASK));
    
    SafeRelease(maskSprite);
	SafeRelease(oldMaskSprite);
    
    int32 texSize = settings->maskSize;
    if(savedTexture)
    {
        texSize = savedTexture->width;
    }
    
    maskSprite = Sprite::CreateAsRenderTarget(texSize, texSize, Texture::FORMAT_RGBA8888);
	oldMaskSprite = Sprite::CreateAsRenderTarget(texSize, texSize, Texture::FORMAT_RGBA8888);
	toolSprite = Sprite::CreateAsRenderTarget(texSize, texSize, Texture::FORMAT_RGBA8888);
    
    if(savedTexture)
    {
        RenderManager::Instance()->LockNonMain();
        
        Sprite *oldMask = Sprite::CreateFromTexture(savedTexture, 0, 0, savedTexture->width, savedTexture->height);

        RenderManager::Instance()->SetRenderTarget(oldMaskSprite);
        oldMask->SetPosition(0.f, 0.f);
        oldMask->Draw();
        RenderManager::Instance()->RestoreRenderTarget();
        
        RenderManager::Instance()->SetRenderTarget(maskSprite);
        oldMask->SetPosition(0.f, 0.f);
        oldMask->Draw();
        RenderManager::Instance()->RestoreRenderTarget();

        SafeRelease(oldMask);

        RenderManager::Instance()->UnlockNonMain();
    }
    
	workingLandscape->SetTexture(LandscapeNode::TEXTURE_TILE_MASK, oldMaskSprite->GetTexture());
}

void LandscapeEditor::SaveNewMask()
{
    state = ELE_SAVING_MASK;
    
    if(savedTexture)
    {
        String pathToFile = savedTexture->relativePathname;
        SaveMaskAs(pathToFile, true);
    }
    else if(!fileSystemDialog->GetParent())
    {
        fileSystemDialog->SetExtensionFilter(".png");
        fileSystemDialog->SetOperationType(UIFileSystemDialog::OPERATION_SAVE);
        
        fileSystemDialog->SetCurrentDir(EditorSettings::Instance()->GetDataSourcePath());
        
        fileSystemDialog->Show(UIScreenManager::Instance()->GetScreen());
        fileSystemDialogOpMode = DIALOG_OPERATION_SAVE;
    }
}

void LandscapeEditor::SaveMaskAs(const String &pathToFile, bool closeLE)
{
    if(maskSprite)
    {
        Image *img = maskSprite->GetTexture()->CreateImageFromMemory();   
        if(img)
        {
            img->Save(pathToFile);
            SafeRelease(img);
            
            SafeRelease(savedTexture);
            workingLandscape->SetTexture(LandscapeNode::TEXTURE_TILE_MASK, pathToFile); 
            savedTexture = SafeRetain(workingLandscape->GetTexture(LandscapeNode::TEXTURE_TILE_MASK));
            workingLandscape->SetTexture(LandscapeNode::TEXTURE_TILE_MASK, maskSprite->GetTexture());
        }
    }
    
    if(closeLE)
    {
        state = ELE_MASK_SAVED;
        Close();
    }
}

bool LandscapeEditor::GetLandscapePoint(const Vector2 &touchPoint, Vector2 &landscapePoint)
{
    Vector3 from, dir;
    parent->GetCursorVectors(&from, &dir, touchPoint);
    Vector3 to = from + dir * 200.f;
    
    Vector3 point;
    bool isIntersect = workingScene->LandscapeIntersection(from, to, point);
    
    if(isIntersect)
    {
        AABBox3 box = workingLandscape->GetBoundingBox();
        
        landscapePoint.x = (point.x - box.min.x)* maskSprite->GetWidth() / (box.max.x - box.min.x);
        landscapePoint.y = (point.y - box.min.y) * maskSprite->GetWidth() / (box.max.y - box.min.y);
    }
    
    return isIntersect;
}



bool LandscapeEditor::Input(DAVA::UIEvent *touch)
{
    if(UIEvent::BUTTON_1 == touch->tid)
    {
        if(UIEvent::PHASE_BEGAN == touch->phase)
        {
            Vector2 point;
            isPaintActive = GetLandscapePoint(touch->point, point);
            if(isPaintActive)
            {
                prevDrawPos = Vector2(-100, -100);
                
                startPoint = endPoint = point;
                UpdateTileMaskTool();
            }
            return true;
        }
        else if(UIEvent::PHASE_DRAG == touch->phase)
        {
            Vector2 point;
            bool isIntersect = GetLandscapePoint(touch->point, point);
            if(isIntersect)
            {
                if(!isPaintActive)
                {
                    isPaintActive = true;
                    startPoint = point;
                }
                
                endPoint = point;
                UpdateTileMaskTool();
            }
            else 
            {
                isPaintActive = false;
                endPoint = point;
                
                UpdateTileMaskTool();
                prevDrawPos = Vector2(-100, -100);
            }
            return true;
        }
        else if(UIEvent::PHASE_ENDED == touch->phase)
        {
            Vector2 point;
            GetLandscapePoint(touch->point, point);
            if(isPaintActive)
            {
                isPaintActive = false;
                
                endPoint = point;
                UpdateTileMaskTool();
                prevDrawPos = Vector2(-100, -100);
            }
            return true;
        }
    }

    return false;
}

void LandscapeEditor::UpdateTileMask()
{
	int32 colorType;
	if(settings->redMask)
	{
		colorType = 0;
	}
	else if(settings->greenMask)
	{
		colorType = 1;
	}
	else if(settings->blueMask)
	{
		colorType = 2;
	}
	else if(settings->alphaMask)
	{
		colorType = 3;
	}
	else
	{
		return; // no color selected
	}
    
	RenderManager::Instance()->SetRenderTarget(maskSprite);
    
	srcBlendMode = RenderManager::Instance()->GetSrcBlend();
	dstBlendMode = RenderManager::Instance()->GetDestBlend();
	RenderManager::Instance()->SetBlendMode(BLEND_ONE, BLEND_ZERO);
    
	RenderManager::Instance()->SetShader(tileMaskEditorShader);
	oldMaskSprite->PrepareSpriteRenderData(0);
	RenderManager::Instance()->SetRenderData(oldMaskSprite->spriteRenderObject);
	RenderManager::Instance()->SetTexture(oldMaskSprite->GetTexture(), 0);
	RenderManager::Instance()->SetTexture(toolSprite->GetTexture(), 1);
	RenderManager::Instance()->FlushState();
    
	int32 tex0 = tileMaskEditorShader->FindUniformLocationByName("texture0");
	tileMaskEditorShader->SetUniformValue(tex0, 0);
	int32 tex1 = tileMaskEditorShader->FindUniformLocationByName("texture1");
	tileMaskEditorShader->SetUniformValue(tex1, 1);
	int32 colorTypeUniform = tileMaskEditorShader->FindUniformLocationByName("colorType");
	tileMaskEditorShader->SetUniformValue(colorTypeUniform, colorType);
	int32 intensityUniform = tileMaskEditorShader->FindUniformLocationByName("intensity");
    
    float32 intension = currentTool->intension * currentTool->intension;
//	tileMaskEditorShader->SetUniformValue(intensityUniform, currentTool->intension);
	tileMaskEditorShader->SetUniformValue(intensityUniform, intension);
    
	RenderManager::Instance()->HWDrawArrays(PRIMITIVETYPE_TRIANGLESTRIP, 0, 4);
    
	RenderManager::Instance()->SetBlendMode(srcBlendMode, dstBlendMode);
	RenderManager::Instance()->RestoreRenderTarget();
    
	workingLandscape->SetTexture(LandscapeNode::TEXTURE_TILE_MASK, maskSprite->GetTexture());
	Sprite * temp = oldMaskSprite;
	oldMaskSprite = maskSprite;
	maskSprite = temp;
}

void LandscapeEditor::UpdateTileMaskTool()
{
	if(currentTool && currentTool->sprite && currentTool->zoom)
	{
		float32 scaleSize = currentTool->sprite->GetWidth() * ( currentTool->zoom );
		Vector2 deltaPos = endPoint - startPoint;
		{
			Vector2 pos = startPoint - Vector2(scaleSize, scaleSize)/2;
			if(pos != prevDrawPos)
			{
				wasTileMaskToolUpdate = true;
                
				RenderManager::Instance()->SetRenderTarget(toolSprite);
				currentTool->sprite->SetScaleSize(scaleSize, scaleSize);
				currentTool->sprite->SetPosition(pos);
				currentTool->sprite->Draw();
				RenderManager::Instance()->RestoreRenderTarget();
			}
			startPoint = endPoint;
		}
	}
}

#pragma mark -- LandscapeToolsPanelDelegate
void LandscapeEditor::OnToolSelected(PaintTool *newTool)
{
    currentTool = newTool;
}

#pragma mark -- LandscapeEditorPropertyControlDelegate
void LandscapeEditor::LandscapeEditorSettingsChanged(LandscapeEditorSettings *settings)
{
    settings = settings;
}

void LandscapeEditor::MaskTextureWillChanged()
{
    if(savedTexture)
    {
        String pathToFile = savedTexture->relativePathname;
        SaveMaskAs(pathToFile, false);
    }
}

void LandscapeEditor::MaskTextureDidChanged()
{
    CreateMaskTexture();
}

#pragma mark -- UIFileSystemDialogDelegate
void LandscapeEditor::OnFileSelected(UIFileSystemDialog *forDialog, const String &pathToFile)
{
    switch (fileSystemDialogOpMode) 
    {
        case DIALOG_OPERATION_SAVE:
        {
            SaveMaskAs(pathToFile, true);
            break;
        }
            
        default:
            break;
    }
    
    fileSystemDialogOpMode = DIALOG_OPERATION_NONE;
}

void LandscapeEditor::OnFileSytemDialogCanceled(UIFileSystemDialog *forDialog)
{
    fileSystemDialogOpMode = DIALOG_OPERATION_NONE;
    
    Close();
}

