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
#include "Scene3D/MeshInstanceNode.h"
#include "Scene3D/Scene.h"
#include "Render/3D/StaticMesh.h"
#include "Render/RenderManager.h"
#include "Render/RenderHelper.h"

namespace DAVA 
{
    
    
REGISTER_CLASS(MeshInstanceNode);
    

MeshInstanceNode::MeshInstanceNode(Scene * _scene)
:	SceneNode(_scene)
,   lodPresents(false)
,   currentLod(NULL)
,   lastLodUpdateFrame(0)
{
	
}
	
MeshInstanceNode::~MeshInstanceNode()
{
    const List<LodData>::const_iterator & end = lodLayers.end();
    for (List<LodData>::iterator it = lodLayers.begin(); it != end; ++it)
    {
        LodData & ld = *it;
        size_t size = ld.materials.size();
        for (size_t idx = 0; idx < size; ++idx)
        {
            SafeRelease(ld.materials[idx]);
        }
    }
}

void MeshInstanceNode::AddPolygonGroup(StaticMesh * mesh, int32 polygonGroupIndex, Material* material)
{
    LodData *ld = NULL;
    if (lodLayers.empty()) 
    {
        LodData d;
        d.layer = 0;
        lodLayers.push_back(d);
        currentLod = &(*lodLayers.begin());
    }
    if (name.find("lod0dummy") != name.npos)
    {
        return;
    }
    ld = &(*lodLayers.begin());

	ld->meshes.push_back(mesh);
	ld->polygonGroupIndexes.push_back(polygonGroupIndex);
	ld->materials.push_back(SafeRetain(material));
	
	PolygonGroup * group = mesh->GetPolygonGroup(polygonGroupIndex);
	bbox.AddAABBox(group->GetBoundingBox());
}

void MeshInstanceNode::AddPolygonGroupForLayer(int32 layer, StaticMesh * mesh, int32 polygonGroupIndex, Material* material)
{
    LodData *ld = NULL;
    if (layer != 0) 
    {
        lodPresents = true;
    }
    if (lodLayers.empty()) 
    {
        LodData d;
        d.layer = layer;
        lodLayers.push_back(d);
        ld = &(*lodLayers.begin());
        currentLod = ld;
    }
    else 
    {
        bool isFind = false;
        for (List<LodData>::iterator it = lodLayers.begin(); it != lodLayers.end(); it++)
        {
            if (it->layer == layer) 
            {
                ld = &(*lodLayers.begin());
                isFind = true;
                break;
            }
            if (layer < it->layer)
            {
                isFind = true;
                LodData d;
                d.layer = layer;
                List<LodData>::iterator newIt = lodLayers.insert(it, d);
                ld = &(*newIt);
                break;
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

    
	ld->meshes.push_back(mesh);
	ld->polygonGroupIndexes.push_back(polygonGroupIndex);
	ld->materials.push_back(SafeRetain(material));

	if (ld->layer == 0) 
    {
        PolygonGroup * group = mesh->GetPolygonGroup(polygonGroupIndex);
        bbox.AddAABBox(group->GetBoundingBox());
    }
}

void MeshInstanceNode::AddDummyLODLayer(int32 layer)
{
    if (layer != 0) 
    {
        lodPresents = true;
    }
    if (lodLayers.empty()) 
    {
        LodData d;
        d.layer = layer;
        lodLayers.push_back(d);
        currentLod = &(*lodLayers.begin());
    }
    else 
    {
        for (List<LodData>::iterator it = lodLayers.begin(); it != lodLayers.end(); it++)
        {
            if (it->layer == layer) 
            {
                return;
            }
            if (layer < it->layer)
            {
                LodData d;
                d.layer = layer;
                List<LodData>::iterator newIt = lodLayers.insert(it, d);
                return;
            }
        }
        
        LodData d;
        d.layer = layer;
        lodLayers.push_back(d);
    }
}

    
void MeshInstanceNode::Update(float32 timeElapsed)
{
    bool needUpdateTransformBox = false;
    if (!(flags & NODE_WORLD_MATRIX_ACTUAL)) 
    {
        needUpdateTransformBox = true;
    }
    SceneNode::Update(timeElapsed);
    
    if (needUpdateTransformBox)
        bbox.GetTransformedBox(worldTransform, transformedBox);
    
    if (lodPresents && visible)
    {
        lastLodUpdateFrame++;
        if (lastLodUpdateFrame > 3)
        {
            lastLodUpdateFrame = 0;
            if (scene->GetForceLodLayer() != -1)
            {
                for (List<LodData>::iterator it = lodLayers.begin(); it != lodLayers.end(); it++)
                {
                    if (scene->GetForceLodLayer() == it->layer)
                    {
                        currentLod = &(*it);
                        return;
                    }
                }
            }
            else 
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
                                //                        Logger::Info("Draw selected LOD %d", currentLod->layer);
                            return;
                        }
                    }
                }
            }

        }
//        Logger::Info("Draw selected LOD %d", currentLod->layer);
    }
    
}
    
