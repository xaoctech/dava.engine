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
#include "Scene3D/LandscapeNode.h"
#include "Render/Image.h"
#include "Render/RenderManager.h"
#include "Render/RenderHelper.h"
#include "Render/RenderDataObject.h"
#include "Render/Texture.h"
#include "Scene3D/Scene.h"
#include "Render/Shader.h"
#include "Platform/SystemTimer.h"
#include "Utils/StringFormat.h"
#include "Scene3D/SceneFileV2.h"
#include "Scene3D/Heightmap.h"
#include "FileSystem/FileSystem.h"

#include "Debug/Stats.h"

namespace DAVA
{
REGISTER_CLASS(LandscapeNode);

	
LandscapeNode::LandscapeNode()
	: SceneNode()
    , indices(0)
{
    heightmapPath = "";
    
    for (int32 t = 0; t < TEXTURE_COUNT; ++t)
        textures[t] = 0;
    
    frustum = 0; //new Frustum();
    renderingMode = RENDERING_MODE_TEXTURE;
    
    tileMaskShader = NULL;
    fullTiledShader = NULL;
    
	cursor = 0;
    uniformCameraPosition = -1;
    
    for (int32 k = 0; k < TEXTURE_COUNT; ++k)
    {
        uniformTextures[k] = -1;
        uniformTextureTiling[k] = -1;
        textureTiling[k] = Vector2(1.0f, 1.0f);
    }
    uniformFogDensity = -1;
    uniformFogColor = -1;
    
    heightmap = new Heightmap();
    
    Stats::Instance()->RegisterEvent("Scene.LandscapeNode", "Everything related to LandscapeNode");
    // Stats::Instance()->RegisterEvent("Scene.LandscapeNode.Update", "Time spent in LandscapeNode Update");
    Stats::Instance()->RegisterEvent("Scene.LandscapeNode.Draw", "Time spent in LandscapeNode Draw");
    
    prevLodLayer = -1;
}

LandscapeNode::~LandscapeNode()
{
    ReleaseShaders();
    ReleaseAllRDOQuads();
    
    for (int32 t = 0; t < TEXTURE_COUNT; ++t)
    {
        SafeRelease(textures[t]);
    }
    SafeDeleteArray(indices);

    SafeRelease(heightmap);
	SafeDelete(cursor);
}
    
void LandscapeNode::InitShaders()
{
    tileMaskShader = new Shader();
    tileMaskShader->LoadFromYaml("~res:/Shaders/Landscape/tilemask.shader");
    //tileMaskShader->SetDefineList("VERTEX_FOG");
    tileMaskShader->Recompile();
    
    uniformTextures[TEXTURE_TILE0] = tileMaskShader->FindUniformLocationByName("tileTexture0");
    uniformTextures[TEXTURE_TILE1] = tileMaskShader->FindUniformLocationByName("tileTexture1");
    uniformTextures[TEXTURE_TILE2] = tileMaskShader->FindUniformLocationByName("tileTexture2");
    uniformTextures[TEXTURE_TILE3] = tileMaskShader->FindUniformLocationByName("tileTexture3");
    uniformTextures[TEXTURE_TILE_MASK] = tileMaskShader->FindUniformLocationByName("tileMask");
    uniformTextures[TEXTURE_COLOR] = tileMaskShader->FindUniformLocationByName("colorTexture");
    
    uniformCameraPosition = tileMaskShader->FindUniformLocationByName("cameraPosition");
    
    uniformTextureTiling[TEXTURE_TILE0] = tileMaskShader->FindUniformLocationByName("texture0Tiling");
    uniformTextureTiling[TEXTURE_TILE1] = tileMaskShader->FindUniformLocationByName("texture1Tiling");
    uniformTextureTiling[TEXTURE_TILE2] = tileMaskShader->FindUniformLocationByName("texture2Tiling");
    uniformTextureTiling[TEXTURE_TILE3] = tileMaskShader->FindUniformLocationByName("texture3Tiling");
    
    uniformFogColor = tileMaskShader->FindUniformLocationByName("fogColor");
    uniformFogDensity = tileMaskShader->FindUniformLocationByName("fogDensity");            
}
    
void LandscapeNode::ReleaseShaders()
{
    SafeRelease(tileMaskShader);
    SafeRelease(fullTiledShader);
    
    uniformCameraPosition = -1;
    for (int32 k = 0; k < TEXTURE_COUNT; ++k)
    {
        uniformTextures[k] = -1;
        uniformTextureTiling[k] = -1;
    }
}


int8 LandscapeNode::AllocateRDOQuad(LandscapeQuad * quad)
{
//    Logger::Debug("AllocateRDOQuad: %d %d size: %d", quad->x, quad->y, quad->size);
    DVASSERT(quad->size == RENDER_QUAD_WIDTH - 1);
    LandscapeVertex * landscapeVertices = new LandscapeVertex[(quad->size + 1) * (quad->size + 1)];
    
    int32 index = 0;
    for (int32 y = quad->y; y < quad->y + quad->size + 1; ++y)
        for (int32 x = quad->x; x < quad->x + quad->size + 1; ++x)
        {
            landscapeVertices[index].position = GetPoint(x, y, heightmap->Data()[y * heightmap->Size() + x]);
            landscapeVertices[index].texCoord = Vector2((float32)(x) / (float32)(heightmap->Size() - 1), (float32)(y) / (float32)(heightmap->Size() - 1));           

            //landscapeVertices[index].texCoord -= Vector2(0.5f, 0.5f);
            //Logger::Debug("AllocateRDOQuad: %d pos(%f, %f)", index, landscapeVertices[index].position.x, landscapeVertices[index].position.y);
            index++;
        }
    
    // setup a base RDO
    RenderDataObject * landscapeRDO = new RenderDataObject();
    landscapeRDO->SetStream(EVF_VERTEX, TYPE_FLOAT, 3, sizeof(LandscapeVertex), &landscapeVertices[0].position); 
    landscapeRDO->SetStream(EVF_TEXCOORD0, TYPE_FLOAT, 2, sizeof(LandscapeVertex), &landscapeVertices[0].texCoord); 
    landscapeRDO->BuildVertexBuffer((quad->size + 1) * (quad->size + 1));
//    SafeDeleteArray(landscapeVertices);
    
    landscapeVerticesArray.push_back(landscapeVertices);
    
    landscapeRDOArray.push_back(landscapeRDO);
    
//    Logger::Debug("Allocated vertices: %d KB", sizeof(LandscapeVertex) * (quad->size + 1) * (quad->size + 1) / 1024);
    
    return (int8)landscapeRDOArray.size() - 1;
}

void LandscapeNode::ReleaseAllRDOQuads()
{
    for (size_t k = 0; k < landscapeRDOArray.size(); ++k)
    {
        SafeRelease(landscapeRDOArray[k]);
        SafeDeleteArray(landscapeVerticesArray[k]);
    }
    landscapeRDOArray.clear();
    landscapeVerticesArray.clear();
}

void LandscapeNode::SetLods(const Vector4 & lods)
{
    lodLevelsCount = 4;
    
    lodDistance[0] = lods.x;
    lodDistance[1] = lods.y;
    lodDistance[2] = lods.z;
    lodDistance[3] = lods.w;
    
    for (int32 ll = 0; ll < lodLevelsCount; ++ll)
        lodSqDistance[ll] = lodDistance[ll] * lodDistance[ll];
}
    
void LandscapeNode::SetRenderingMode(eRenderingMode _renderingMode)
{
    renderingMode = _renderingMode;
}

void LandscapeNode::BuildLandscapeFromHeightmapImage(eRenderingMode _renderingMode, const String & heightmapPathname, const AABBox3 & _box)
{
    heightmapPath = heightmapPathname;
    
    ReleaseShaders(); // release previous shaders
    ReleaseAllRDOQuads();
    renderingMode = _renderingMode;
    InitShaders(); // init new shaders according to the selected rendering mode
    
    
//    String extension = FileSystem::Instance()->GetExtension(heightmapPath);
//    Image *image = Image::CreateFromFile(heightmapPathname);
//    if (image->GetPixelFormat() != FORMAT_A8)
//    {
//        Logger::Error("Image for landscape should be grayscale");
//        SafeRelease(image);
//        return;
//    }
//    
//    DVASSERT(image->GetWidth() == image->GetHeight());
//    heightmap->BuildFromImage(image);
//    SafeRelease(image);
    BuildHeightmap();
    
    box = _box;    

    quadTreeHead.data.x = quadTreeHead.data.y = quadTreeHead.data.lod = 0;
    //quadTreeHead.data.xbuf = quadTreeHead.data.ybuf = 0;
    quadTreeHead.data.size = heightmap->Size() - 1; 
    quadTreeHead.data.rdoQuad = -1;
    
    SetLods(Vector4(60.0f, 120.0f, 240.0f, 480.0f));
    
    allocatedMemoryForQuads = 0;
    RecursiveBuild(&quadTreeHead, 0, lodLevelsCount);
    FindNeighbours(&quadTreeHead);
    
    indices = new uint16[INDEX_ARRAY_COUNT];
    
//    Logger::Debug("Allocated indices: %d KB", RENDER_QUAD_WIDTH * RENDER_QUAD_WIDTH * 6 * 2 / 1024);
//    Logger::Debug("Allocated memory for quads: %d KB", allocatedMemoryForQuads / 1024);
//    Logger::Debug("sizeof(LandscapeQuad): %d bytes", sizeof(LandscapeQuad));
//    Logger::Debug("sizeof(QuadTreeNode): %d bytes", sizeof(QuadTreeNode<LandscapeQuad>));
}

bool LandscapeNode::BuildHeightmap()
{
    bool retValue = false;
    String extension = FileSystem::Instance()->GetExtension(heightmapPath);
    if(".png" == extension)
    {
        Image *image = Image::CreateFromFile(heightmapPath);
        if(image)
        {
            if ((image->GetPixelFormat() != FORMAT_A8) && (image->GetPixelFormat() != FORMAT_A16))
            {
                Logger::Error("Image for landscape should be grayscale");
            }
            else 
            {
                DVASSERT(image->GetWidth() == image->GetHeight());
                heightmap->BuildFromImage(image);
                retValue = true;
            }
            SafeRelease(image);
        }
    }
    else if(Heightmap::FileExtension() == extension)
    {
        retValue = heightmap->Load(heightmapPath);
    }
    else 
    {
        DVASSERT(false && "wrong extension");
    }

    return retValue;
}
    
/*
    level 0 = full landscape
    level 1 = first set of quads
    level 2 = 2
    level 3 = 3
    level 4 = 4
 */
    
//float32 LandscapeNode::BitmapHeightToReal(uint8 height)
Vector3 LandscapeNode::GetPoint(int16 x, int16 y, uint16 height)
{
    Vector3 res;
    res.x = (box.min.x + (float32)x / (float32)(heightmap->Size() - 1) * (box.max.x - box.min.x));
    res.y = (box.min.y + (float32)y / (float32)(heightmap->Size() - 1) * (box.max.y - box.min.y));
    res.z = (box.min.z + ((float32)height / (float32)Heightmap::MAX_VALUE) * (box.max.z - box.min.z));
    return res;
};

bool LandscapeNode::PlacePoint(const Vector3 & point, Vector3 & result)
{
	if (point.x > box.max.x ||
		point.x < box.min.x ||
		point.y > box.max.y ||
		point.y < box.min.y)		
	{
		return false;
	}
	float32 kW = (float32)(heightmap->Size() - 1) / (box.max.x - box.min.x);
	
	float32 x = (point.x - box.min.x) * kW;
	float32 y = (point.y - box.min.y) * kW;

	float32 x1 = floor(x);
	float32 y1 = floor(y);

	float32 x2 = ceil(x);
	float32 y2 = ceil(y);

	if (x1 == x2)
		x2 += 1.0f;

	if (y1 == y2)
		y2 += 1.0f;

	uint16 * data = heightmap->Data();
	int32 imW = heightmap->Size();

	Vector3 p1(x1, y1, 0);
	p1.z = data[(int32)p1.y * imW + (int32)p1.x];

	Vector3 p2(x2, y2, 0);
	p2.z = data[(int32)p2.y * imW + (int32)p2.x];

	Vector3 p3;
	if (x - x1 >= y - y1)
		p3 = Vector3(x2, y1, 0);
	else
		p3 = Vector3(x1, y2, 0);
	p3.z = data[(int32)p3.y * imW + (int32)p3.x];

	//http://algolist.manual.ru/maths/geom/equation/plane.php
	float32 A = p1.y * (p2.z - p3.z) + p2.y * (p3.z - p1.z) + p3.y * (p1.z - p2.z); 
	float32 B = p1.z * (p2.x - p3.x) + p2.z * (p3.x - p1.x) + p3.z * (p1.x - p2.x);
	float32 C = p1.x * (p2.y - p3.y) + p2.x * (p3.y - p1.y) + p3.x * (p1.y - p2.y);
	float32 D = p1.x * (p2.y * p3.z - p3.y * p2.z) + p2.x * (p3.y * p1.z - p1.y * p3.z) + p3.x * (p1.y * p2.z - p2.y * p1.z);

	result.x = point.x;
	result.y = point.y;

	result.z = (D - B * y - A * x) / C;
	result.z = box.min.z + result.z / ((float32)Heightmap::MAX_VALUE) * (box.max.z - box.min.z);
	return true;
};
	
	
	
void LandscapeNode::RecursiveBuild(LandQuadTreeNode<LandscapeQuad> * currentNode, int32 level, int32 maxLevels)
{
    allocatedMemoryForQuads += sizeof(LandQuadTreeNode<LandscapeQuad>);
    currentNode->data.lod = level;
    
    // if we have parrent get rdo quad 
    if (currentNode->parent)
    {
        currentNode->data.rdoQuad = currentNode->parent->data.rdoQuad;
    }
    
    if ((currentNode->data.rdoQuad == -1) && (currentNode->data.size == RENDER_QUAD_WIDTH - 1))
    {
        currentNode->data.rdoQuad = AllocateRDOQuad(&currentNode->data);
        //currentNode->data.xbuf = 0;
        //currentNode->data.ybuf = 0;
    }
    
    // 
    // Check if we can build tree with less number of nodes
    // I think we should stop much earlier to perform everything faster
    //
    
    if (currentNode->data.size == 2)
    {
        // compute node bounding box
        uint16 * data = heightmap->Data();
        for (int16 x = currentNode->data.x; x <= currentNode->data.x + currentNode->data.size; ++x)
            for (int16 y = currentNode->data.y; y <= currentNode->data.y + currentNode->data.size; ++y)
            {
                uint16 value = data[heightmap->Size() * y + x];
                Vector3 pos = GetPoint(x, y, value);
                
                currentNode->data.bbox.AddPoint(pos);
            }
        return;
    }
    
    // alloc and process childs
    currentNode->AllocChilds();
    
    int16 minIndexX = currentNode->data.x;
    int16 minIndexY = currentNode->data.y;
    
    //int16 bufMinIndexX = currentNode->data.xbuf;
    //int16 bufMinIndexY = currentNode->data.ybuf;
    
    int16 size = currentNode->data.size;
    
    // We should be able to divide landscape by 2 here
    DVASSERT((size & 1) == 0);

    LandQuadTreeNode<LandscapeQuad> * child0 = &currentNode->childs[0];
    child0->data.x = minIndexX;
    child0->data.y = minIndexY;
    //child0->data.xbuf = bufMinIndexX;
    //child0->data.ybuf = bufMinIndexY;
    child0->data.size = size / 2;
    
    LandQuadTreeNode<LandscapeQuad> * child1 = &currentNode->childs[1];
    child1->data.x = minIndexX + size / 2;
    child1->data.y = minIndexY;
    //child1->data.xbuf = bufMinIndexX + size / 2;
    //child1->data.ybuf = bufMinIndexY;
    child1->data.size = size / 2;

    LandQuadTreeNode<LandscapeQuad> * child2 = &currentNode->childs[2];
    child2->data.x = minIndexX;
    child2->data.y = minIndexY + size / 2;
    //child2->data.xbuf = bufMinIndexX;
    //child2->data.ybuf = bufMinIndexY + size / 2;
    child2->data.size = size / 2;

    LandQuadTreeNode<LandscapeQuad> * child3 = &currentNode->childs[3];
    child3->data.x = minIndexX + size / 2;
    child3->data.y = minIndexY + size / 2;
    //child3->data.xbuf = bufMinIndexX + size / 2;
    //child3->data.ybuf = bufMinIndexY + size / 2;
    child3->data.size = size / 2;
    
    for (int32 index = 0; index < 4; ++index)
    {
        LandQuadTreeNode<LandscapeQuad> * child = &currentNode->childs[index];
        child->parent = currentNode;
        RecursiveBuild(child, level + 1, maxLevels);
        
        currentNode->data.bbox.AddPoint(child->data.bbox.min);
        currentNode->data.bbox.AddPoint(child->data.bbox.max);
    }


}
/*
    Neighbours looks up
    *********
    *0*1*0*1*
    **0***1**
    *2*3*2*3*
    ****0****
    *0*1*0*1*
    **2***3**
    *2*3*2*3*
    *********
    *0*1*0*1*
    **0***1**
    *2*3*2*3*
    ****2****
    *0*1*0*1*
    **2***3**
    *2*3*2*3*
    *********
 */

LandQuadTreeNode<LandscapeNode::LandscapeQuad> * LandscapeNode::FindNodeWithXY(LandQuadTreeNode<LandscapeQuad> * currentNode, int16 quadX, int16 quadY, int16 quadSize)
{
    if ((currentNode->data.x <= quadX) && (quadX < currentNode->data.x + currentNode->data.size))
        if ((currentNode->data.y <= quadY) && (quadY < currentNode->data.y + currentNode->data.size))
    {
        if (currentNode->data.size == quadSize)return currentNode;
        if (currentNode->childs)
        {
            for (int32 index = 0; index < 4; ++index)
            {
                LandQuadTreeNode<LandscapeQuad> * child = &currentNode->childs[index];
                LandQuadTreeNode<LandscapeQuad> * result = FindNodeWithXY(child, quadX, quadY, quadSize);
                if (result)
                    return result;
            } 
        }
    } else
    {
        return 0;
    }
    
    return 0;
}
    
void LandscapeNode::FindNeighbours(LandQuadTreeNode<LandscapeQuad> * currentNode)
{
    currentNode->neighbours[LEFT] = FindNodeWithXY(&quadTreeHead, currentNode->data.x - 1, currentNode->data.y, currentNode->data.size);
    currentNode->neighbours[RIGHT] = FindNodeWithXY(&quadTreeHead, currentNode->data.x + currentNode->data.size, currentNode->data.y, currentNode->data.size);
    currentNode->neighbours[TOP] = FindNodeWithXY(&quadTreeHead, currentNode->data.x, currentNode->data.y - 1, currentNode->data.size);
    currentNode->neighbours[BOTTOM] = FindNodeWithXY(&quadTreeHead, currentNode->data.x, currentNode->data.y + currentNode->data.size, currentNode->data.size);
    
    if (currentNode->childs)
    {
        for (int32 index = 0; index < 4; ++index)
        {
            LandQuadTreeNode<LandscapeQuad> * child = &currentNode->childs[index];
            FindNeighbours(child);
        }
    }
}

void LandscapeNode::MarkFrames(LandQuadTreeNode<LandscapeQuad> * currentNode, int32 & depth)
{
    if (--depth <= 0)
    {
        currentNode->data.frame = Core::Instance()->GetGlobalFrameIndex();
        depth++;
        return;
    }
    if (currentNode->childs)
    {
        for (int32 index = 0; index < 4; ++index)
        {
            LandQuadTreeNode<LandscapeQuad> * child = &currentNode->childs[index];
            MarkFrames(child, depth);
        }
    }
    depth++;
}
    
void LandscapeNode::SetTextureTiling(eTextureLevel level, const Vector2 & tiling)
{
    textureTiling[level] = tiling;
}
    
const Vector2 & LandscapeNode::GetTextureTiling(eTextureLevel level)
{
    return textureTiling[level];
}
    
void LandscapeNode::SetTexture(eTextureLevel level, const String & textureName)
{
    Image::EnableAlphaPremultiplication(false);

    SafeRelease(textures[level]);
    
    Texture * texture = Texture::CreateFromFile(textureName); 
    if (texture)
    {
        textureNames[level] = textureName;
        texture->GenerateMipmaps();
        texture->SetWrapMode(Texture::WRAP_REPEAT, Texture::WRAP_REPEAT);
    }
    textures[level] = texture;
    
    Image::EnableAlphaPremultiplication(true);
}

void LandscapeNode::SetTexture(eTextureLevel level, Texture *texture)
{
    SafeRelease(textures[level]);
    textures[level] = SafeRetain(texture);
}

    
Texture * LandscapeNode::GetTexture(eTextureLevel level)
{
	return textures[level];
}
    
void LandscapeNode::FlushQueue()
{
    if (queueRenderCount == 0)return;
    
    RenderManager::Instance()->SetRenderData(landscapeRDOArray[queueRdoQuad]);
    RenderManager::Instance()->FlushState();
    RenderManager::Instance()->HWDrawElements(PRIMITIVETYPE_TRIANGLELIST, queueRenderCount, EIF_16, indices); 

    ClearQueue();
}
    
void LandscapeNode::ClearQueue()
{
    queueRenderCount = 0;
    queueRdoQuad = -1;
    queueDrawIndices = indices;
}

void LandscapeNode::DrawQuad(LandQuadTreeNode<LandscapeQuad> * currentNode, int8 lod)
{
//    Logger::Debug("QUAD: size = %d, x = %d, y = %d, lod = %d", currentNode->data.size, currentNode->data.x, currentNode->data.y, lod);

    int32 depth = currentNode->data.size / (1 << lod);
    if (depth == 1)
    {
        currentNode->parent->data.frame = Core::Instance()->GetGlobalFrameIndex();
    }else
    {
        //int32 newdepth = (int)(logf((float)depth) / logf(2.0f) + 0.5f);
        int32 newdepth2 = CountLeadingZeros(depth);
        //Logger::Debug("dp: %d %d %d", depth, newdepth, newdepth2);
        //DVASSERT(newdepth == newdepth2); // Check of math, we should use optimized version with depth2
        
        MarkFrames(currentNode, newdepth2);
    }
    
    int32 step = (1 << lod);
    
    
    if ((currentNode->data.rdoQuad != queueRdoQuad) && (queueRdoQuad != -1))
    {
        FlushQueue();
    }
    
    queueRdoQuad = currentNode->data.rdoQuad;
    
    //int16 width = heightmap->GetWidth();
    for (uint16 y = (currentNode->data.y & RENDER_QUAD_AND); y < (currentNode->data.y & RENDER_QUAD_AND) + currentNode->data.size; y += step)
        for (uint16 x = (currentNode->data.x & RENDER_QUAD_AND); x < (currentNode->data.x & RENDER_QUAD_AND) + currentNode->data.size; x += step)
        {
            *queueDrawIndices++ = x + y * RENDER_QUAD_WIDTH;
            *queueDrawIndices++ = (x + step) + y * RENDER_QUAD_WIDTH;
            *queueDrawIndices++ = x + (y + step) * RENDER_QUAD_WIDTH;
            
            *queueDrawIndices++ = (x + step) + y * RENDER_QUAD_WIDTH;
            *queueDrawIndices++ = (x + step) + (y + step) * RENDER_QUAD_WIDTH;
            *queueDrawIndices++ = x + (y + step) * RENDER_QUAD_WIDTH;     
 
            queueRenderCount += 6;
        }
    
    DVASSERT(queueRenderCount < INDEX_ARRAY_COUNT);
}
    
void LandscapeNode::DrawFans()
{
    uint32 currentFrame = Core::Instance()->GetGlobalFrameIndex();;
    int16 width = RENDER_QUAD_WIDTH;//heightmap->GetWidth();
    
    ClearQueue();
    
    List<LandQuadTreeNode<LandscapeQuad>*>::const_iterator end = fans.end();
    for (List<LandQuadTreeNode<LandscapeQuad>*>::iterator t = fans.begin(); t != end; ++t)
    {
        //uint16 * drawIndices = indices;
        LandQuadTreeNode<LandscapeQuad>* node = *t;
        
        //RenderManager::Instance()->SetRenderData(landscapeRDOArray[node->data.rdoQuad]);
        
        if ((node->data.rdoQuad != queueRdoQuad) && (queueRdoQuad != -1))
        {
            
            FlushQueue();
        }
        queueRdoQuad = node->data.rdoQuad;
        
        //int32 count = 0;
        int16 halfSize = (node->data.size >> 1);
        int16 xbuf = node->data.x & RENDER_QUAD_AND;
        int16 ybuf = node->data.y & RENDER_QUAD_AND;
        
        
#define ADD_VERTEX(index) queueDrawIndices[queueRenderCount++] = (index);
        
        //drawIndices[count++] = (xbuf + halfSize) + (ybuf + halfSize) * width;
        //drawIndices[count++] = (xbuf) + (ybuf) * width;

        ADD_VERTEX((xbuf + halfSize) + (ybuf + halfSize) * width);
        ADD_VERTEX((xbuf) + (ybuf) * width);
        
        if ((node->neighbours[TOP]) && (node->neighbours[TOP]->data.frame == currentFrame))
        {
            ADD_VERTEX((xbuf + halfSize) + (ybuf) * width);
            ADD_VERTEX((xbuf + halfSize) + (ybuf + halfSize) * width);
            ADD_VERTEX((xbuf + halfSize) + (ybuf) * width);
        }
        
        ADD_VERTEX((xbuf + node->data.size) + (ybuf) * width);
        ADD_VERTEX((xbuf + halfSize) + (ybuf + halfSize) * width);
        ADD_VERTEX((xbuf + node->data.size) + (ybuf) * width);

        
        if ((node->neighbours[RIGHT]) && (node->neighbours[RIGHT]->data.frame == currentFrame))
        {
            ADD_VERTEX((xbuf + node->data.size) + (ybuf + halfSize) * width);
            ADD_VERTEX((xbuf + halfSize) + (ybuf + halfSize) * width);
            ADD_VERTEX((xbuf + node->data.size) + (ybuf + halfSize) * width);
        }

        ADD_VERTEX((xbuf + node->data.size) + (ybuf + node->data.size) * width);
        ADD_VERTEX((xbuf + halfSize) + (ybuf + halfSize) * width);
        ADD_VERTEX((xbuf + node->data.size) + (ybuf + node->data.size) * width);
        
        if ((node->neighbours[BOTTOM]) && (node->neighbours[BOTTOM]->data.frame == currentFrame))
        {
            ADD_VERTEX((xbuf + halfSize) + (ybuf + node->data.size) * width);
            ADD_VERTEX((xbuf + halfSize) + (ybuf + halfSize) * width);
            ADD_VERTEX((xbuf + halfSize) + (ybuf + node->data.size) * width);
        }
        
        ADD_VERTEX((xbuf) + (ybuf + node->data.size) * width);
        ADD_VERTEX((xbuf + halfSize) + (ybuf + halfSize) * width);
        ADD_VERTEX((xbuf) + (ybuf + node->data.size) * width);
        
        if ((node->neighbours[LEFT]) && (node->neighbours[LEFT]->data.frame == currentFrame))
        {
            ADD_VERTEX((xbuf) + (ybuf + halfSize) * width);
            ADD_VERTEX((xbuf + halfSize) + (ybuf + halfSize) * width);
            ADD_VERTEX((xbuf) + (ybuf + halfSize) * width);
        }

        ADD_VERTEX((xbuf) + (ybuf) * width);
        
#undef ADD_VERTEX
        //RenderManager::Instance()->SetColor(1.0f, 0.0f, 0.0f, 1.0f);
//        RenderManager::Instance()->SetRenderData(landscapeRDOArray[node->data.rdoQuad]);
//        RenderManager::Instance()->FlushState();
//        RenderManager::Instance()->HWDrawElements(PRIMITIVETYPE_TRIANGLELIST, count, EIF_16, indices); 
    }
    
    FlushQueue();
    
/*  DRAW TRIANGLE FANS
    List<LandQuadTreeNode<LandscapeQuad>*>::const_iterator end = fans.end();
    for (List<LandQuadTreeNode<LandscapeQuad>*>::iterator t = fans.begin(); t != end; ++t)
    {
        uint16 * drawIndices = indices;
        LandQuadTreeNode<LandscapeQuad>* node = *t;
        
        RenderManager::Instance()->SetRenderData(landscapeRDOArray[node->data.rdoQuad]);

        int32 count = 0;
        int16 halfSize = (node->data.size >> 1);
        int16 xbuf = node->data.x & RENDER_QUAD_AND;
        int16 ybuf = node->data.y & RENDER_QUAD_AND;
        
        drawIndices[count++] = (xbuf + halfSize) + (ybuf + halfSize) * width;
        drawIndices[count++] = (xbuf) + (ybuf) * width;
        
        if ((node->neighbours[TOP]) && (node->neighbours[TOP]->data.frame == currentFrame))
            drawIndices[count++] = (xbuf + halfSize) + (ybuf) * width;
        
        drawIndices[count++] = (xbuf + node->data.size) + (ybuf) * width;
        
        if ((node->neighbours[RIGHT]) && (node->neighbours[RIGHT]->data.frame == currentFrame))
            drawIndices[count++] = (xbuf + node->data.size) + (ybuf + halfSize) * width;
            
        drawIndices[count++] = (xbuf + node->data.size) + (ybuf + node->data.size) * width;
        
        if ((node->neighbours[BOTTOM]) && (node->neighbours[BOTTOM]->data.frame == currentFrame))
            drawIndices[count++] = (xbuf + halfSize) + (ybuf + node->data.size) * width;

        drawIndices[count++] = (xbuf) + (ybuf + node->data.size) * width;
        
        if ((node->neighbours[LEFT]) && (node->neighbours[LEFT]->data.frame == currentFrame))
            drawIndices[count++] = (xbuf) + (ybuf + halfSize) * width;

        drawIndices[count++] = (xbuf) + (ybuf) * width;
        
        //RenderManager::Instance()->SetColor(1.0f, 0.0f, 0.0f, 1.0f);
        RenderManager::Instance()->FlushState();
        RenderManager::Instance()->HWDrawElements(PRIMITIVETYPE_TRIANGLEFAN, count, EIF_16, indices); 
    }
 */
}  
    
void LandscapeNode::Draw(LandQuadTreeNode<LandscapeQuad> * currentNode)
{
    //Frustum * frustum = scene->GetClipCamera()->GetFrustum();
    // if (!frustum->IsInside(currentNode->data.bbox))return;
    Frustum::eFrustumResult frustumRes = frustum->Classify(currentNode->data.bbox);
    if (frustumRes == Frustum::EFR_OUTSIDE)return;
    
    /*
        If current quad do not have geometry just traverse it childs. 
        Magic starts when we have a geometry
     */
    if (currentNode->data.rdoQuad == -1)
    {
        if (currentNode->childs)
        {
            for (int32 index = 0; index < 4; ++index)
            {
                LandQuadTreeNode<LandscapeQuad> * child = &currentNode->childs[index];
                Draw(child); 
            }
        }
        return;
    }
    /*
        // UNCOMMENT THIS TO CHECK GEOMETRY WITH 0 LEVEL OF DETAIL
        else
        {
            DrawQuad(currentNode, 0);
            return;
        }
     */
    
    /*
        We can be here only if we have a geometry in the node. 
        Here we use Geomipmaps rendering algorithm. 
        These quads are 129x129.
     */
//    Camera * cam = clipCamera;
    
    Vector3 corners[8];
    currentNode->data.bbox.GetCorners(corners);
    
    float32 minDist =  100000000.0f;
    float32 maxDist = -100000000.0f;
    for (int32 k = 0; k < 8; ++k)
    {
        Vector3 v = cameraPos - corners[k];
        float32 dist = v.SquareLength();
        if (dist < minDist)
            minDist = dist;
        if (dist > maxDist)
            maxDist = dist;
    };
    
    int32 minLod = 0;
    int32 maxLod = 0;
    
    for (int32 k = 0; k < lodLevelsCount; ++k)
    {
        if (minDist > lodSqDistance[k])
            minLod = k + 1;
        if (maxDist > lodSqDistance[k])
            maxLod = k + 1;
    }
    
    // debug block
#if 1
    if (currentNode == &quadTreeHead)
    {
        //Logger::Debug("== draw start ==");
    }
    //Logger::Debug("%f %f %d %d", minDist, maxDist, minLod, maxLod);
#endif
                      
//    if (frustum->IsFullyInside(currentNode->data.bbox))
//    {
//        RenderManager::Instance()->SetColor(1.0f, 0.0f, 0.0f, 1.0f);
//        RenderHelper::Instance()->DrawBox(currentNode->data.bbox);
//    }
    
    
    if ((minLod == maxLod) && (/*frustum->IsFullyInside(currentNode->data.bbox)*/(frustumRes == Frustum::EFR_INSIDE) || currentNode->data.size <= (1 << maxLod) + 1) )
    {
        //Logger::Debug("lod: %d depth: %d pos(%d, %d)", minLod, currentNode->data.lod, currentNode->data.x, currentNode->data.y);
        
//        if (currentNode->data.size <= (1 << maxLod))
//            RenderManager::Instance()->SetColor(0.0f, 1.0f, 0.0f, 1.0f);
//        if (frustum->IsFullyInside(currentNode->data.bbox))
//            RenderManager::Instance()->SetColor(1.0f, 0.0f, 0.0f, 1.0f);
        //if (frustum->IsFullyInside(currentNode->data.bbox) && (currentNode->data.size <= (1 << maxLod)))
        //    RenderManager::Instance()->SetColor(1.0f, 1.0f, 0.0f, 1.0f);

            
        //RenderManager::Instance()->SetColor(0.0f, 1.0f, 0.0f, 1.0f);
//        DrawQuad(currentNode, maxLod);
        currentNode->data.lod = maxLod;
        if(maxLod)
        {
            lodNot0quads.push_back(currentNode);
        }
        else 
        {
            lod0quads.push_back(currentNode);
        }

        
        
        //RenderManager::Instance()->SetColor(1.0f, 0.0f, 0.0f, 1.0f);
        //RenderHelper::Instance()->DrawBox(currentNode->data.bbox);

        return;
    }

    
    if ((minLod != maxLod) && (currentNode->data.size <= (1 << maxLod) + 1))
    {
//        RenderManager::Instance()->SetColor(0.0f, 0.0f, 1.0f, 1.0f);
        //DrawQuad(currentNode, minLod);
        //RenderManager::Instance()->SetColor(1.0f, 0.0f, 0.0f, 1.0f);
        //RenderHelper::Instance()->DrawBox(currentNode->data.bbox);
        fans.push_back(currentNode);
        return;
    }
    
    //
    // Check performance and identify do we need a sorting here. 
    // Probably sorting algorithm can be helpfull to render on Mac / Windows, but will be useless for iOS and Android
    //
    
    {
        if (currentNode->childs)
        {
            for (int32 index = 0; index < 4; ++index)
            {
                LandQuadTreeNode<LandscapeQuad> * child = &currentNode->childs[index];
                Draw(child); 
            }
        }
        /* EXPERIMENTAL => reduce level of quadtree, results was not successfull 
         else
        {
            DrawQuad(currentNode, maxLod);  
        }*/
    }
}

    
//void LandscapeNode::Draw(LandQuadTreeNode<LandscapeQuad> * headNode)
//{
//    Vector3 bboxSize = box.max - box.min;
//    int32 x = (bboxSize.x) ? ((float32)(cameraPos.x - box.min.x) / (float32)bboxSize.x * (float32)heightmap->Size()) : -100000;
//    int32 y = (bboxSize.y) ? ((float32)(cameraPos.y - box.min.y) / (float32)bboxSize.y * heightmap->Size()) : -100000;
////    Logger::Debug("CAMERA: x = %d, y = %d", x, y);
//
////    Vector<LandQuadTreeNode<LandscapeQuad> *>lod0quads;
////    Vector<LandQuadTreeNode<LandscapeQuad> *>lodNot0quads;
//
//    Deque<LandQuadTreeNode<LandscapeQuad> *> quadDeque;
//    
//    quadDeque.push_back(headNode);
//    while(quadDeque.size())
//    {
//        LandQuadTreeNode<LandscapeQuad> * currentNode = quadDeque.front();
//        quadDeque.pop_front();
//        
//        
//        //Frustum * frustum = scene->GetClipCamera()->GetFrustum();
//        // if (!frustum->IsInside(currentNode->data.bbox))return;
//        Frustum::eFrustumResult frustumRes = frustum->Classify(currentNode->data.bbox);
//        if (frustumRes == Frustum::EFR_OUTSIDE) continue;
//        
//        /*
//         If current quad do not have geometry just traverse it childs. 
//         Magic starts when we have a geometry
//         */
//        if (currentNode->data.rdoQuad == -1)
//        {
//            if (currentNode->childs)
//            {
////                // calc child lod levels
////                Vector2 childLodLevels[4];
////                for (int32 index = 0; index < 4; ++index)
////                {
////                    LandQuadTreeNode<LandscapeQuad> * child = &currentNode->childs[index];
////                    Vector3 v = cameraPos - child->data.bbox.GetCenter();
////                    float32 dist = v.Length();
////                    childLodLevels[index].x = dist;
////                    childLodLevels[index].y = index;
////                }
////                
////                bool again = true;
////                for (int32 i = 0; i < 4 && again; ++i)
////                {
////                    again = false;
////                    for(int32 j = 4 - 1; j > i; --j)
////                    {
////                        if(childLodLevels[j-1].x > childLodLevels[j].x)
////                        {
////                            again = true;
////                            
////                            Vector2 tmp = childLodLevels[j-1];
////                            childLodLevels[j-1] = childLodLevels[j];
////                            childLodLevels[j] = tmp;
////                        }
////                    }
////                }
//                
//                for (int32 index = 0; index < 4; ++index)
//                {
////                    LandQuadTreeNode<LandscapeQuad> * child = &currentNode->childs[(int32)childLodLevels[index].y];
//                    LandQuadTreeNode<LandscapeQuad> * child = &currentNode->childs[index];
//                    quadDeque.push_back(child);
////                    Draw(child); 
//                }
//            }
//            continue;
//        }
//        /*
//         // UNCOMMENT THIS TO CHECK GEOMETRY WITH 0 LEVEL OF DETAIL
//         else
//         {
//         DrawQuad(currentNode, 0);
//         return;
//         }
//         */
//        
//        /*
//         We can be here only if we have a geometry in the node. 
//         Here we use Geomipmaps rendering algorithm. 
//         These quads are 129x129.
//         */
//        //    Camera * cam = clipCamera;
//        
//        Vector3 corners[8];
//        currentNode->data.bbox.GetCorners(corners);
//        
//        float32 minDist =  100000000.0f;
//        float32 maxDist = -100000000.0f;
//        for (int32 k = 0; k < 8; ++k)
//        {
//            Vector3 v = cameraPos - corners[k];
//            float32 dist = v.SquareLength();
//            if (dist < minDist)
//                minDist = dist;
//            if (dist > maxDist)
//                maxDist = dist;
//        };
//        
//        int32 minLod = 0;
//        int32 maxLod = 0;
//        
//        for (int32 k = 0; k < lodLevelsCount; ++k)
//        {
//            if (minDist > lodSqDistance[k])
//                minLod = k + 1;
//            if (maxDist > lodSqDistance[k])
//                maxLod = k + 1;
//        }
//        
//        // debug block
//#if 1
//        if (currentNode == &quadTreeHead)
//        {
//            //Logger::Debug("== draw start ==");
//        }
//        //Logger::Debug("%f %f %d %d", minDist, maxDist, minLod, maxLod);
//#endif
//        
//        //    if (frustum->IsFullyInside(currentNode->data.bbox))
//        //    {
//        //        RenderManager::Instance()->SetColor(1.0f, 0.0f, 0.0f, 1.0f);
//        //        RenderHelper::Instance()->DrawBox(currentNode->data.bbox);
//        //    }
//        
//        
//        if ((minLod == maxLod) && (/*frustum->IsFullyInside(currentNode->data.bbox)*/(frustumRes == Frustum::EFR_INSIDE) || currentNode->data.size <= (1 << maxLod) + 1) )
//        {
//            //Logger::Debug("lod: %d depth: %d pos(%d, %d)", minLod, currentNode->data.lod, currentNode->data.x, currentNode->data.y);
//            
//            //        if (currentNode->data.size <= (1 << maxLod))
//            //            RenderManager::Instance()->SetColor(0.0f, 1.0f, 0.0f, 1.0f);
//            //        if (frustum->IsFullyInside(currentNode->data.bbox))
//            //            RenderManager::Instance()->SetColor(1.0f, 0.0f, 0.0f, 1.0f);
//            //if (frustum->IsFullyInside(currentNode->data.bbox) && (currentNode->data.size <= (1 << maxLod)))
//            //    RenderManager::Instance()->SetColor(1.0f, 1.0f, 0.0f, 1.0f);
//            
//            
//            //RenderManager::Instance()->SetColor(0.0f, 1.0f, 0.0f, 1.0f);
//            
////            DrawQuad(currentNode, maxLod);
//            if(maxLod)
//            {
//                lodNot0quads.push_back(currentNode);
//            }
//            else 
//            {
//                lod0quads.push_back(currentNode);
//            }
//            //RenderManager::Instance()->SetColor(1.0f, 0.0f, 0.0f, 1.0f);
//            //RenderHelper::Instance()->DrawBox(currentNode->data.bbox);
//            
//            continue;
//        }
//        
//        
//        if ((minLod != maxLod) && (currentNode->data.size <= (1 << maxLod) + 1))
//        {
//            //        RenderManager::Instance()->SetColor(0.0f, 0.0f, 1.0f, 1.0f);
//            //DrawQuad(currentNode, minLod);
//            //RenderManager::Instance()->SetColor(1.0f, 0.0f, 0.0f, 1.0f);
//            //RenderHelper::Instance()->DrawBox(currentNode->data.bbox);
//            fans.push_back(currentNode);
//            continue;
//        }
//        
//        //
//        // Check performance and identify do we need a sorting here. 
//        // Probably sorting algorithm can be helpfull to render on Mac / Windows, but will be useless for iOS and Android
//        //
//        
//        {
//            if (currentNode->childs)
//            {
////                // calc child lod levels
////                Vector2 childLodLevels[4];
////                for (int32 index = 0; index < 4; ++index)
////                {
////                    LandQuadTreeNode<LandscapeQuad> * child = &currentNode->childs[index];
////                    Vector3 v = cameraPos - child->data.bbox.GetCenter();
////                    float32 dist = v.Length();
////                    childLodLevels[index].x = dist;
////                    childLodLevels[index].y = index;
////                }
////                
////                bool again = true;
////                for (int32 i = 0; i < 4 && again; ++i)
////                {
////                    again = false;
////                    for(int32 j = 4 - 1; j > i; --j)
////                    {
////                        if(childLodLevels[j-1].x > childLodLevels[j].x)
////                        {
////                            again = true;
////                            
////                            Vector2 tmp = childLodLevels[j-1];
////                            childLodLevels[j-1] = childLodLevels[j];
////                            childLodLevels[j] = tmp;
////                        }
////                    }
////                }
////                
//                for (int32 index = 0; index < 4; ++index)
//                {
////                    LandQuadTreeNode<LandscapeQuad> * child = &currentNode->childs[(int32)childLodLevels[index].y];
//                    LandQuadTreeNode<LandscapeQuad> * child = &currentNode->childs[index];
//                    quadDeque.push_back(child);
////                    Draw(child); 
//                }
//            }
//            
//            
//            //        if (currentNode->childs)
//            //        {
//            //            for (int32 index = 0; index < 4; ++index)
//            //            {
//            //                LandQuadTreeNode<LandscapeQuad> * child = &currentNode->childs[index];
//            //                Draw(child); 
//            //            }
//            //        }
//            /* EXPERIMENTAL => reduce level of quadtree, results was not successfull 
//             else
//             {
//             DrawQuad(currentNode, maxLod);  
//             }*/
//        }
//    }
//    
//    int32 count = lod0quads.size();
//    for(int32 i = 0; i < count; ++i)
//    {
//        DrawQuad(lod0quads[i], 0);
//    }
//    
//    count = lodNot0quads.size();
//    for(int32 i = 0; i < count; ++i)
//    {
//        DrawQuad(lodNot0quads[i], 1);
//    }
//}

    
    
void LandscapeNode::BindMaterial(int32 lodLayer)
{
    if(-1 != prevLodLayer)
    {
        UnbindMaterial();
    }

    if(0 == lodLayer)
    {
        if (textures[TEXTURE_TILE0])
            RenderManager::Instance()->SetTexture(textures[TEXTURE_TILE0], 0);
        if (textures[TEXTURE_TILE1])
            RenderManager::Instance()->SetTexture(textures[TEXTURE_TILE1], 1);
        if (textures[TEXTURE_TILE2])
            RenderManager::Instance()->SetTexture(textures[TEXTURE_TILE2], 2);
        if (textures[TEXTURE_TILE3])
            RenderManager::Instance()->SetTexture(textures[TEXTURE_TILE3], 3);
        if (textures[TEXTURE_TILE_MASK])
            RenderManager::Instance()->SetTexture(textures[TEXTURE_TILE_MASK], 4);
        if (textures[TEXTURE_COLOR])
            RenderManager::Instance()->SetTexture(textures[TEXTURE_COLOR], 5);
        
        RenderManager::Instance()->SetShader(tileMaskShader);
        RenderManager::Instance()->FlushState();
        
        if (uniformTextures[TEXTURE_TILE0] != -1)
            tileMaskShader->SetUniformValue(uniformTextures[TEXTURE_TILE0], 0);
        
        if (uniformTextures[TEXTURE_TILE1] != -1)
            tileMaskShader->SetUniformValue(uniformTextures[TEXTURE_TILE1], 1);
        
        if (uniformTextures[TEXTURE_TILE2] != -1)
            tileMaskShader->SetUniformValue(uniformTextures[TEXTURE_TILE2], 2);
        
        if (uniformTextures[TEXTURE_TILE3] != -1)
            tileMaskShader->SetUniformValue(uniformTextures[TEXTURE_TILE3], 3);
        
        if (uniformTextures[TEXTURE_TILE_MASK] != -1)
            tileMaskShader->SetUniformValue(uniformTextures[TEXTURE_TILE_MASK], 4);
        
        if (uniformTextures[TEXTURE_COLOR] != -1)
            tileMaskShader->SetUniformValue(uniformTextures[TEXTURE_COLOR], 5);
        
        if (uniformCameraPosition != -1)
            tileMaskShader->SetUniformValue(uniformCameraPosition, cameraPos);    
        
        if (uniformTextureTiling[TEXTURE_TILE0] != -1)
            tileMaskShader->SetUniformValue(uniformTextureTiling[TEXTURE_TILE0], textureTiling[TEXTURE_TILE0]);
        
        if (uniformTextureTiling[TEXTURE_TILE1] != -1)
            tileMaskShader->SetUniformValue(uniformTextureTiling[TEXTURE_TILE1], textureTiling[TEXTURE_TILE1]);
        
        if (uniformTextureTiling[TEXTURE_TILE2] != -1)
            tileMaskShader->SetUniformValue(uniformTextureTiling[TEXTURE_TILE2], textureTiling[TEXTURE_TILE2]);
        
        if (uniformTextureTiling[TEXTURE_TILE3] != -1)
            tileMaskShader->SetUniformValue(uniformTextureTiling[TEXTURE_TILE3], textureTiling[TEXTURE_TILE3]);

        if (uniformFogColor != -1)
            tileMaskShader->SetUniformValue(uniformFogColor, Color((float32)0x87 / 255.0f, (float32)0xbe / 255.0f, (float32)0xd7 / 255.0f, 1.0f));
        if (uniformFogDensity != -1)
            tileMaskShader->SetUniformValue(uniformFogDensity, 0.006f);
    }
    else 
    {
        RenderManager::Instance()->SetRenderEffect(RenderManager::TEXTURE_MUL_FLAT_COLOR);
        if (textures[TEXTURE_TILE_FULL])
            RenderManager::Instance()->SetTexture(textures[TEXTURE_TILE_FULL], 0);
    }
    
    prevLodLayer = lodLayer;
}

void LandscapeNode::UnbindMaterial()
{
    if(0 == prevLodLayer)
    {
        RenderManager::Instance()->SetTexture(0, 0);
        RenderManager::Instance()->SetTexture(0, 1);
        RenderManager::Instance()->SetTexture(0, 2);
        RenderManager::Instance()->SetTexture(0, 3);
        RenderManager::Instance()->SetTexture(0, 4);
        RenderManager::Instance()->SetTexture(0, 5);
        
        RenderManager::Instance()->SetShader(NULL);
        RenderManager::Instance()->FlushState();
    }
    else if(-1 != prevLodLayer)
    {
        RenderManager::Instance()->SetTexture(0, 0);
    }

    prevLodLayer = -1;
}


void LandscapeNode::Draw()
{
    Stats::Instance()->BeginTimeMeasure("Scene.LandscapeNode.Draw", this);
    //uint64 time = SystemTimer::Instance()->AbsoluteMS();

//    Logger::Debug("*** [LandscapeNode::Draw] start");

    
    
#if defined(__DAVAENGINE_OPENGL__) && (defined(__DAVAENGINE_MACOS__) || defined(__DAVAENGINE_WIN32__))
    if (debugFlags & DEBUG_DRAW_GRID)
    {
        RenderManager::Instance()->SetColor(1.0f, 0.f, 0.f, 1.f);
        RenderManager::Instance()->SetRenderEffect(RenderManager::FLAT_COLOR);
        RenderManager::Instance()->SetShader(0);
        RenderManager::Instance()->FlushState();

        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    }
#endif //#if defined(__DAVAENGINE_OPENGL__)
    
    SceneNode::Draw();

#if defined(__DAVAENGINE_OPENGL__) && (defined(__DAVAENGINE_MACOS__) || defined(__DAVAENGINE_WIN32__))
    if (!(debugFlags & DEBUG_DRAW_GRID))
    {
        RenderManager::Instance()->ResetColor();
    }
#endif //#if defined(__DAVAENGINE_OPENGL__)

//    RenderManager::Instance()->ResetColor();

    ClearQueue();

    
    
/*
    Boroda: I do not understand why, but this code breaks frustrum culling on landscape.
    I've spent an hour, trying to understand what's going on, without luck. 
 */
    
//	Matrix4 prevMatrix = RenderManager::Instance()->GetMatrix(RenderManager::MATRIX_MODELVIEW);
//	Matrix4 meshFinalMatrix = worldTransform * prevMatrix;
//    //frustum->Set(meshFinalMatrix * RenderManager::Instance()->GetMatrix(RenderManager::MATRIX_PROJECTION));
//    cameraPos = scene->GetClipCamera()->GetPosition() * worldTransform;
//
//    RenderManager::Instance()->SetMatrix(RenderManager::MATRIX_MODELVIEW, meshFinalMatrix);
//    frustum->Set();

    frustum = scene->GetClipCamera()->GetFrustum();
    cameraPos = scene->GetClipCamera()->GetPosition();
    
    fans.clear();
    
    lod0quads.clear();
    lodNot0quads.clear();

	Draw(&quadTreeHead);
    
    BindMaterial(0);
    int32 count0 = lod0quads.size();
    for(int32 i = 0; i < count0; ++i)
    {
        DrawQuad(lod0quads[i], 0);
    }

    BindMaterial(1);
    int32 countNot0 = lodNot0quads.size();
    for(int32 i = 0; i < countNot0; ++i)
    {
        DrawQuad(lodNot0quads[i], lodNot0quads[i]->data.lod);
    }
    
    
	FlushQueue();
//	DrawFans();
    
#if defined(__DAVAENGINE_MACOS__)
    if (debugFlags & DEBUG_DRAW_ALL)
    {
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    }   
#endif 

	if(cursor)
	{
		RenderManager::Instance()->AppendState(RenderStateBlock::STATE_BLEND);
		eBlendMode src = RenderManager::Instance()->GetSrcBlend();
		eBlendMode dst = RenderManager::Instance()->GetDestBlend();
		RenderManager::Instance()->SetBlendMode(BLEND_SRC_ALPHA, BLEND_ONE_MINUS_SRC_ALPHA);
		RenderManager::Instance()->SetDepthFunc(CMP_LEQUAL);
		fans.clear();
		cursor->Prepare();
		ClearQueue();
		Draw(&quadTreeHead);
		FlushQueue();
		DrawFans();
		RenderManager::Instance()->SetDepthFunc(CMP_LESS);
		RenderManager::Instance()->RemoveState(RenderStateBlock::STATE_BLEND);
		RenderManager::Instance()->SetBlendMode(src, dst);
	}
    
    UnbindMaterial();
//    Logger::Debug("*** [LandscapeNode::Draw] finish");

//#if defined(__DAVAENGINE_MACOS__)
//    if (debugFlags & DEBUG_DRAW_ALL)
//    {
//        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
//        glEnable(GL_POLYGON_OFFSET_LINE);
//        glPolygonOffset(0.0f, 1.0f);
//        RenderManager::Instance()->SetRenderEffect(RenderManager::FLAT_COLOR);
//        fans.clear();
//        Draw(&quadTreeHead);
//        DrawFans();
//        glDisable(GL_POLYGON_OFFSET_LINE);
//        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
//    }   
//#endif
    
    //RenderManager::Instance()->SetMatrix(RenderManager::MATRIX_MODELVIEW, prevMatrix);
    //uint64 drawTime = SystemTimer::Instance()->AbsoluteMS() - time;
    //Logger::Debug("landscape draw time: %lld", drawTime);
    
    Stats::Instance()->EndTimeMeasure("Scene.LandscapeNode.Draw", this);
}


void LandscapeNode::GetGeometry(Vector<LandscapeVertex> & landscapeVertices, Vector<int32> & indices)
{
	LandQuadTreeNode<LandscapeQuad> * currentNode = &quadTreeHead;
	LandscapeQuad * quad = &currentNode->data;
	
	landscapeVertices.resize((quad->size + 1) * (quad->size + 1));

	int32 index = 0;
	for (int32 y = quad->y; y < quad->y + quad->size + 1; ++y)
	{
		for (int32 x = quad->x; x < quad->x + quad->size + 1; ++x)
		{
			landscapeVertices[index].position = GetPoint(x, y, heightmap->Data()[y * heightmap->Size() + x]);
			landscapeVertices[index].texCoord = Vector2((float32)x / (float32)(heightmap->Size() - 1), (float32)y / (float32)(heightmap->Size() - 1));           
			index++;
		}
	}

	indices.resize(heightmap->Size()*heightmap->Size()*6);
	int32 step = 1;
	int32 indexIndex = 0;
	int32 quadWidth = heightmap->Size();
	for(int32 y = 0; y < currentNode->data.size-1; y += step)
	{
		for(int32 x = 0; x < currentNode->data.size-1; x += step)
		{
			indices[indexIndex++] = x + y * quadWidth;
			indices[indexIndex++] = (x + step) + y * quadWidth;
			indices[indexIndex++] = x + (y + step) * quadWidth;

			indices[indexIndex++] = (x + step) + y * quadWidth;
			indices[indexIndex++] = (x + step) + (y + step) * quadWidth;
			indices[indexIndex++] = x + (y + step) * quadWidth;     
		}
	}
}

AABBox3 LandscapeNode::GetWTMaximumBoundingBox()
{
    AABBox3 retBBox = box;
    box.GetTransformedBox(GetWorldTransform(), retBBox);
    
    const Vector<SceneNode*>::iterator & itEnd = children.end();
    for (Vector<SceneNode*>::iterator it = children.begin(); it != itEnd; ++it)
    {
        AABBox3 lbox = (*it)->GetWTMaximumBoundingBox();
        if(  (AABBOX_INFINITY != lbox.min.x && AABBOX_INFINITY != lbox.min.y && AABBOX_INFINITY != lbox.min.z)
           &&(-AABBOX_INFINITY != lbox.max.x && -AABBOX_INFINITY != lbox.max.y && -AABBOX_INFINITY != lbox.max.z))
        {
            retBBox.AddAABBox(lbox);
        }
    }
    
    return retBBox;
}

const String & LandscapeNode::GetHeightmapPathname()
{
    return heightmapPath;
}
    
void LandscapeNode::Save(KeyedArchive * archive, SceneFileV2 * sceneFile)
{
    SceneNode::Save(archive, sceneFile);
        
    //TODO: remove code in future. Need for transition from *.png to *.heightmap
    String extension = FileSystem::Instance()->GetExtension(heightmapPath);
    if(Heightmap::FileExtension() != extension)
    {
        heightmapPath = FileSystem::Instance()->ReplaceExtension(heightmapPath, Heightmap::FileExtension());
        heightmap->Save(heightmapPath);
    }
    //
    
    archive->SetString("hmap", sceneFile->AbsoluteToRelative(heightmapPath));
    archive->SetInt32("renderingMode", renderingMode);
    archive->SetByteArrayAsType("bbox", box);
    for (int32 k = 0; k < TEXTURE_COUNT; ++k)
    {
        String path = textureNames[k];
        String relPath  = sceneFile->AbsoluteToRelative(path);
        
        if(sceneFile->DebugLogEnabled())
            Logger::Debug("landscape tex save: %s rel: %s", path.c_str(), relPath.c_str());
        
        archive->SetString(Format("tex_%d", k), relPath);
        archive->SetByteArrayAsType(Format("tiling_%d", k), textureTiling[k]);
    }
}
    
void LandscapeNode::Load(KeyedArchive * archive, SceneFileV2 * sceneFile)
{
    SceneNode::Load(archive, sceneFile);
    
    String path = archive->GetString("hmap");
    path = sceneFile->RelativeToAbsolute(path);
    AABBox3 boxDef;
    boxDef = archive->GetByteArrayAsType("bbox", boxDef);
    
    renderingMode = (eRenderingMode)archive->GetInt32("renderingMode", RENDERING_MODE_TEXTURE);
    
    BuildLandscapeFromHeightmapImage(renderingMode, path, boxDef);
        
    for (int32 k = 0; k < TEXTURE_COUNT; ++k)
    {
        String textureName = archive->GetString(Format("tex_%d", k));
        String absPath = sceneFile->RelativeToAbsolute(textureName);
        if(sceneFile->DebugLogEnabled())
            Logger::Debug("landscape tex %d load: %s abs:%s", k, textureName.c_str(), absPath.c_str());

        if(TEXTURE_TILE_FULL == k && "" == textureName)
        {
            absPath = CreateFullTiledTexture();
        }
        
        if (sceneFile->GetVersion() >= 4)
        {
            SetTexture((eTextureLevel)k, absPath);
            textureTiling[k] = archive->GetByteArrayAsType(Format("tiling_%d", k), textureTiling[k]);
        }
        else
        {
            if ((k == 0) || (k == 1)) // if texture 0 or texture 1, move them to TILE0, TILE1
                SetTexture((eTextureLevel)(k + 2), absPath);
                
            if (k == 3)
                SetTexture(TEXTURE_COLOR, absPath);

            if ((k == 0) || (k == 1))
                textureTiling[k] = archive->GetByteArrayAsType(Format("tiling_%d", k), textureTiling[k]);
        }
    }
}

const String & LandscapeNode::GetTextureName(DAVA::LandscapeNode::eTextureLevel level)
{
    return textureNames[level];
}

void LandscapeNode::CursorEnable()
{
	DVASSERT(0 == cursor);
	cursor = new LandscapeCursor();
}

void LandscapeNode::CursorDisable()
{
	SafeDelete(cursor);
}

void LandscapeNode::SetCursorTexture(Texture * texture)
{
	cursor->SetCursorTexture(texture);
}

void LandscapeNode::SetCursorPosition(const Vector2 & position)
{
	cursor->SetPosition(position);
}

void LandscapeNode::SetCursorScale(float32 scale)
{
	cursor->SetScale(scale);
}

void LandscapeNode::SetBigTextureSize(float32 bigSize)
{
	cursor->SetBigTextureSize(bigSize);
}
    
Heightmap * LandscapeNode::GetHeightmap()
{
    return heightmap;
}

String LandscapeNode::CreateFullTiledTexture()
{
    //Set indexes
    Vector<float32> ftVertexes;
    Vector<float32> ftTextureCoords;
    
    float32 x0 = 0;
    float32 y0 = 0;
    float32 x1 = TEXTURE_TILE_FULL_SIZE-1;
    float32 y1 = TEXTURE_TILE_FULL_SIZE-1;
    
    //triangle 1
    //0, 0
    ftVertexes.push_back(x0);
    ftVertexes.push_back(y0);
    ftVertexes.push_back(0);
    ftTextureCoords.push_back(0);
    ftTextureCoords.push_back(0);
    
    
    //1, 0
    ftVertexes.push_back(x1);
    ftVertexes.push_back(y0);
    ftVertexes.push_back(0);
    ftTextureCoords.push_back(1);
    ftTextureCoords.push_back(0);
    
    
    //0, 1
    ftVertexes.push_back(x0);
    ftVertexes.push_back(y1);
    ftVertexes.push_back(0);
    ftTextureCoords.push_back(0);
    ftTextureCoords.push_back(1);
    
    //1, 1
    ftVertexes.push_back(x1);
    ftVertexes.push_back(y1);
    ftVertexes.push_back(0);
    ftTextureCoords.push_back(1);
    ftTextureCoords.push_back(1);
    
    RenderDataObject *ftRenderData = new RenderDataObject();
    ftRenderData->SetStream(EVF_VERTEX, TYPE_FLOAT, 3, 0, &ftVertexes.front());
    ftRenderData->SetStream(EVF_TEXCOORD0, TYPE_FLOAT, 2, 0, &ftTextureCoords.front());


    //Draw landscape to texture
    Rect oldViewport = RenderManager::Instance()->GetViewport();
    
    Texture *fullTiled = Texture::CreateFBO(TEXTURE_TILE_FULL_SIZE, TEXTURE_TILE_FULL_SIZE, FORMAT_RGBA8888, Texture::DEPTH_NONE);
    RenderManager::Instance()->SetRenderTarget(fullTiled);

	RenderManager::Instance()->ClearWithColor(1.f, 1.f, 1.f, 1.f);
 
    RenderManager::Instance()->SetMatrix(RenderManager::MATRIX_MODELVIEW, Matrix4::IDENTITY);
    Matrix4 projection;
    projection.glOrtho(0, TEXTURE_TILE_FULL_SIZE, 0, TEXTURE_TILE_FULL_SIZE, 0, 1);
    RenderManager::Instance()->SetMatrix(RenderManager::MATRIX_PROJECTION, projection);
    RenderManager::Instance()->SetState(RenderStateBlock::DEFAULT_2D_STATE);
    
    prevLodLayer = -1;
    BindMaterial(0);
    RenderManager::Instance()->SetRenderData(ftRenderData);
    RenderManager::Instance()->FlushState();
    RenderManager::Instance()->HWDrawArrays(PRIMITIVETYPE_TRIANGLESTRIP, 0, 4);
    UnbindMaterial();

#ifdef __DAVAENGINE_OPENGL__
	BindFBO(RenderManager::Instance()->fboViewFramebuffer);
#endif
    
	RenderManager::Instance()->SetViewport(oldViewport, true);
    SafeRelease(ftRenderData);

    // Safe To File
    String colorTextureMame = GetTextureName(TEXTURE_COLOR);
    String filename, pathname;
    FileSystem::Instance()->SplitPath(colorTextureMame, pathname, filename);
    
    String pathToSave = pathname + FileSystem::Instance()->ReplaceExtension(filename, ".thumbnail.png");
    Image *image = fullTiled->CreateImageFromMemory();
    image->Save(pathToSave);
    SafeRelease(image);
    SafeRelease(fullTiled);
    
    return pathToSave;
}
    
};
