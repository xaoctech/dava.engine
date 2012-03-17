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
#ifndef __DAVAENGINE_MESH_INSTANCE_H__
#define __DAVAENGINE_MESH_INSTANCE_H__

#include "Scene3D/SceneNode.h"

namespace DAVA 
{
class Scene;
class StaticMesh;
class Material;
class Texture;
class SceneFileV2;
class PolygonGroup;
class MeshInstanceNode;
class LightNode;
class InstanceMaterialState;
    
class PolygonGroupWithMaterial : public BaseObject
{
public:
    PolygonGroupWithMaterial(StaticMesh * mesh, int32 polygroupIndex, Material * material);
    virtual ~PolygonGroupWithMaterial();
    
    StaticMesh * GetMesh();
    int32 GetPolygroupIndex();
    PolygonGroup * GetPolygonGroup();
    Material * GetMaterial();
    
private:
    StaticMesh * mesh;
    int32 polygroupIndex;
    Material * material;
    friend class MeshInstanceNode;
};

    
class MeshInstanceNode : public SceneNode
{
public:	
	struct LightmapData
	{
		Texture * lightmap;
		String lightmapName;
		Vector2 uvOffset;
		Vector2 uvScale;
	};

	MeshInstanceNode(Scene * _scene = 0);
	~MeshInstanceNode();
	
	void AddPolygonGroup(StaticMesh * mesh, int32 polygonGroupIndex, Material* material);

    virtual void Update(float32 timeElapsed);
	virtual void Draw();
	
	inline void SetVisible(bool isVisible);
	inline bool GetVisible();
	
	inline AABBox3 & GetBoundingBox();
	
    Vector<PolygonGroupWithMaterial*> & GetPolygonGroups();
    	
    virtual SceneNode* Clone(SceneNode *dstNode = NULL);
    
    //Returns maximum Bounding Box as WorlTransformedBox
    virtual AABBox3 GetWTMaximumBoundingBox();

	
    /**
        \brief virtual function to save node to KeyedArchive
     */
    virtual void Save(KeyedArchive * archive, SceneFileV2 * sceneFile);
    
    /**
        \brief virtual function to load node to KeyedArchive
     */
	virtual void Load(KeyedArchive * archive, SceneFileV2 * sceneFile);

	/**
        \brief Add lightmap texture.
        Consequent calls of this function will add lightmaps to consequent PolygonGroups of this MeshInstance.
            
        Normal usage (pseudocode):
        \code
        ClearLightmaps();
        for(each polygon group)
        {
            AddLightmap(...);
        }
        \endcode

        \param[in] lightmapName path to texture
	 */
	void AddLightmap(int32 polygonGroupIndex, const LightmapData & lightmapData);

	/**
        \brief Delete all lightmaps for this MeshInstance. 
	 */
	void ClearLightmaps();
    
	/**
        \brief Replace material for polygon group. 
	 */
    void ReplaceMaterial(Material *material, int32 index);

	void CreateDynamicShadowNode();
	void DeleteDynamicShadowNode();
	void ConvertToShadowVolume();
    
    virtual void GetDataNodes(Set<DataNode*> & dataNodes);


    Texture * GetLightmapForIndex(int32 index);
    int32 GetLightmapCount();

    /**
        \brief Bake transformations into polygon groups if polygon groups single instanced.
     */
    virtual void BakeTransforms();
    
    /**
        \brief Register nearest node to this MeshInstanceNode.
        MeshInstance can have own criteria of detection on which light nodes are interesting for this particular mesh and which are not.
     */
    virtual void RegisterNearestLight(LightNode * node);

protected:
    virtual void UpdateLights();
    
//    virtual SceneNode* CopyDataTo(SceneNode *dstNode);
    
    Vector<PolygonGroupWithMaterial*> polygroups;
    Vector<LightNode*> nearestLights;
    InstanceMaterialState * materialState;
    
	AABBox3 bbox;
    AABBox3 transformedBox;
    
	Vector<LightmapData> lightmaps;

};
	
inline AABBox3 & MeshInstanceNode::GetBoundingBox()
{
	return bbox;
}

};

#endif // __DAVAENGINE_MESH_INSTANCE_H__
