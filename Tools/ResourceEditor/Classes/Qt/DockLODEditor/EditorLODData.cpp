/*==================================================================================
    Copyright (c) 2008, DAVA, INC
    All rights reserved.

    Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:
    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the DAVA, INC nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE DAVA, INC AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL DAVA, INC BE LIABLE FOR ANY
    DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
=====================================================================================*/

#include "EditorLODData.h"

#include "Classes/Qt/Scene/SceneSignals.h"

EditorLODData::EditorLODData()
{
    lodLayersCount = 0;
    
    connect(SceneSignals::Instance(), SIGNAL(Selected(SceneEditor2 *, DAVA::Entity *)), SLOT(EntitySelected(SceneEditor2 *, DAVA::Entity *)));
    connect(SceneSignals::Instance(), SIGNAL(Deselected(SceneEditor2 *, DAVA::Entity *)), SLOT(EntityDeselected(SceneEditor2 *, DAVA::Entity *)));

}

EditorLODData::~EditorLODData()
{
    
}


void EditorLODData::SetupFromEntity(DAVA::Entity *entity)
{
    DAVA::LodComponent *lod = DAVA::GetLodComponent(entity);
    if(lod)
    {
        lodLayersCount = lod->GetLodLayersCount();
        for(DAVA::int32 i = 0; i < lodLayersCount; ++i)
        {
            lodDistances[i] = lod->GetLodLayerDistance(i);
        }
        
        
        emit DataChanged();
    }
    else if(lodLayersCount)
    {
        Clear();
    }
}


void EditorLODData::Clear()
{
    lodLayersCount = 0;
    
    emit DataChanged();
}


DAVA::int32 EditorLODData::GetLayersCount() const
{
    return lodLayersCount;
}

DAVA::float32 EditorLODData::GetLayerDistance(DAVA::int32 layerNum) const
{
    DVASSERT(0 <= layerNum && layerNum < lodLayersCount)
    return lodDistances[layerNum];
}

void EditorLODData::SetLayerDistance(DAVA::int32 layerNum, DAVA::float32 distance)
{
    DVASSERT(0 <= layerNum && layerNum < lodLayersCount)
    if(lodDistances[layerNum] != distance)
    {
        lodDistances[layerNum] = distance;

        emit LODLevelChanged(layerNum, distance);
    }
}

void EditorLODData::EntitySelected(SceneEditor2 *scene, DAVA::Entity *entity)
{
    SetupFromEntity(entity);
    
    //TODO: set entity lod settings
}

void EditorLODData::EntityDeselected(SceneEditor2 *scene, DAVA::Entity *entity)
{
    //TODO: clear entity lod settings
    //    editedLODData->Clear();
}


void EditorLODData::EnableForceDistance(bool enable)
{
    
}

void EditorLODData::SetForceDistance(DAVA::float32 distance)
{
    
}


