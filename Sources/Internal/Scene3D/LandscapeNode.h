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
#ifndef __DAVAENGINE_LANDSCAPE_NODE_H__
#define __DAVAENGINE_LANDSCAPE_NODE_H__

#include "Base/BaseObject.h"
#include "Base/BaseTypes.h"
#include "Base/BaseMath.h"
#include "Render/RenderBase.h"
#include "Scene3D/SceneNode.h"
#include "Scene3D/Frustum.h"
#include "Scene3D/LandscapeCursor.h"


namespace DAVA
{

class Scene;
class Image;
class Texture;
class RenderDataObject;
class Shader;
class SceneFileV2;
class Heightmap;
    
template<class T>
class LandQuadTreeNode
{
public:
    LandQuadTreeNode()
    {
        childs = 0;
        parent = 0;
        for (int32 k = 0; k < 4; ++k)
            neighbours[k] = 0;
    }
    ~LandQuadTreeNode()
    {
        SafeDeleteArray(childs);
    }
    
    void AllocChilds()
    {
        childs = new LandQuadTreeNode[4];
    }
    
    LandQuadTreeNode * childs;  // It's array of 4 child nodes
    LandQuadTreeNode * parent;
    LandQuadTreeNode * neighbours[4]; 
    T data;
};
    
template <class T>
class LinearQuadTree
{
public:
    
    
    

};
    
/**    
    \brief Implementation of cdlod algorithm to render landscapes
    This class is base of the landscape code on all platforms
    Landscape node is always axial aligned for simplicity of frustum culling calculations
    Keep in mind that landscape orientation cannot be changed using localTransform and worldTransform matrices. 
 */ 

class LandscapeNode : public SceneNode
{
public:	
    enum 
    {
        LEFT = 0,
        RIGHT = 1,
        TOP = 2,
        BOTTOM = 3,
    };
    
	LandscapeNode(Scene * scene = 0);
	virtual ~LandscapeNode();
    
    /**
        \brief Set lod coefficients for dynamic roam landscape
        Default values: (60, 120, 240, 480)
        Every next value should be almost twice higher than previous to avoid gaps between levels
     */
    void SetLods(const Vector4 & lods);
    

    enum eRenderingMode
    {
        RENDERING_MODE_TEXTURE = 0,
        RENDERING_MODE_DETAIL_SHADER, 
        RENDERING_MODE_BLENDED_SHADER,  // revision 1 of blender shader. Support only 2 textures per landscape.
        RENDERING_MODE_TILE_MASK_SHADER,  // revision 2 of blender shader. Support 4 textures per landscape.
    };
    
    /**
        \brief Change rendering mode. 
        \param[in] renderingMode rendering mode of landscape.
     */
    void SetRenderingMode(eRenderingMode _renderingMode);
    
    /**
        \brief Get rendering mode. 
        \returns rendering mode of landscape.
     */
    inline const eRenderingMode GetRenderingMode();

    
    /**
        \brief Builds landscape from heightmap image and bounding box of this landscape block
        \param[in] renderingMode rendering mode of landscape.
        \param[in] landscapeBox axial-aligned bounding box of the landscape block
     */
    void BuildLandscapeFromHeightmapImage(eRenderingMode renderingMode, const String & heightmapPathname, const AABBox3 & landscapeBox);
    
    enum eTextureLevel
    {
        TEXTURE_COLOR = 0,  // in case of BLENDED_SHADER in alpha channel it can be tile mask for TILED_TEXTURES
        TEXTURE_TILE_MASK,
        TEXTURE_TILE0,   
        TEXTURE_TILE1,
        TEXTURE_TILE2,
        TEXTURE_TILE3,
        // TEXTURE_BUMP,
        
        TEXTURE_DETAIL, 
        TEXTURE_COUNT,
    };

	class LandscapeVertex
	{
	public:
		Vector3 position;
		Vector2 texCoord;
	};
    
    /**
        \brief Set texture for the specific texture level
        
     To render landscape you need to set textures.  
        For RENDERING_MODE_TEXTURE you need to set only TEXTURE_TEXTURE0.
        For RENDERING_MODE_DETAIL_SHADER you have to set TEXTURE_TEXTURE0 and TEXTURE_DETAIL
        For RENDERING_MODE_BLENDED_SHADER you have to set TEXTURE_TEXTURE0, TEXTURE_TEXTURE1, TEXTURE_TEXTUREMASK
          
        \param[in] level level of texture you want to set
        \param[in] textureName name of texture you want to open and set to specific level
     */
    void SetTexture(eTextureLevel level, const String & textureName);

    
    /**
     \brief Set texture for the specific texture level
     
     To render landscape you need to set textures.  
     For RENDERING_MODE_TEXTURE you need to set only TEXTURE_TEXTURE0.
     For RENDERING_MODE_DETAIL_SHADER you have to set TEXTURE_TEXTURE0 and TEXTURE_DETAIL
     For RENDERING_MODE_BLENDED_SHADER you have to set TEXTURE_TEXTURE0, TEXTURE_TEXTURE1, TEXTURE_TEXTUREMASK
     
     \param[in] level level of texture you want to set
     \param[in] texture you want to set to specific level
     */
    void SetTexture(eTextureLevel level, Texture * texture);

