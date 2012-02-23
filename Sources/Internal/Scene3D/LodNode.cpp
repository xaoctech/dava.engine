/*==================================================================================
    Copyright (c) 2008, DAVA Consulting, LLC
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are met:
    * Redistributions of source code must retain the above copyright
    notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
    notice, this list of conditions and the following disclaimer in the
    documentation and/or other materials provided with the distribution.
    * Neither the name of the DAVA Consulting, LLC nor the
    names of its contributors may be used to endorse or promote products
    derived from this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE DAVA CONSULTING, LLC AND CONTRIBUTORS "AS IS" AND
    ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL DAVA CONSULTING, LLC BE LIABLE FOR ANY
    DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
    ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
    SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

    Revision History:
        * Created by Vitaliy Borodovsky 
=====================================================================================*/
#include "Scene3D/LodNode.h"
#include "Scene3D/Scene.h"
#include "Scene3D/SceneFileV2.h"
#include "Render/3D/StaticMesh.h"
#include "Render/RenderManager.h"
#include "Render/RenderHelper.h"
#include "Utils/StringFormat.h"
#include "Scene3D/SceneNodeAnimation.h"
#include "Scene3D/SceneNodeAnimationList.h"

namespace DAVA 
{
    
    
REGISTER_CLASS(LodNode);
    

LodNode::LodNode(Scene * _scene)
:	SceneNode(_scene)
,   currentLod(NULL)
,   lastLodUpdateFrame(0)
{
	
}
	
LodNode::~LodNode()
{
//    const List<LodData>::const_iterator & end = lodLayers.end();
//    for (List<LodData>::iterator it = lodLayers.begin(); it != end; ++it)
//    {
//        LodData & ld = *it;
//        size_t size = ld.materials.size();
//        for (size_t idx = 0; idx < size; ++idx)
//        {
//            SafeRelease(ld.materials[idx]);
//            SafeRelease(ld.meshes[idx]);
//        }
//    }
}

void LodNode::AddNodeInLayer(SceneNode * node, int32 layer)
{
    AddNode(node);
    RegisterNodeInLayer(node, layer);
}

LodNode::LodData	*LodNode::CreateNewLayer(int32 layer)
{
    LodData *ld = NULL;
    if (lodLayers.empty()) 
    {
        LodData d;
        d.layer = layer;
        lodLayers.push_back(d);
        ld = &(*lodLayers.begin());
        SetCurrentLod(ld);
    }
    else 
    {
        bool isFind = false;
        for (List<LodData>::iterator it = lodLayers.begin(); it != lodLayers.end(); it++)
        {
            if (it->layer == layer) 
            {
                ld = &(*it);
                isFind = true;
                return ld;
            }
            if (layer < it->layer)
            {
                isFind = true;
                LodData d;
                d.layer = layer;
                List<LodData>::iterator newIt = lodLayers.insert(it, d);
                ld = &(*newIt);
                return ld;
            }
        }
        if (!isFind) 
        {
            LodData d;
            d.layer = layer;
            lodLayers.push_back(d);
            ld = &(*lodLayers.rbegin());
        }
    }
    
    return ld;
}

void LodNode::RegisterNodeInLayer(SceneNode * node, int32 layer)
{
    LodData *ld = CreateNewLayer(layer);
    if (node->GetName().find("dummy") != String::npos) 
    {
        if (ld->nodes.empty()) 
        {
            ld->isDummy = true;
        }
    }
    else 
    {
        ld->isDummy = false;
    }

    ld->nodes.push_back(node);
    node->AddFlagRecursive(SceneNode::NODE_IS_LOD_PART);
    if (ld != currentLod) 
    {
        node->SetUpdatable(false);
    }
    
}
    
void LodNode::RegisterIndexInLayer(int32 nodeIndex, int32 layer)
{
    LodData *ld = CreateNewLayer(layer);
    ld->indexes.push_back(nodeIndex);
}

    
void LodNode::RemoveNode(SceneNode * node)
{
    SceneNode::RemoveNode(node);
    List<LodData>::iterator ei = lodLayers.end();
    for (List<LodData>::iterator i = lodLayers.begin(); i != ei; i++) 
    {
        Vector<SceneNode*>::iterator eit = i->nodes.end();
        for (Vector<SceneNode*>::iterator it = i->nodes.begin(); it != eit; it++) 
        {
            if (*it == node) 
            {
                node->RemoveFlagRecursive(SceneNode::NODE_IS_LOD_PART);
                i->nodes.erase(it);
                return;
            }
        }
    }
}
    
void LodNode::RemoveAllChildren()
{
    SceneNode::RemoveAllChildren();
    RemoveFlagRecursive(SceneNode::NODE_IS_LOD_PART);
    lodLayers.clear();
    currentLod = NULL;
}
    
bool LodNode::IsLodMain(SceneNode *childToCheck)
{
    if (!childToCheck) 
    {
        return true;
    }
    if (childToCheck->GetName().find("dummy") != String::npos) 
    {
        return true;
    }
    
    List<LodData>::iterator ei = lodLayers.end();
    for (List<LodData>::iterator i = lodLayers.begin(); i != ei; i++) 
    {
        if(i->isDummy) 
        {
            continue;
        }
        Vector<SceneNode*>::iterator eit = i->nodes.end();
        for (Vector<SceneNode*>::iterator it = i->nodes.begin(); it != eit; it++) 
        {
            if (*it == childToCheck) 
            {
                return true;
            }
        }
        return false;
    }
    return false;
}

    
void LodNode::RecheckLod()
{
//#define LOD_DEBUG
    if (!currentLod)return;

    if (scene->GetForceLodLayer() != -1) 
    {
        for (List<LodData>::iterator it = lodLayers.begin(); it != lodLayers.end(); it++)
        {
            if (it->layer >= scene->GetForceLodLayer())
            {
                currentLod = &(*it);
                return;
            }
        }
            return;
    }
    
#ifdef LOD_DEBUG
    int32 cl = currentLod->layer;
#endif
    {
        float32 dst = (scene->GetCurrentCamera()->GetPosition() - GetWorldTransform().GetTranslationVector()).SquareLength();
        dst *= scene->GetCurrentCamera()->GetZoomFactor() * scene->GetCurrentCamera()->GetZoomFactor();
        if (dst > scene->GetLodLayerFarSquare(currentLod->layer) || dst < scene->GetLodLayerNearSquare(currentLod->layer))
        {
            for (List<LodData>::iterator it = lodLayers.begin(); it != lodLayers.end(); it++)
            {
                if (dst >= scene->GetLodLayerNearSquare(it->layer))
                {
                    currentLod = &(*it);
                }
                else 
                {
#ifdef LOD_DEBUG
                    if (cl != currentLod->layer) 
                    {
                        Logger::Info("Switch lod to %d", currentLod->layer);
                    }
#endif
                    return;
                }
            }
        }
    }
#ifdef LOD_DEBUG
    if (cl != currentLod->layer) 
    {
        Logger::Info("Switch lod to %d", currentLod->layer);
    }
#endif
}

void LodNode::SetCurrentLod(LodData *newLod)
{
    if (newLod != currentLod) 
    {
        if (currentLod) 
        {
            int32 size = currentLod->nodes.size();
            for (int i = 0; i < size; i++) 
            {
                currentLod->nodes[i]->SetUpdatable(false);
            }
        }
        currentLod = newLod;
        int32 size = currentLod->nodes.size();
        for (int i = 0; i < size; i++) 
        {
            currentLod->nodes[i]->SetUpdatable(true);
        }
    }
}


void LodNode::Update(float32 timeElapsed)
{
    
    if (flags&SceneNode::NODE_VISIBLE)
    {
        lastLodUpdateFrame++;
        if (lastLodUpdateFrame > 3)
        {
            lastLodUpdateFrame = 0;
            LodData *oldLod = currentLod;
            RecheckLod();
            if (oldLod != currentLod) 
            {
                if (oldLod) 
                {
                    int32 size = oldLod->nodes.size();
                    for (int i = 0; i < size; i++) 
                    {
                        oldLod->nodes[i]->SetUpdatable(false);
                    }
                }
                int32 size = currentLod->nodes.size();
                for (int i = 0; i < size; i++) 
                {
                    currentLod->nodes[i]->SetUpdatable(true);
                }
            }
        }
    }
    
    SceneNode::Update(timeElapsed);
    
}

	
SceneNode* LodNode::Clone(SceneNode *dstNode)
{
    if (!dstNode) 
    {
        dstNode = new LodNode(scene);
    }
    
    SceneNode::Clone(dstNode);
    LodNode *nd = (LodNode *)dstNode;

    nd->lodLayers = lodLayers;
    int32 lodIdx = 0;// Don't ask me how it's works
    const List<LodData>::const_iterator &end = nd->lodLayers.end();
    nd->currentLod = &(*nd->lodLayers.begin());
    for (List<LodData>::iterator it = nd->lodLayers.begin(); it != end; ++it)
    {
        LodData & ld = *it;
        size_t size = ld.nodes.size();
        for (size_t idx = 0; idx < size; ++idx)
        {
            for (int i = 0; i < children.size(); i++) 
            {
                if(children[i] == ld.nodes[idx])
                {
                    ld.nodes[idx] = nd->children[i];
                    if (nd->currentLod != &ld) 
                    {
                        ld.nodes[idx]->SetUpdatable(false);
                    }
                    else 
                    {
                        ld.nodes[idx]->SetUpdatable(true);
                    }

                    break;
                }
            }
        }
        lodIdx++;
    }
    return dstNode;
}
    /**
     \brief virtual function to save node to KeyedArchive
     */
void LodNode::Save(KeyedArchive * archive, SceneFileV2 * sceneFile)
{
    SceneNode::Save(archive, sceneFile);
    archive->SetInt32("lodCount", (int32)lodLayers.size());
    
    int32 lodIdx = 0;
    const List<LodData>::const_iterator &end = lodLayers.end();
    for (List<LodData>::iterator it = lodLayers.begin(); it != end; ++it)
    {
        LodData & ld = *it;
        archive->SetInt32(Format("lod%d_layer", lodIdx), (int32)ld.layer);
        size_t size = ld.nodes.size();
        archive->SetInt32(Format("lod%d_cnt", lodIdx), (int32)size);
        for (size_t idx = 0; idx < size; ++idx)
        {
            for (int i = 0; i < children.size(); i++) 
            {
                if(children[i] == ld.nodes[idx])
                {
                    archive->SetInt32(Format("l%d_%d_ni", lodIdx, idx), i);
                    break;
                }
            }
        }
        lodIdx++;
    }
}
    
