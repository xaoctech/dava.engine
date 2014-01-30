/*==================================================================================
    Copyright (c) 2008, binaryzebra
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright
    notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
    notice, this list of conditions and the following disclaimer in the
    documentation and/or other materials provided with the distribution.
    * Neither the name of the binaryzebra nor the
    names of its contributors may be used to endorse or promote products
    derived from this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE binaryzebra AND CONTRIBUTORS "AS IS" AND
    ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL binaryzebra BE LIABLE FOR ANY
    DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
    ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
    SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
=====================================================================================*/



#include "UI/UIParticles.h"
#include "Render/RenderManager.h"
#include "Render/RenderHelper.h"
#include "Core/Core.h"
#include "Scene3D/Components/ComponentHelpers.h"

namespace DAVA 
{


UIParticles::UIParticles(const Rect &rect, bool rectInAbsoluteCoordinates)
    :   UIControl(rect, rectInAbsoluteCoordinates)    
    , effect(NULL)
    , system(new ParticleEffectSystem(NULL))
    , camera(new Camera())
    , updateTime(0)
{
    matrix.Identity();
    float32 w = Core::Instance()->GetVirtualScreenXMax() - Core::Instance()->GetVirtualScreenXMin();
    float32 h = Core::Instance()->GetVirtualScreenYMax() - Core::Instance()->GetVirtualScreenYMin();
    float32 aspect = w / h;
    camera->SetupOrtho(w, aspect, 1, 1000);        
    camera->SetPosition(Vector3(w/2,-500, -h/2));
    camera->SetTarget(Vector3(w/2, 0, -h/2));  
    camera->SetUp(Vector3(0, 0, 1));
    camera->RebuildCameraFromValues();        
}

UIParticles::~UIParticles()
{    
    if (effect&&effect->state!=ParticleEffectComponent::STATE_STOPPED)
        system->RemoveFromActive(effect);
    delete system;
    delete effect;        
    SafeRelease(camera);
}


void UIParticles::Start()
{
    DVASSERT(effect);
    updateTime = 0;
    effect->isPaused = false;    
    system->RunEffect(effect);
    
}

void UIParticles::Stop(bool isDeleteAllParticles)
{
    DVASSERT(effect);
    updateTime = 0;
    if (effect->state == ParticleEffectComponent::STATE_STOPPED) return;
    if (isDeleteAllParticles)
    {
        effect->ClearCurrentGroups();		
        effect->effectData.infoSources.resize(1);        
        effect->isPaused = false;
        system->RemoveFromActive(effect);
    }
    else
    {
        effect->state = ParticleEffectComponent::STATE_STOPPING;
    }
}

void UIParticles::Pause(bool isPaused /*= true*/)
{	
    DVASSERT(effect);
    effect->isPaused = isPaused;
}

bool UIParticles::IsStopped()
{
    DVASSERT(effect);
    return effect->state == ParticleEffectComponent::STATE_STOPPED;
}

bool UIParticles::IsPaused()
{
    DVASSERT(effect);
    return effect->isPaused;
}

void UIParticles::Restart(bool isDeleteAllParticles)
{
    DVASSERT(effect);
    effect->isPaused = false;
    if (isDeleteAllParticles)
        effect->ClearCurrentGroups();
    effect->currRepeatsCont = 0;
    system->RunEffect(effect);
}


void UIParticles::AddControl(UIControl *control)
{
    DVASSERT(0 && "UIParticles do not support children");
}

    
void UIParticles::Update(float32 timeElapsed)
{
    updateTime += timeElapsed;        
}

void UIParticles::Draw(const UIGeometricData & geometricData)
{
    if ((!effect)||(effect->state == ParticleEffectComponent::STATE_STOPPED)) 
        return;


    
    matrix.CreateRotation(Vector3(0,1,0), geometricData.angle);
    //negative y is to make 2d/3d particles look similar (matrix setup)
    matrix.SetTranslationVector(Vector3(geometricData.position.x, 0, -geometricData.position.y));
    system->Process(updateTime);
    updateTime = 0;
    		
    RenderManager::Instance()->PushDrawMatrix();
    RenderManager::Instance()->PushMappingMatrix();
    
    /*draw particles here*/
    RenderManager::Instance()->SetDefault3DState();    
    RenderManager::Instance()->FlushState();    
    camera->SetupDynamicParameters();

    
    effect->effectRenderObject->PrepareToRender(camera);
    for (int32 i=0, sz = effect->effectRenderObject->GetRenderBatchCount(); i<sz; ++i)
        effect->effectRenderObject->GetRenderBatch(i)->Draw(PASS_FORWARD, camera);
                       
    RenderManager::Instance()->GetRenderer2D()->Setup2DMatrices();    
    RenderManager::Instance()->PopDrawMatrix();
    RenderManager::Instance()->PopMappingMatrix();
	RenderManager::Instance()->SetDefault2DState();
	        
}
    


void UIParticles::Load(const FilePath& path)
{
    SceneFileV2 *sceneFile = new SceneFileV2();
    sceneFile->EnableDebugLog(false);
    SceneArchive *archive = sceneFile->LoadSceneArchive(path);    
    SafeRelease(sceneFile);
    if (archive->children.size()>0)
    {
        Entity *e = new Entity();
        SerializationContext serializationContext;
        serializationContext.SetRootNodePath(path);
        serializationContext.SetScenePath(FilePath(path.GetDirectory()));
        serializationContext.SetVersion(10);
        serializationContext.SetScene(NULL);
        serializationContext.SetDefaultMaterialQuality(NMaterial::DEFAULT_QUALITY_NAME);
        e->Load(archive->children[0]->archive, &serializationContext);
        ParticleEffectComponent *effSrc = GetEffectComponent(e);
        if (effSrc)
            effect = (ParticleEffectComponent*)effSrc->Clone(NULL);
        SafeRelease(e);
    }

    SafeRelease(archive);

    if (effect)
        effect->effectRenderObject->SetEffectMatrix(&matrix);
    
}

}