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



#include "DeleteLODCommand.h"
#include "EntityRemoveCommand.h"


DeleteLODCommand::DeleteLODCommand(DAVA::LodComponent *lod, DAVA::int32 lodIndex)
	: Command2(CMDID_LOD_DELETE, "Delete first LOD")
	, lodComponent(lod)
    , deletedLodIndex(lodIndex)
{
    if(lodComponent)
    {
        lodLayers = lodComponent->lodLayers;
        lodLayersArray = lodComponent->lodLayersArray;

        if(lodLayers.size())
        {
            DAVA::uint32 nodesSize = lodLayers[deletedLodIndex].nodes.size();
            for(DAVA::uint32 i = 0; i < nodesSize; ++i)
            {
                nodes.push_back(DAVA::SafeRetain(lodLayers[deletedLodIndex].nodes[i]));

                //we need remove entities from scene
                if(nodes[i]->GetParent())
                {
                    //we need remove entiies with only one layer occurence
                    DAVA::int32 occurenceCount = CountOccurence(nodes[i]);
                    if(occurenceCount == 1)
                    {
                        EntityRemoveCommand *removeCommand = new EntityRemoveCommand(nodes[i]);
                        commands.push_back(removeCommand);
                    }
                }
            }
        }
    }
}

DeleteLODCommand::~DeleteLODCommand()
{
    DAVA::uint32 nodesSize = nodes.size();
    for(DAVA::uint32 i = 0; i < nodesSize; ++i)
    {
        DAVA::SafeRelease(nodes[i]);
    }
    nodes.clear();
    
    DAVA::uint32 commandsSize = commands.size();
    for(DAVA::uint32 i = 0; i < commandsSize; ++i)
    {
        DAVA::SafeDelete(commands[i]);
    }
    commands.clear();
}

void DeleteLODCommand::Redo()
{
	if(!lodComponent) return;
    
    //remove nodes from scene
    DAVA::uint32 commandsSize = commands.size();
    for(DAVA::uint32 i = 0; i < commandsSize; ++i)
    {
        RedoInternalCommand(commands[i]);
    }
    
    //remove lodlayer
    DAVA::Vector<DAVA::LodComponent::LodData>::iterator deleteIt = lodComponent->lodLayers.begin();
    std::advance(deleteIt, deletedLodIndex);
    lodComponent->lodLayers.erase(deleteIt);
    
    //update distances
    DAVA::int32 layersSize = lodComponent->GetLodLayersCount();
    for(DAVA::int32 i = deletedLodIndex; i < DAVA::LodComponent::MAX_LOD_LAYERS-1; ++i)
    {
        lodComponent->lodLayersArray[i] = lodComponent->lodLayersArray[i+1];
    }

    //last lod
    if(layersSize)
    {
        lodComponent->lodLayersArray[layersSize-1].SetFarDistance(2 * DAVA::LodComponent::MAX_LOD_DISTANCE);
    }
    
    //first lod
    lodComponent->SetLodLayerDistance(0, 0);
    lodComponent->lodLayersArray[0].SetNearDistance(0.0f);
    
    //visual part
    lodComponent->currentLod = DAVA::LodComponent::INVALID_LOD_LAYER;
    lodComponent->forceLodLayer = DAVA::LodComponent::INVALID_LOD_LAYER;
}

void DeleteLODCommand::Undo()
{
	if(!lodComponent) return;
    
    //restore entities
    DAVA::uint32 commandsSize = commands.size();
    for(DAVA::uint32 i = 0; i < commandsSize; ++i)
    {
        UndoInternalCommand(commands[i]);
        
        commands[i]->GetEntity()->SetLodVisible(false);
    }
    
    //restore lodlayers and disatnces
    lodComponent->lodLayers = lodLayers;
    lodComponent->lodLayersArray = lodLayersArray;
}

DAVA::int32 DeleteLODCommand::CountOccurence(DAVA::Entity *entity) const
{
    DAVA::int32 occurenceCounter = 0;
    
    DAVA::uint32 layersSize = lodLayers.size();
    for(DAVA::uint32 layer = 0; layer < layersSize; ++layer)
    {
        DAVA::uint32 nodesSize = lodLayers[layer].nodes.size();
        for(DAVA::uint32 node = 0; node < nodesSize; ++node)
        {
            if(entity == lodLayers[layer].nodes[node])
            {
                ++occurenceCounter;
            }
        }
    }
    
    return occurenceCounter;
}


DAVA::Entity * DeleteLODCommand::GetEntity() const
{
	if(lodComponent)
		return lodComponent->GetEntity();

	return NULL;
}



