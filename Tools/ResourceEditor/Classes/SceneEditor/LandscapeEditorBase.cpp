#include "LandscapeEditorBase.h"

#include "HeightmapNode.h"
#include "EditorSettings.h"
#include "../EditorScene.h"
#include "ErrorNotifier.h"
#include "EditorBodyControl.h"

#pragma mark --LandscapeEditorBase
LandscapeEditorBase::LandscapeEditorBase(LandscapeEditorDelegate *newDelegate, EditorBodyControl *parentControl)
    :   delegate(newDelegate)
    ,   state(ELE_NONE)
    ,   workingScene(NULL)
    ,   parent(parentControl)
    ,   inverseDrawingEnabled(false)
    ,   touchID(INVALID_TOUCH_ID)
{
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
    workingScene = NULL;

    savedPath = "";
    
    currentTool = NULL;
    heightmapNode = NULL;
    
    toolsPanel = NULL;
    
    landscapeSize = 0;

	cursorTexture = Texture::CreateFromFile("~res:/LandscapeEditor/Tools/cursor/cursor.png");
	cursorTexture->SetWrapMode(Texture::WRAP_CLAMP, Texture::WRAP_CLAMP);
}

LandscapeEditorBase::~LandscapeEditorBase()
{
    savedPath = "";
    
    SafeRelease(toolsPanel);
    
    SafeRelease(heightmapNode);
    SafeRetain(workingLandscape);
    SafeRelease(workingScene);
    
    SafeRelease(fileSystemDialog);

	SafeRelease(cursorTexture);
}


void LandscapeEditorBase::Draw(const DAVA::UIGeometricData &geometricData)
{
}

void LandscapeEditorBase::Update(float32 timeElapsed)
{
}

bool LandscapeEditorBase::SetScene(EditorScene *newScene)
{
    SafeRelease(workingScene);
    
    workingLandscape = SafeRetain(newScene->GetLandScape(newScene));
    if(!workingLandscape)
    {
        ErrorNotifier::Instance()->ShowError("No landscape at level.");
        return false;
    }
    
    workingScene = SafeRetain(newScene);
    return true;
}

void LandscapeEditorBase::SetTool(LandscapeTool *newTool)
{
    currentTool = newTool;
}

LandscapeNode *LandscapeEditorBase::GetLandscape()
{
    return workingLandscape;
}

bool LandscapeEditorBase::IsActive()
{
    return (ELE_NONE != state);
}

void LandscapeEditorBase::Toggle()
{
    if(ELE_ACTIVE == state)
    {
        state = ELE_CLOSING;
        
        SaveTexture();
    }
    else if(ELE_NONE == state)
    {
        touchID = INVALID_TOUCH_ID;
        
        SafeRelease(heightmapNode);
        heightmapNode = new HeightmapNode(workingScene, workingLandscape);
        workingScene->AddNode(heightmapNode);
                
        state = ELE_ACTIVE;
        
        SetTool(toolsPanel->CurrentTool());
        
        if(delegate)
        {
            delegate->LandscapeEditorStarted();
        }
        
        ShowAction();
    }
}

void LandscapeEditorBase::Close()
{
    HideAction();
    
    workingLandscape->SetDebugFlags(workingLandscape->GetDebugFlags() & ~SceneNode::DEBUG_DRAW_GRID);
    
    SafeRelease(workingLandscape);

    workingScene->RemoveNode(heightmapNode);
    SafeRelease(heightmapNode);

    SafeRelease(workingScene);
    
    state = ELE_NONE;
    
    if(delegate)
    {
        delegate->LandscapeEditorFinished();
    }
}

bool LandscapeEditorBase::GetLandscapePoint(const Vector2 &touchPoint, Vector2 &landscapePoint)
{
    DVASSERT(parent);

    Vector3 from, dir;
    parent->GetCursorVectors(&from, &dir, touchPoint);
    Vector3 to = from + dir * 200.f;
    
    Vector3 point;
    bool isIntersect = workingScene->LandscapeIntersection(from, to, point);
    
    if(isIntersect)
    {
        AABBox3 box = workingLandscape->GetBoundingBox();
            
        //TODO: use 
        landscapePoint.x = (point.x - box.min.x) * (landscapeSize - 1) / (box.max.x - box.min.x);
        landscapePoint.y = (point.y - box.min.y) * (landscapeSize - 1) / (box.max.y - box.min.y);
    }
    
    return isIntersect;
}