void MeshInstanceNode::Draw()
{
	if (!visible)return;
        
//    if (GetFullName() == String("MaxScene->node-Cylinder01->VisualSceneNode14->instance_0"))
//    {
//        RenderManager::Instance()->SetColor(1.0f, 0.0f, 0.0f, 1.0f);
//        RenderHelper::Instance()->DrawBox(transformedBox);
//        RenderManager::Instance()->SetColor(1.0f, 1.0f, 1.0f, 1.0f);
//    }    

    if (!scene->GetClipCamera()->GetFrustum()->IsInside(transformedBox))
    {
        return;
    }
		
	Matrix4 prevMatrix = RenderManager::Instance()->GetMatrix(RenderManager::MATRIX_MODELVIEW); 
	Matrix4 meshFinalMatrix = worldTransform * prevMatrix;
    RenderManager::Instance()->SetMatrix(RenderManager::MATRIX_MODELVIEW, meshFinalMatrix);

//    /* float32 proj[16];
//    glGetFloatv(GL_MODELVIEW_MATRIX, proj);
//    
//    for (int32 k = 0; k < 16; ++k)
//    {
//        if (proj[k] != prevMatrix.data[k])
//        {
//            printf("k:%d - %0.3f = %0.3f\n", k, proj[k], prevMatrix.data[k]);
//        }
//    } */
//    
    
    //glMatrixMode(GL_MODELVIEW);
    //glPushMatrix();
    //glMultMatrixf(worldTransform.data);
    
    uint32 meshesSize = (uint32)currentLod->meshes.size();

	for (uint32 k = 0; k < meshesSize; ++k)
	{
		currentLod->meshes[k]->DrawPolygonGroup(currentLod->polygonGroupIndexes[k], currentLod->materials[k]);
	}
	
	if (debugFlags != DEBUG_DRAW_NONE)
	{
        //RenderManager::PushState();
        RenderManager::Instance()->SetState(RenderStateBlock::STATE_DEPTH_WRITE | RenderStateBlock::STATE_CULL); 
//        RenderManager::Instance()->EnableDepthTest(false);
//		RenderManager::Instance()->EnableTexturing(false);
		RenderManager::Instance()->FlushState();
		
		
		if (debugFlags & DEBUG_DRAW_AABBOX)
		{
			RenderManager::Instance()->SetColor(1.0f, 1.0f, 1.0f, 1.0f);
			RenderHelper::Instance()->DrawBox(bbox);
		
			//RenderManager::Instance()->SetColor(1.0f, 0.0f, 0.0f, 1.0f);
            //bbox.GetTransformedBox(worldTransform, transformedBox);
			//RenderHelper::Instance()->DrawBox(transformedBox);
		}
        
		if (debugFlags & DEBUG_DRAW_LOCAL_AXIS)
		{
			RenderManager::Instance()->SetColor(1.0f, 0.0f, 0.0f, 1.0f); 
			RenderHelper::Instance()->DrawLine(Vector3(0.0f, 0.0f, 0.0f), Vector3(50.0f, 0.0f, 0.0f));
			
			RenderManager::Instance()->SetColor(0.0f, 1.0f, 0.0f, 1.0f);
			RenderHelper::Instance()->DrawLine(Vector3(0.0f, 0.0f, 0.0f), Vector3(0.0f, 50.0f, 0.0f));
			
			RenderManager::Instance()->SetColor(0.0f, 0.0f, 1.0f, 1.0f);
			RenderHelper::Instance()->DrawLine(Vector3(0.0f, 0.0f, 0.0f), Vector3(0.0f, 0.0f, 50.0f));
		}

		if (debugFlags & DEBUG_DRAW_AABOX_CORNERS)
		{
			RenderManager::Instance()->SetColor(1.0f, 1.0f, 1.0f, 1.0f);
			float32 offs = ((bbox.max - bbox.min).Length()) * 0.05f;
			Vector3 off = Vector3(offs, offs, offs);			
			AABBox3 newBox(bbox.min - off, bbox.max + off);

//			RenderHelper::Instance()->DrawLine(newBox.min, newBox.min + Vector3(0, 0, offs));
//			RenderHelper::Instance()->DrawLine(newBox.min, newBox.min + Vector3(0, offs, 0));
//			RenderHelper::Instance()->DrawLine(newBox.min, newBox.min + Vector3(offs, 0, 0));
//
//			RenderHelper::Instance()->DrawLine(newBox.max, newBox.max - Vector3(0, 0, offs));
//			RenderHelper::Instance()->DrawLine(newBox.max, newBox.max - Vector3(0, offs, 0));
//			RenderHelper::Instance()->DrawLine(newBox.max, newBox.max - Vector3(offs, 0, 0));
			RenderHelper::Instance()->DrawBox(newBox);
		}
		
		
		
//      RenderManager::Instance()->EnableDepthTest(true);
//		RenderManager::Instance()->EnableTexturing(true);
        RenderManager::Instance()->SetState(RenderStateBlock::DEFAULT_3D_STATE);
        RenderManager::Instance()->SetColor(1.0f, 1.0f, 1.0f, 1.0f);
	}
	//glPopMatrix();
    RenderManager::Instance()->SetMatrix(RenderManager::MATRIX_MODELVIEW, prevMatrix);

    SceneNode::Draw();
}


