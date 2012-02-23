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
        * Created by Alexey Prosin 
=====================================================================================*/
#ifndef __DAVAENGINE_LOD_NODE_H__
#define __DAVAENGINE_LOD_NODE_H__

#include "Scene3D/SceneNode.h"

namespace DAVA 
{
class Scene;
class StaticMesh;
class Material;
class Texture;
class SceneFileV2;
    
class LodNode : public SceneNode
{
public:	
    
    struct LodData
    {
        LodData()
        :layer(-1)
        ,isDummy(false)
        {
        }
        Vector<SceneNode*> nodes;
        Vector<int32> indexes;
        int32 layer;
        bool isDummy;
    };
    
	LodNode(Scene * _scene = 0);
	~LodNode();
	
    
    virtual void	AddNodeInLayer(SceneNode * node, int32 layer);//adds new node and registers this node as a LOD layer
    virtual void	RegisterNodeInLayer(SceneNode * node, int32 layer);//register existing node as a layer
	virtual void	RemoveNode(SceneNode * node);
	virtual void	RemoveAllChildren();
    
    virtual void Update(float32 timeElapsed);
    void SimpleUpdate(float32 timeElapsed);
	
    virtual SceneNode* Clone(SceneNode *dstNode = NULL);
    /**
        \brief virtual function to save node to KeyedArchive
     */
    virtual void Save(KeyedArchive * archive, SceneFileV2 * sceneFile);
    
    /**
        \brief virtual function to load node to KeyedArchive
     */
	virtual void Load(KeyedArchive * archive, SceneFileV2 * sceneFile);
    
    void SetCurrentLod(LodData *newLod);

    virtual void SceneDidLoaded();
    
    virtual bool IsLodMain(SceneNode *childToCheck = NULL);//if childToCheck is NULL checks the caller node

	int32 GetMaxLodLayer();


protected:
//    virtual SceneNode* CopyDataTo(SceneNode *dstNode);

    virtual void	RegisterIndexInLayer(int32 nodeIndex, int32 layer);
    virtual LodData	*CreateNewLayer(int32 layer);

    void RecheckLod();
    
    LodData *currentLod;
    List<LodData> lodLayers;
    
    
    int lastLodUpdateFrame;

};
	
};

#endif // __DAVAENGINE_MESH_INSTANCE_H__