bool LandscapeEditorBase::Input(DAVA::UIEvent *touch)
{
	Vector2 point;
	bool isIntersect = GetLandscapePoint(touch->point, point);
    
    point.x = (int32)point.x;
    point.y = (int32)point.y;
    
	startPoint = endPoint = point;
	UpdateCursor();
	
    if(INVALID_TOUCH_ID == touchID || touchID == touch->tid)
    {
        if(UIEvent::BUTTON_1 == touch->tid)
        {
            inverseDrawingEnabled = InputSystem::Instance()->GetKeyboard()->IsKeyPressed(DVKEY_ALT);
            
            if(UIEvent::PHASE_BEGAN == touch->phase)
            {
                touchID = touch->tid;
                if(isIntersect)
                {
                    prevDrawPos = Vector2(-100, -100);
                    InputAction(touch->phase, isIntersect);
                }
                return true;
            }
            else if(UIEvent::PHASE_DRAG == touch->phase)
            {
                InputAction(touch->phase, isIntersect);
                if(!isIntersect)
                {
                    prevDrawPos = Vector2(-100, -100);
                }
                return true;
            }
            else if(UIEvent::PHASE_ENDED == touch->phase || UIEvent::PHASE_CANCELLED == touch->phase)
            {
                touchID = INVALID_TOUCH_ID;
                
                if(isIntersect)
                {
                    InputAction(touch->phase, isIntersect);
                    prevDrawPos = Vector2(-100, -100);
                }
                return true;
            }
        }
    }
    
    if(UIEvent::PHASE_KEYCHAR == touch->phase)
    {
        if(DVKEY_Z == touch->tid && InputSystem::Instance()->GetKeyboard()->IsKeyPressed(DVKEY_CTRL))
        {
            UndoAction();
            return true;
        }
        if(DVKEY_Z == touch->tid && InputSystem::Instance()->GetKeyboard()->IsKeyPressed(DVKEY_SHIFT))
        {
            RedoAction();
            return true;
        }
    }
    
    return false;
}

void LandscapeEditorBase::SaveTexture()
{
    state = ELE_SAVING_TEXTURE;
    
    if(savedPath.length())
    {
        SaveTextureAs(savedPath, true);
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

void LandscapeEditorBase::SaveTextureAs(const String &pathToFile, bool closeLE)
{
    SaveTextureAction(pathToFile);
    
    if(closeLE)
    {
        state = ELE_TEXTURE_SAVED;
        Close();
    }
}

LandscapeToolsPanel * LandscapeEditorBase::GetToolPanel()
{
    return toolsPanel;
}

#pragma mark -- LandscapeToolsPanelDelegate
void LandscapeEditorBase::OnToolSelected(LandscapeTool *newTool)
{
    SetTool(newTool);
}

void LandscapeEditorBase::OnShowGrid(bool show)
{
    if(workingLandscape)
    {
        if(show)
        {
            workingLandscape->SetDebugFlags(workingLandscape->GetDebugFlags() | SceneNode::DEBUG_DRAW_GRID);
        }
        else 
        {
            workingLandscape->SetDebugFlags(workingLandscape->GetDebugFlags() & ~SceneNode::DEBUG_DRAW_GRID);
        }
    }
    else 
    {
        DVASSERT(false);
    }
}


#pragma mark -- UIFileSystemDialogDelegate
void LandscapeEditorBase::OnFileSelected(UIFileSystemDialog *forDialog, const String &pathToFile)
{
    switch (fileSystemDialogOpMode) 
    {
        case DIALOG_OPERATION_SAVE:
        {
            savedPath = pathToFile;
            SaveTextureAs(pathToFile, true);
            break;
        }
            
        default:
            break;
    }
    
    fileSystemDialogOpMode = DIALOG_OPERATION_NONE;
}

void LandscapeEditorBase::OnFileSytemDialogCanceled(UIFileSystemDialog *forDialog)
{
    fileSystemDialogOpMode = DIALOG_OPERATION_NONE;
    
    Close();
}