SceneNode* MeshInstanceNode::Clone(SceneNode *dstNode)
{
    if (!dstNode) 
    {
        dstNode = new MeshInstanceNode(scene);
    }

    SceneNode::Clone(dstNode);
    MeshInstanceNode *nd = (MeshInstanceNode *)dstNode;
    nd->lodLayers = lodLayers;
    nd->lodPresents = lodPresents;
    nd->lastLodUpdateFrame = 1000;
    nd->currentLod = &(*nd->lodLayers.begin());
//    nd->meshes = meshes;
//    nd->polygonGroupIndexes = polygonGroupIndexes;
//    nd->materials = materials;
    nd->bbox = bbox;
    
    return dstNode;
}

AABBox3 MeshInstanceNode::GetWTMaximumBoundingBox()
{
    AABBox3 retBBox = bbox;
	
    bbox.GetTransformedBox(GetWorldTransform(), retBBox);
    
    Vector<SceneNode*>::iterator itEnd = childs.end();
	for (Vector<SceneNode*>::iterator it = childs.begin(); it != itEnd; ++it)
    {
        AABBox3 box = (*it)->GetWTMaximumBoundingBox();
        if(  (AABBOX_INFINITY != box.min.x && AABBOX_INFINITY != box.min.y && AABBOX_INFINITY != box.min.z)
           &&(-AABBOX_INFINITY != box.max.x && -AABBOX_INFINITY != box.max.y && -AABBOX_INFINITY != box.max.z))
        {
            retBBox.AddAABBox(box);
        }
    }
    
    return retBBox;
}

    
};