    /**
     \brief virtual function to load node to KeyedArchive
     */
void LodNode::Load(KeyedArchive * archive, SceneFileV2 * sceneFile)
{
    SceneNode::Load(archive, sceneFile);

    int32 lodCount = archive->GetInt32("lodCount", 0);
    
    for (int32 lodIdx = 0; lodIdx < lodCount; ++lodIdx)
    {
        int32 layer = archive->GetInt32(Format("lod%d_layer", lodIdx), 0);
        size_t size = archive->GetInt32(Format("lod%d_cnt", lodIdx), 0);
        for (size_t idx = 0; idx < size; ++idx)
        {
            
            int32 index  = archive->GetInt32(Format("l%d_%d_ni", lodIdx, idx), 0);
            RegisterIndexInLayer(index, layer);
        }
    }
}
    
void LodNode::SceneDidLoaded()
{
    SceneNode::SceneDidLoaded();
    const List<LodData>::const_iterator &end = lodLayers.end();
    for (List<LodData>::iterator it = lodLayers.begin(); it != end; ++it)
    {
        LodData & ld = *it;
        size_t size = ld.indexes.size();
        for (size_t idx = 0; idx < size; ++idx)
        {
            ld.nodes.push_back(children[ld.indexes[idx]]);
//            if (&ld != currentLod) 
            {
                children[ld.indexes[idx]]->SetUpdatable(false);
            }
        }
    }

    currentLod = NULL;
    if (lodLayers.size() > 0)
    {
        SetCurrentLod(&(*lodLayers.rbegin()));
        lastLodUpdateFrame = 1000;
    }
    
}

int32 LodNode::GetMaxLodLayer()
{
	int32 ret = -1;
	const List<LodData>::const_iterator &end = lodLayers.end();
	for (List<LodData>::iterator it = lodLayers.begin(); it != end; ++it)
	{
		LodData & ld = *it;
		if(ld.layer > ret)
		{
			ret = ld.layer;
		}
	}

	return ret;
}

    
};
