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

#include "Scene3D/Systems/FoliageSystem.h"

#include "Render/Highlevel/Landscape.h"
#include "Render/Highlevel/Vegetation/VegetationRenderObject.h"
#include "Scene3D/Components/ComponentHelpers.h"
#include "Scene3D/Systems/WindSystem.h"

namespace DAVA
{
    
static const int16 MAX_ANIMATED_CELL_WIDTH = 2;
    
FoliageSystem::FoliageSystem(Scene* scene) : SceneSystem(scene),
        landscapeEntity(NULL),
        foliageEntity(NULL)
{
        
}
    
FoliageSystem::~FoliageSystem()
{
    SafeRelease(landscapeEntity);
    SafeRelease(foliageEntity);
}
    
void FoliageSystem::AddEntity(Entity * entity)
{
    Landscape* landscapeRO = GetLandscape(entity);
    if(landscapeRO &&
        entity != landscapeEntity)
    {
        SafeRelease(landscapeEntity);
        landscapeEntity = SafeRetain(entity);
            landscapeRO->SetFoliageSystem(this);
        
        SyncFoliageWithLandscape();
    }
    
    VegetationRenderObject* vegetationRO = GetVegetation(entity);
    if(vegetationRO &&
       entity != foliageEntity)
    {
        SafeRelease(foliageEntity);
        foliageEntity = SafeRetain(entity);
        
        SyncFoliageWithLandscape();
    }
}

void FoliageSystem::RemoveEntity(Entity * entity)
{
    if(entity == foliageEntity)
    {
        SafeRelease(foliageEntity);
    }
    
    if(entity == landscapeEntity)
    {
        SafeRelease(landscapeEntity);
    }
}
    
void FoliageSystem::Process(float32 timeElapsed)
{
    VegetationRenderObject* vegetationRO = GetVegetation(foliageEntity);
    if(vegetationRO && vegetationRO->ReadyToRender())
    {
        WindSystem * windSystem = GetScene()->windSystem;
        
        Camera * camera = GetScene()->GetRenderSystem()->GetMainCamera();
        Vector<AbstractQuadTreeNode<SpatialData>*> & visibleCells = vegetationRO->BuildVisibleCellList(camera);
        uint32 cellsCount = visibleCells.size();
        for(uint32 i = 0; i < cellsCount; ++i)
        {
            AbstractQuadTreeNode<SpatialData>* cell = visibleCells[i];
            
            if(cell->data.width > MAX_ANIMATED_CELL_WIDTH)
                continue;
            
            const Vector3 & min = cell->data.bbox.min;
            const Vector3 & max = cell->data.bbox.max;
            
            Vector3 cellPos[4] = {
                Vector3(min.x, min.y, max.z),
                Vector3(min.x, max.y, max.z),
                Vector3(max.x, min.y, max.z),
                Vector3(max.x, max.y, max.z)
            };
            
            Vector4 layersAnimationSpring = vegetationRO->GetLayersAnimationSpring();
            
            for(uint32 layerIndex = 0; layerIndex < 4; ++layerIndex)
            {
                Vector3 windVec = windSystem->GetWind(cellPos[layerIndex]);
                Vector2 windVec2D(windVec.x, windVec.y);
                
                Vector2 & offset = cell->data.animationOffset[layerIndex];
                Vector2 & velocity = cell->data.animationVelocity[layerIndex];
                
                velocity += (windVec2D - layersAnimationSpring.data[layerIndex] * offset - 1.4f * velocity * velocity.Length()) * timeElapsed;
                offset += velocity * timeElapsed;
            }
        }
    }
}
    
void FoliageSystem::SyncFoliageWithLandscape()
{
    if(landscapeEntity && foliageEntity)
    {
        Landscape* landscapeRO = GetLandscape(landscapeEntity);
        VegetationRenderObject* vegetationRO = GetVegetation(foliageEntity);
        
        vegetationRO->SetHeightmap(landscapeRO->GetHeightmap());
        vegetationRO->SetHeightmapPath(landscapeRO->GetHeightmapPathname());
        vegetationRO->SetWorldSize(Vector3(landscapeRO->GetLandscapeSize(),
                                           landscapeRO->GetLandscapeSize(),
                                           landscapeRO->GetLandscapeHeight()));
    }
}

void FoliageSystem::SetPerturbation(const Vector3& point,
                                    const Vector3& force,
                                    float32 distance)
{
    VegetationRenderObject* vegetationRO = GetVegetation(foliageEntity);
    if(vegetationRO != NULL)
    {
        vegetationRO->SetPerturbation(point, force, distance);
    }
}

void FoliageSystem::SetFoliageVisible(bool show)
{
    VegetationRenderObject* vegetationRO = GetVegetation(foliageEntity);
    if(NULL != vegetationRO)
    {
        vegetationRO->SetVegetationVisible(show);
    }
}

bool FoliageSystem::IsFoliageVisible() const
{
    VegetationRenderObject* vegetationRO = GetVegetation(foliageEntity);
    
    return (NULL != vegetationRO) ? vegetationRO->GetVegetationVisible() : false;;
}

void FoliageSystem::DebugDrawVegetation()
{
    VegetationRenderObject* vegetationRO = GetVegetation(foliageEntity);
    if(NULL != vegetationRO)
    {
        vegetationRO->DebugDrawVisibleNodes();
    }
}

};