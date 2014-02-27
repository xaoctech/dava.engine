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


#include "Render/Highlevel/RenderPass.h"
#include "Render/Highlevel/RenderFastNames.h"
#include "Render/Highlevel/RenderLayer.h"
#include "Render/Highlevel/RenderBatchArray.h"
#include "Render/Highlevel/Camera.h"
#include "Render/ImageLoader.h"

namespace DAVA
{
    
RenderPass::RenderPass(const FastName & _name, RenderPassID _id)    
    :   name(_name)
    ,   id(_id)
{
    renderPassBatchArray = new RenderPassBatchArray();
    renderLayers.reserve(RENDER_LAYER_ID_COUNT);
}

RenderPass::~RenderPass()
{
    SafeDelete(renderPassBatchArray);
}
    
void RenderPass::AddRenderLayer(RenderLayer * layer, const FastName & afterLayer)
{
	if(LAST_LAYER != afterLayer)
	{
		uint32 size = renderLayers.size();
		for(uint32 i = 0; i < size; ++i)
		{
			const FastName & name = renderLayers[i]->GetName();
			if(afterLayer == name)
			{
				renderLayers.insert(renderLayers.begin() +i+1, layer);
				return;
			}
		}
		DVASSERT(0 && "RenderPass::AddRenderLayer afterLayer not found");
	}
	else
	{
		renderLayers.push_back(layer);
	}
}
    
void RenderPass::RemoveRenderLayer(RenderLayer * layer)
{
	Vector<RenderLayer*>::iterator it = std::find(renderLayers.begin(), renderLayers.end(), layer);
	DVASSERT(it != renderLayers.end());

	renderLayers.erase(it);
}

void RenderPass::Draw(Camera * camera, RenderSystem * renderSystem)
{    
    PrepareVisibilityArrays(camera, renderSystem);
    DrawLayers(camera);
}

void RenderPass::PrepareVisibilityArrays(Camera *camera, RenderSystem * renderSystem)
{
    visibilityArray.Clear();
    renderSystem->GetRenderHierarchy()->Clip(camera, &visibilityArray);
    renderPassBatchArray->Clear();
    renderPassBatchArray->PrepareVisibilityArray(&visibilityArray, camera); 
}

void RenderPass::DrawLayers(Camera *camera)
{
    // Draw all layers with their materials
    uint32 size = (uint32)renderLayers.size();
    for (uint32 k = 0; k < size; ++k)
    {
        RenderLayer * layer = renderLayers[k];
        RenderLayerBatchArray * renderLayerBatchArray = renderPassBatchArray->Get(layer->GetRenderLayerID());
        if (renderLayerBatchArray)
        {
            layer->Draw(name, camera, renderLayerBatchArray); //why camera here?
        }
    }
}


MainForwardRenderPass::MainForwardRenderPass(const FastName & name, RenderPassID id):RenderPass(name, id),
    reflectionTexture(NULL), reflectionSprite(NULL), refractionTexture(NULL), refractionSprite(NULL),
    reflectionPass(NULL), refractionPass(NULL)
{
    const RenderLayerManager * renderLayerManager = RenderLayerManager::Instance();
    AddRenderLayer(renderLayerManager->GetRenderLayer(LAYER_OPAQUE), LAST_LAYER);
    AddRenderLayer(renderLayerManager->GetRenderLayer(LAYER_AFTER_OPAQUE), LAST_LAYER);
    AddRenderLayer(renderLayerManager->GetRenderLayer(LAYER_ALPHA_TEST_LAYER), LAST_LAYER);
    AddRenderLayer(renderLayerManager->GetRenderLayer(LAYER_WATER), LAST_LAYER);
    AddRenderLayer(renderLayerManager->GetRenderLayer(LAYER_TRANSLUCENT), LAST_LAYER);
    AddRenderLayer(renderLayerManager->GetRenderLayer(LAYER_AFTER_TRANSLUCENT), LAST_LAYER);
    AddRenderLayer(renderLayerManager->GetRenderLayer(LAYER_SHADOW_VOLUME), LAST_LAYER);
}

void MainForwardRenderPass::Draw(Camera * camera, RenderSystem * renderSystem)
{
	//PrepareVisibilityArrays(camera, renderSystem);

	/*one global water pass*/
	/*RenderLayerBatchArray *waterLayer = renderPassBatchArray->Get(RenderLayerManager::Instance()->GetLayerIDByName(LAYER_WATER));
	bool needWaterPrepass = false;
	uint32 waterBatchesCount = waterLayer->GetRenderBatchCount();
	AABBox3 waterBox;
	if (waterBatchesCount)
	{
		for (uint32 i=0; i<waterBatchesCount; ++i)
		{
			RenderBatch *batch = waterLayer->Get(i);
			if (batch->GetMaterial())//->GetSomePropertyToUnderstandThisWaterTypeRequiresRealReflectionRefractionRender()
			{
				needWaterPrepass = true;
				waterBox.AddAABBox(batch->GetRenderObject()->GetWorldBoundingBox());
			}
		}
	}*/
    bool needWaterPrepass = true;
    AABBox3 waterBox (Vector3(0,0,17), 0);
	if (needWaterPrepass)
	{
        const static int32 TEX_SIZE = 1024;
        if (!reflectionPass)
        {             
            reflectionPass = new WaterReflectionRenderPass(PASS_FORWARD, RENDER_PASS_WATER_REFLECTION);
            reflectionTexture = Texture::CreateFBO(TEX_SIZE, TEX_SIZE, FORMAT_RGBA8888, Texture::DEPTH_RENDERBUFFER);          
            reflectionSprite = Sprite::CreateFromTexture(reflectionTexture, 0, 0, TEX_SIZE, TEX_SIZE);
            
            refractionPass = new WaterRefractionRenderPass(PASS_FORWARD, RENDER_PASS_WATER_REFRACTION);
            refractionTexture = Texture::CreateFBO(TEX_SIZE, TEX_SIZE, FORMAT_RGBA8888, Texture::DEPTH_RENDERBUFFER);          
            refractionSprite = Sprite::CreateFromTexture(refractionTexture, 0, 0, TEX_SIZE, TEX_SIZE);
        }
        Rect viewportSave = RenderManager::Instance()->GetViewport();
        RenderManager::Instance()->SetRenderTarget(reflectionSprite);
        RenderManager::Instance()->SetViewport(Rect(0, 0, TEX_SIZE, TEX_SIZE), true);        
        RenderManager::Instance()->Clear(Color(0.0f, 1.0f, 0.0f, 1.0f), 1.0f, 0);     
        RenderManager::Instance()->SetRenderState(RenderState::RENDERSTATE_3D_BLEND);
        RenderManager::Instance()->FlushState();
        reflectionPass->SetWaterLevel(waterBox.min.z);
        reflectionPass->Draw(camera, renderSystem);
        RenderManager::Instance()->RestoreRenderTarget();
                
        RenderManager::Instance()->SetRenderTarget(refractionSprite);
        RenderManager::Instance()->SetViewport(Rect(0, 0, TEX_SIZE, TEX_SIZE), true);        
        RenderManager::Instance()->Clear(Color(1.0f, 0.0f, 0.0f, 1.0f), 1.0f, 0);
        RenderManager::Instance()->SetRenderState(RenderState::RENDERSTATE_3D_BLEND);
        RenderManager::Instance()->FlushState();
        refractionPass->SetWaterLevel(waterBox.max.z);
        refractionPass->Draw(camera, renderSystem);
        RenderManager::Instance()->RestoreRenderTarget();
        RenderManager::Instance()->SetViewport(viewportSave, true);                

        camera->SetupDynamicParameters();
		
		/*for (uint32 i=0; i<waterBatchesCount; ++i)
		{
			NMaterial *mat = waterLayer->Get(i)->GetMaterial();
			mat->SetTexture(FastName("reflTex"), reflectionTexture);
			mat->SetTexture(FastName("refrTex"), refractionTexture);
		}*/
	}	

    PrepareVisibilityArrays(camera, renderSystem);
    RenderLayerBatchArray *waterLayer = renderPassBatchArray->Get(RenderLayerManager::Instance()->GetLayerIDByName(LAYER_WATER));
    uint32 waterBatchesCount = waterLayer->GetRenderBatchCount();    
    static FastName rss = FastName("rcpScreenSize");
    Rect viewport = RenderManager::Instance()->GetViewport();
    Vector2 rssVal(1.0f/viewport.dx, 1.0f/viewport.dy);
    if (waterBatchesCount)
    {
        for (uint32 i=0; i<waterBatchesCount; ++i)
        {
            NMaterial *mat = waterLayer->Get(i)->GetMaterial();
            mat->SetPropertyValue(rss, Shader::UT_FLOAT_VEC2, 1, &rssVal);
            mat->SetTexture(FastName("reflTex"), reflectionTexture);
            mat->SetTexture(FastName("refrTex"), refractionTexture);
        }
    }
	DrawLayers(camera);

    /*if (needWaterPrepass)
    {
        static int t=0;        
        if ((t%700) == 0)
        {
            Image * img = reflectionTexture->CreateImageFromMemory(RenderState::RENDERSTATE_2D_BLEND);
            ImageLoader::Save(img, "reflectionTexture.png");
            img = refractionTexture->CreateImageFromMemory(RenderState::RENDERSTATE_2D_BLEND);
            ImageLoader::Save(img, "refractiontest.png");
        }
        t++;
    }*/
}

WaterPrePass::WaterPrePass(const FastName & name, RenderPassID id):RenderPass(name, id), passCamera(NULL)
{
    const RenderLayerManager * renderLayerManager = RenderLayerManager::Instance();
    AddRenderLayer(renderLayerManager->GetRenderLayer(LAYER_OPAQUE), LAST_LAYER);
    AddRenderLayer(renderLayerManager->GetRenderLayer(LAYER_AFTER_OPAQUE), LAST_LAYER);
    AddRenderLayer(renderLayerManager->GetRenderLayer(LAYER_ALPHA_TEST_LAYER), LAST_LAYER);
    AddRenderLayer(renderLayerManager->GetRenderLayer(LAYER_TRANSLUCENT), LAST_LAYER);
    AddRenderLayer(renderLayerManager->GetRenderLayer(LAYER_AFTER_TRANSLUCENT), LAST_LAYER);
    //AddRenderLayer(renderLayerManager->GetRenderLayer(LAYER_SHADOW_VOLUME), LAST_LAYER);
}
WaterPrePass::~WaterPrePass()
{
    SafeRelease(passCamera);
}

WaterReflectionRenderPass::WaterReflectionRenderPass(const FastName & name, RenderPassID id):WaterPrePass(name, id)
{
    
}



void WaterReflectionRenderPass::Draw(Camera * camera, RenderSystem * renderSystem)
{    
    if (!passCamera)
        passCamera = new Camera();    
    passCamera->CopyMathOnly(*camera);    
    Vector3 v;
    v = passCamera->GetPosition();
    v.z = waterLevel - (v.z - waterLevel);
    passCamera->SetPosition(v);
    v = passCamera->GetTarget();
    v.z = waterLevel - (v.z - waterLevel);
    passCamera->SetTarget(v);    
    //v = passCamera->GetUp();
    //v*=-1;
    //passCamera->SetUp(v);
    passCamera->SetupDynamicParameters();        
    //add clipping plane
    PrepareVisibilityArrays(passCamera, renderSystem);    
    DrawLayers(passCamera);
}


WaterRefractionRenderPass::WaterRefractionRenderPass(const FastName & name, RenderPassID id) : WaterPrePass(name, id)
{
}

void WaterRefractionRenderPass::Draw(Camera * camera, RenderSystem * renderSystem)
{
    if (!passCamera)
        passCamera = new Camera();        
    passCamera->CopyMathOnly(*camera);    
    passCamera->SetupDynamicParameters();    
    //add clipping plane
    PrepareVisibilityArrays(passCamera, renderSystem);
    DrawLayers(passCamera);
    
}


};
