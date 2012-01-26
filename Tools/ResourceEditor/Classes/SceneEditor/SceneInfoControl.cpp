#include "SceneInfoControl.h"
#include "ControlsFactory.h"
#include "PropertyList.h"
#include "EditorSettings.h"
#include "SceneValidator.h"


SceneInfoControl::SceneInfoControl(const Rect &rect)
    :   UIControl(rect)
{
    workingScene = NULL;
    SetInputEnabled(false);
    
    sceneInfo = new PropertyList(Rect(0, 0, rect.dx, rect.dy), NULL);
    sceneInfo->SetInputEnabled(false);
    sceneInfo->GetBackground()->SetColor(Color(1.0f, 1.0f, 1.0f, 0.0f));
    AddControl(sceneInfo);
}


SceneInfoControl::~SceneInfoControl()
{
    SafeRelease(sceneInfo);
}

void SceneInfoControl::WillAppear()
{
    SceneValidator::Instance()->SetInfoControl(this);
    
    UpdateInfo(NULL, NULL, NULL);
}

void SceneInfoControl::UpdateInfo(BaseObject * owner, void * userData, void * callerData)
{
    if(workingScene)
    {
        InvalidateRenderStats();
    }
    
    Animation * anim = WaitAnimation(2.f); //every 2 seconds
    anim->AddEvent(Animation::EVENT_ANIMATION_END, Message(this, &SceneInfoControl::UpdateInfo));
}

void SceneInfoControl::SetIntInfoValue(const String &key, int32 newValue)
{
    if(!sceneInfo->IsPropertyAvaliable(key))
    {
        sceneInfo->AddIntProperty(key, PropertyList::PROPERTY_IS_READ_ONLY);
    }
    
    sceneInfo->SetIntPropertyValue(key, newValue);
}

void SceneInfoControl::SetFloatInfoValue(const String &key, float32 newValue)
{
    if(!sceneInfo->IsPropertyAvaliable(key))
    {
        sceneInfo->AddFloatProperty(key, PropertyList::PROPERTY_IS_READ_ONLY);
    }
    
    sceneInfo->SetFloatPropertyValue(key, newValue);
}


void SceneInfoControl::SetWorkingScene(Scene *scene)
{
    workingScene = scene;
    renderStats.Clear();
}

void SceneInfoControl::InvalidateTexturesInfo(int32 count, int32 size)
{
    SetIntInfoValue("Text.Count", count);
    SetIntInfoValue("Text.Memory", size);

    if(workingScene)
    {
        SetIntInfoValue("Mat.Count", workingScene->GetMaterialCount());
    }
    
    RedrawCells();
}

void SceneInfoControl::SetRenderStats(const RenderManager::Stats & newRenderStats)
{
    renderStats = newRenderStats;
}

void SceneInfoControl::InvalidateRenderStats()
{
    SetIntInfoValue("ArraysCalls", renderStats.drawArraysCalls);
    SetIntInfoValue("ElementsCalls", renderStats.drawElementsCalls);

    SetIntInfoValue("PointsList", renderStats.primitiveCount[PRIMITIVETYPE_POINTLIST]);
    SetIntInfoValue("LineList", renderStats.primitiveCount[PRIMITIVETYPE_LINELIST]);
    SetIntInfoValue("LineStrip", renderStats.primitiveCount[PRIMITIVETYPE_LINESTRIP]);
    SetIntInfoValue("TriangleList", renderStats.primitiveCount[PRIMITIVETYPE_TRIANGLELIST]);
    SetIntInfoValue("TriangleStrip", renderStats.primitiveCount[PRIMITIVETYPE_TRIANGLESTRIP]);
    SetIntInfoValue("TriangleFan", renderStats.primitiveCount[PRIMITIVETYPE_TRIANGLEFAN]);
    
    RedrawCells();
}


void SceneInfoControl::RedrawCells()
{
    //Set cells background
    List<UIControl*> cells = sceneInfo->GetVisibleCells();
    for(List<UIControl*>::iterator it = cells.begin(); it != cells.end(); ++it)
    {
        UIControl *cell = (*it);
        ControlsFactory::CusomizeTransparentControl(cell, 0.1f);
    }
}