	/**
        \brief Get texture that was previously set in SetTexture.
        \param[in] level 
        \returns current texture
	 */
	Texture * GetTexture(eTextureLevel level);
    
	/**
        \brief Get texture name that was previously set in SetTexture.
        \param[in] level 
        \returns current texture name
	 */
    const String & GetTextureName(eTextureLevel level);
    
    
	/**
        \brief Set tiling for specific texture level.
        This function gives you can control of tiling for specific landscape level.
     */    
    void SetTextureTiling(eTextureLevel level, const Vector2 & tiling);

    /**
        \brief Get tiling for specified texture level.
        \returns Tiling for specified texture level.
     */
    const Vector2 & GetTextureTiling(eTextureLevel level); 
    
    /**
        \brief Overloaded draw function to draw landscape
     */
	virtual void Draw();

	/**
        \brief Get landscape mesh geometry.
        Unoptimized lod0 mesh is returned.
        \param[out] vertices landscape vertices
        \param[out] indices landscape indices
	 */
	void GetGeometry(Vector<LandscapeVertex> & vertices, Vector<int32> & indices);
    
    //Returns maximum Bounding Box as WorlTransformedBox
    virtual AABBox3 GetWTMaximumBoundingBox();

	inline AABBox3 & GetBoundingBox();

    /**
        \brief Function to receive pathname of heightmap object
        \returns pathname of heightmap
     */
    const String & GetHeightMapPathname();

    
    void Save(KeyedArchive * archive, SceneFileV2 * sceneFile);
    void Load(KeyedArchive * archive, SceneFileV2 * sceneFile);
    
    // TODO: Need comment here
	bool PlacePoint(const Vector3 & point, Vector3 & result);
	Vector3 GetPoint(int16 x, int16 y, uint16 height);

	void CursorEnable();
	void CursorDisable();
	void SetCursorTexture(Texture * texture);
	void SetBigTextureSize(float32 bigSize);
	void SetCursorPosition(const Vector2 & position);
	void SetCursorScale(float32 scale);

    Heightmap *GetHeightmap();
    
protected:	
    
    class LandscapeQuad
    {
    public:
        int16   x, y;
        //int16   xbuf, ybuf;
        int16   size;
        int8    lod;
        int8    rdoQuad;
        AABBox3 bbox;
        uint32  frame;
    };
   
    static const int32 RENDER_QUAD_WIDTH = 129;
    static const int32 RENDER_QUAD_AND = RENDER_QUAD_WIDTH - 2;
    static const int32 INDEX_ARRAY_COUNT = RENDER_QUAD_WIDTH * RENDER_QUAD_WIDTH * 6;
    

    void RecursiveBuild(LandQuadTreeNode<LandscapeQuad> * currentNode, int32 level, int32 maxLevels);
    LandQuadTreeNode<LandscapeQuad> * FindNodeWithXY(LandQuadTreeNode<LandscapeQuad> * currentNode, int16 quadX, int16 quadY, int16 quadSize);
    void FindNeighbours(LandQuadTreeNode<LandscapeQuad> * currentNode);
    void MarkFrames(LandQuadTreeNode<LandscapeQuad> * currentNode, int32 & depth);

    void BindMaterial();
    void UnbindMaterial();
    
    void DrawQuad(LandQuadTreeNode<LandscapeQuad> * currentNode, int8 lod);
    void Draw(LandQuadTreeNode<LandscapeQuad> * currentNode);
    void DrawFans();

    AABBox3     box;
    
    int8 AllocateRDOQuad(LandscapeQuad * quad);
    void ReleaseAllRDOQuads();

    Vector<LandscapeVertex *> landscapeVerticesArray;
    Vector<RenderDataObject *> landscapeRDOArray;
    
    uint16 * indices;
    Texture * textures[TEXTURE_COUNT];
    String textureNames[TEXTURE_COUNT];
    
    int32 lodLevelsCount;
    float32 lodDistance[8]; //
    float32 lodSqDistance[8];
    
    LandQuadTreeNode<LandscapeQuad> quadTreeHead;

    List<LandQuadTreeNode<LandscapeQuad>*> fans;
    
    int32 allocatedMemoryForQuads;
    
    Vector3 cameraPos;
    Frustum *frustum;
    
    eRenderingMode renderingMode;
    ePrimitiveType primitypeType;

    void InitShaders();
    void ReleaseShaders();
    
    int32 uniformCameraPosition;
    int32 uniformTextures[TEXTURE_COUNT];
    int32 uniformTextureTiling[TEXTURE_COUNT];
    Vector2 textureTiling[TEXTURE_COUNT];
    
    Shader * activeShader;

	LandscapeCursor * cursor;

//    Shader * singleTextureShader;
//    Shader * detailShader;
//    Shader * blendedShader;
        
    int8 queueRdoQuad;
    int32 queueRenderCount;
    uint16 * queueDrawIndices;
    
    void FlushQueue();
    void ClearQueue();
    
    bool BuildHeightmap();
    Heightmap *heightmap;
    String heightmapPath;
};

inline AABBox3 & LandscapeNode::GetBoundingBox()
{
    return box;
}
    
inline const LandscapeNode::eRenderingMode LandscapeNode::GetRenderingMode()
{
    return renderingMode;
}

    
};

#endif // __DAVAENGINE_LANDSCAPE_NODE_H__





