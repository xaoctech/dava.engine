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

#include <cfloat>

#include "Render/Highlevel/Vegetation/TextureSheet.h"
#include "Render/Highlevel/Vegetation/VegetationFixedGeometryGenerator.h"
#include "Utils/Random.h"

namespace DAVA
{

/*static const Vector3 CLUSTER_TYPE_0[] =
 {
 Vector3(-0.5f, 0.0f, 1.0f),
 Vector3(0.5f, 0.0f, 1.0f),
 Vector3(0.5f, 0.0f, 0.0f),
 Vector3(-0.5f, 0.0f, 0.0f),
 
 Vector3(-0.35f, -0.35f, 1.0f),
 Vector3(0.35f, 0.35f, 1.0f),
 Vector3(0.35f, 0.35f, 0.0f),
 Vector3(-0.35f, -0.35f, 0.0f),
 
 Vector3(-0.35f, 0.35f, 1.0f),
 Vector3(0.35f, -0.35f, 1.0f),
 Vector3(0.35f, -0.35f, 0.0f),
 Vector3(-0.35f, 0.35f, 0.0f),
 };
 
 
 static const Vector3 CLUSTER_TYPE_1[] =
 {
 Vector3(-0.5f, 0.1f, 1.0f),
 Vector3(0.5f, 0.1f, 1.0f),
 Vector3(0.5f, 0.1f, 0.0f),
 Vector3(-0.5f, 0.1f, 0.0f),
 
 Vector3(-0.15f, -0.53f, 1.0f),
 Vector3(0.35f, 0.33f, 1.0f),
 Vector3(0.35f, 0.33f, 0.0f),
 Vector3(-0.15f, -0.53f, 0.0f),
 
 Vector3(-0.35f, 0.33f, 1.0f),
 Vector3(0.15f, -0.53f, 1.0f),
 Vector3(0.15f, -0.53f, 0.0f),
 Vector3(-0.35f, 0.33f, 0.0f),
 };
 
 static const Vector3 CLUSTER_TYPE_0_NORMALS[] =
 {
 Vector3(0.0f, 0.0f, -1.0f),
 Vector3(0.0f, 0.0f, 1.0f),
 Vector3(0.0f, 0.0f, 1.0f),
 Vector3(0.0f, 0.0f, -1.0f),
 
 Vector3(0.0f, 0.0f, -1.0f),
 Vector3(0.0f, 0.0f, 1.0f),
 Vector3(0.0f, 0.0f, 1.0f),
 Vector3(0.0f, 0.0f, -1.0f),
 
 Vector3(0.0f, 0.0f, -1.0f),
 Vector3(0.0f, 0.0f, 1.0f),
 Vector3(0.0f, 0.0f, 1.0f),
 Vector3(0.0f, 0.0f, -1.0f),
 };
 
 static const Vector3 CLUSTER_TYPE_1_NORMALS[] =
 {
 Vector3(0.0f, 0.0f, -1.0f),
 Vector3(0.0f, 0.0f, 1.0f),
 Vector3(0.0f, 0.0f, 1.0f),
 Vector3(0.0f, 0.0f, -1.0f),
 
 Vector3(0.0f, 0.0f, -1.0f),
 Vector3(0.0f, 0.0f, 1.0f),
 Vector3(0.0f, 0.0f, 1.0f),
 Vector3(0.0f, 0.0f, -1.0f),
 
 Vector3(0.0f, 0.0f, -1.0f),
 Vector3(0.0f, 0.0f, 1.0f),
 Vector3(0.0f, 0.0f, 1.0f),
 Vector3(0.0f, 0.0f, -1.0f),
 };
 
 
 static const int16 CLUSTER_INDICES[] =
 {
 0, 3,  1, 1, 3,  2,
 4, 7,  5, 5, 7,  6,
 8, 11, 9, 9, 11, 10
 };
 
 static const Vector3* VEGETATION_CLUSTER[] =
 {
 CLUSTER_TYPE_0,
 CLUSTER_TYPE_1
 };
 
 static const int16* VEGETATION_CLUSTER_INDICES[] =
 {
 CLUSTER_INDICES,
 CLUSTER_INDICES
 };
 
 static const Vector3* VEGETATION_CLUSTER_NORMALS[] =
 {
 CLUSTER_TYPE_0_NORMALS,
 CLUSTER_TYPE_1_NORMALS
 };
 
 static const uint32 VEGETATION_CLUSTER_SIZE[] =
 {
 COUNT_OF(CLUSTER_TYPE_0),
 COUNT_OF(CLUSTER_TYPE_1)
 };
 
 static const uint32 VEGETATION_CLUSTER_INDEX_SIZE[] =
 {
 COUNT_OF(CLUSTER_INDICES),
 COUNT_OF(CLUSTER_INDICES)
 };*/

static const Vector3 CLUSTER_TYPE_0[] =
{
    Vector3(-0.5f, 0.0f, 1.0f),
    Vector3(0.5f, 0.0f, 1.0f),
    Vector3(0.5f, 0.0f, 0.0f),
    Vector3(-0.5f, 0.0f, 0.0f),
};


static const Vector3 CLUSTER_TYPE_1[] =
{
    Vector3(-0.5f, 0.1f, 1.0f),
    Vector3(0.5f, 0.1f, 1.0f),
    Vector3(0.5f, 0.1f, 0.0f),
    Vector3(-0.5f, 0.1f, 0.0f),
};

static const Vector3 CLUSTER_TYPE_0_NORMALS[] =
{
    Vector3(0.0f, 0.0f, 1.0f),
    Vector3(0.0f, 0.0f, -1.0f),
    Vector3(0.0f, 0.0f, -1.0f),
    Vector3(0.0f, 0.0f, 1.0f),
};

static const Vector3 CLUSTER_TYPE_1_NORMALS[] =
{
    Vector3(0.0f, 0.0f, 1.0f),
    Vector3(0.0f, 0.0f, -1.0f),
    Vector3(0.0f, 0.0f, -1.0f),
    Vector3(0.0f, 0.0f, 1.0f),
};


static const int16 CLUSTER_INDICES[] =
{
    0, 3,  1, 1, 3,  2
};

static const Vector3* VEGETATION_CLUSTER[] =
{
    CLUSTER_TYPE_0,
    CLUSTER_TYPE_1
};

static const int16* VEGETATION_CLUSTER_INDICES[] =
{
    CLUSTER_INDICES,
    CLUSTER_INDICES
};

static const Vector3* VEGETATION_CLUSTER_NORMALS[] =
{
    CLUSTER_TYPE_0_NORMALS,
    CLUSTER_TYPE_1_NORMALS
};

static const uint32 VEGETATION_CLUSTER_SIZE[] =
{
    COUNT_OF(CLUSTER_TYPE_0),
    COUNT_OF(CLUSTER_TYPE_1)
};

static const uint32 VEGETATION_CLUSTER_INDEX_SIZE[] =
{
    COUNT_OF(CLUSTER_INDICES),
    COUNT_OF(CLUSTER_INDICES)
};

int32 RandomShuffleFunc(int32 limit)
{
    return (Random::Instance()->Rand() % limit);
}
    
VegetationFixedGeometryGenerator::VegetationFixedGeometryGenerator(
                                    uint32 _maxClusters,
                                    uint32 _maxDensityLevels,
                                    uint32 _maxLayerTypes,
                                    Vector2 _unitSize,
                                    const FilePath& textureSheetPath,
                                    uint32* _resolutionCellSquare,
                                    uint32 resolutionCellSquareCount,
                                    float32* _resolutionScale,
                                    uint32 resolutionScaleCount,
                                    const Vector<Vector2>& _resolutionRanges,
                                    uint32* _resolutionTilesPerRow,
                                    uint32 resolutionTilesPerRowCount)
{
    maxClusters = _maxClusters;
    maxDensityLevels = _maxDensityLevels;
    maxLayerTypes = _maxLayerTypes;
    unitSize = _unitSize;
    textureSheet.Load(textureSheetPath);
    
    for(uint32 i = 0; i < resolutionCellSquareCount; ++i)
    {
        resolutionCellSquare.push_back(_resolutionCellSquare[i]);
    }
    
    for(uint32 i = 0; i < resolutionScaleCount; ++i)
    {
        resolutionScale.push_back(_resolutionScale[i]);
    }
    
    for(size_t i = 0; i < _resolutionRanges.size(); ++i)
    {
        resolutionRanges.push_back(_resolutionRanges[i]);
    }
    
    for(uint32 i = 0; i < resolutionTilesPerRowCount; ++i)
    {
        resolutionTilesPerRow.push_back(_resolutionTilesPerRow[i]);
    }
}


void VegetationFixedGeometryGenerator::Build(VegetationRenderData& renderData)
{
    size_t resolutionCount = resolutionScale.size();
    uint32 sortDirectionCount = GetSortDirectionCount();
    Vector<int16>& indexData = renderData.GetIndices();
    Vector<VegetationVertex>& vertexData = renderData.GetVertices();
    
    uint32 tilesPerRow = (uint32)resolutionScale[resolutionCount - 1];
    uint32 maxClusterRowSize = (tilesPerRow * maxClusters);
    size_t maxTotalClusters = maxClusterRowSize * maxClusterRowSize;
    
    uint32 layerDataCount = 0;
    uint32 indexDataCount = 0;
    for(uint32 layerIndex = 0; layerIndex < maxLayerTypes; ++layerIndex)
    {
        TextureSheetCell& cellData = textureSheet.cells[layerIndex];
        layerDataCount += VEGETATION_CLUSTER_SIZE[cellData.geometryId];
        indexDataCount += VEGETATION_CLUSTER_INDEX_SIZE[cellData.geometryId];
    }
    
    uint32 totalIndexCount = 0;
    for(uint32 i = 0; i < resolutionCount; ++i)
    {
        totalIndexCount += indexDataCount * (maxTotalClusters / (uint32)resolutionScale[i]);
    }
    
    totalIndexCount *= sortDirectionCount;
    
    indexData.resize(totalIndexCount);
    vertexData.resize(maxTotalClusters * layerDataCount);
    
    Vector<uint32> layerOffsets(maxLayerTypes);
    
    GenerateVertices(maxClusters,
                     maxTotalClusters,
                     maxClusterRowSize,
                     tilesPerRow,
                     unitSize,
                     layerOffsets,
                     renderData);
    
    GenerateIndices(maxClusters, maxClusterRowSize, layerOffsets, renderData);
    
    //VI: need to build vertex & index objects AFTER initialization
    GenerateRenderDataObjects(renderData);
    
}
    
void VegetationFixedGeometryGenerator::GenerateVertices(uint32 maxClusters,
                    size_t maxTotalClusters,
                    uint32 maxClusterRowSize,
                    uint32 tilesPerRow,
                    Vector2 unitSize,
                    Vector<uint32>& layerOffsets,
                    VegetationRenderData& renderData)
{
    uint32 clustersPerTile = maxClusters * maxClusters;
    float32 atomicOffsetY = unitSize.y / clustersPerTile;
    
    Vector<uint32> shuffleDepth;
    shuffleDepth.reserve(maxTotalClusters);
    
    Vector<uint32> shuffleDensity;
    shuffleDensity.reserve(maxTotalClusters);
    
    Vector<uint32> depthScratchArray;
    depthScratchArray.reserve(clustersPerTile);
    
    Vector<uint32> densityScratchArray;
    densityScratchArray.reserve(clustersPerTile);
    
    for(uint32 i = 0; i < clustersPerTile; ++i)
    {
        depthScratchArray[i] = i;
        densityScratchArray[i] = (i % maxDensityLevels) + 1;
    }
    
    Vector<VegetationVertex>& vertexData = renderData.GetVertices();
    
    size_t vertexIndex = 0;
    uint32 totalTiles = tilesPerRow * tilesPerRow;
    for(uint32 layerIndex = 0; layerIndex < maxLayerTypes; ++layerIndex)
    {
        layerOffsets[layerIndex] = vertexIndex;
        
        TextureSheetCell& cellData = textureSheet.cells[layerIndex];
        
        const Vector3* clusterVertices = VEGETATION_CLUSTER[cellData.geometryId];
        const Vector3* clusterNormals = VEGETATION_CLUSTER_NORMALS[cellData.geometryId];
        uint32 clusterVertexCount = VEGETATION_CLUSTER_SIZE[cellData.geometryId];
        
        uint32 clusterCounter = 0;
        for(uint32 i = 0; i < totalTiles; ++i)
        {
            std::random_shuffle(densityScratchArray.begin(), densityScratchArray.end(), RandomShuffleFunc);
            std::random_shuffle(depthScratchArray.begin(), depthScratchArray.end(), RandomShuffleFunc);
            for(uint32 k = 0; k < clustersPerTile; ++k)
            {
                shuffleDensity[clusterCounter] = densityScratchArray[k];
                shuffleDepth[clusterCounter] = depthScratchArray[k];
                clusterCounter++;
            }
        }
        
        DVASSERT(clusterCounter == maxTotalClusters);
        
        AABBox3 clusterBBox;
        for(uint32 clusterVertexIndex = 0; clusterVertexIndex < clusterVertexCount; ++clusterVertexIndex)
        {
            clusterBBox.AddPoint(clusterVertices[clusterVertexIndex]);
        }
        
        float32 clusterVisualSize = cellData.geometryScale.x * (clusterBBox.max.x - clusterBBox.min.x);
        uint32 clustersInRow = unitSize.x / clusterVisualSize;
        if(clustersInRow <= 0)
        {
            clustersInRow = 1;
        }
        
        for(size_t clusterIndex = 0; clusterIndex < maxTotalClusters; ++clusterIndex)
        {
            uint32 clusterIndexX = clusterIndex % maxClusterRowSize;
            uint32 clusterIndexY = clusterIndex / maxClusterRowSize;
            
            uint32 matrixIndex = (clusterIndexX / maxClusters) + tilesPerRow * (clusterIndexY / maxClusters); //0...15
            DVASSERT(matrixIndex >= 0 && matrixIndex < (tilesPerRow * tilesPerRow));
            
            uint32 matrixIndexX = matrixIndex % tilesPerRow;
            uint32 matrixIndexY = matrixIndex / tilesPerRow;
            
            Vector2 matrixCellStart(unitSize.x * matrixIndexX, unitSize.y * matrixIndexY);
            
            float32 randomOffsetX = clusterVisualSize * (clusterIndex % clustersInRow) + clusterVisualSize * (0.5f - Random::Instance()->RandFloat());//unitSize.x * Random::Instance()->RandFloat();
            float32 randomOffsetY = atomicOffsetY * shuffleDepth[clusterIndex]; //unitSize.y * Random::Instance()->RandFloat();
            
            Vector3 clusterCenter(matrixCellStart.x + randomOffsetX, matrixCellStart.y + randomOffsetY, 0.0f);
            
            uint32 densityId = shuffleDensity[clusterIndex];
            
            for(uint32 clusterVertexIndex = 0; clusterVertexIndex < clusterVertexCount; ++clusterVertexIndex)
            {
                DVASSERT(vertexIndex < vertexData.size());
                VegetationVertex& vertex = vertexData[vertexIndex];
                
                vertex.coord.x = clusterCenter.x + clusterVertices[clusterVertexIndex].x * cellData.geometryScale.x;
                vertex.coord.y = clusterCenter.y + clusterVertices[clusterVertexIndex].y * cellData.geometryScale.x;
                vertex.coord.z = clusterCenter.z + clusterVertices[clusterVertexIndex].z * cellData.geometryScale.y;
                
                vertex.normal = clusterNormals[clusterVertexIndex];
                
                vertex.binormal = clusterCenter;
                
                vertex.tangent.x = matrixIndex * 2.0f * 4.0f; //each cluster is described by 2 vectors
                vertex.tangent.y = layerIndex;
                vertex.tangent.z = densityId;
                
                vertex.texCoord0.x = cellData.coords[clusterVertexIndex % MAX_CELL_TEXTURE_COORDS].x;
                vertex.texCoord0.y = cellData.coords[clusterVertexIndex % MAX_CELL_TEXTURE_COORDS].y;
                
                vertexIndex++;
            }
        }
    }
}
    
void VegetationFixedGeometryGenerator::GenerateIndices(uint32 maxClusters,
                    uint32 maxClusterRowSize,
                    Vector<uint32>& layerOffsets,
                    VegetationRenderData& renderData)
{
    Vector<PolygonSortData> sortingArray(1);
    Vector<int16> preparedIndices;
    size_t polygonElementCount = COUNT_OF(sortingArray[0].indices);
    
    //generate indices
    size_t totalResolutionCount = resolutionRanges.size();
    size_t currentIndexIndex = 0;
    
    Vector<Vector<Vector<SortedBufferItem> > >& indexRenderDataObject = renderData.GetIndexBuffers();
    Vector<Vector3> directionPoints;
    
    for(size_t resolutionIndex = 0; resolutionIndex < totalResolutionCount; ++resolutionIndex)
    {
        uint32 resolutionOffset = (uint32)resolutionScale[resolutionIndex];
        uint32 indexBufferCount = resolutionCellSquare[resolutionCellSquare.size() - 1] / resolutionCellSquare[resolutionIndex];
        
        indexRenderDataObject.push_back(Vector<Vector<SortedBufferItem> >());
        
        Vector<Vector<SortedBufferItem> >& currentResolutionIndexArray = indexRenderDataObject[resolutionIndex];
        
        for(uint32 i = 0; i < indexBufferCount; ++i)
        {
            AABBox3 indexBufferBBox;
            
            PrepareIndexBufferData(i,
                                   maxClusters,
                                   maxClusterRowSize,
                                   resolutionIndex,
                                   resolutionOffset,
                                   layerOffsets,
                                   preparedIndices,
                                   indexBufferBBox,
                                   renderData);
            
            directionPoints.clear();
            SetupCameraPositions(indexBufferBBox, directionPoints);
            
            PrepareSortedIndexBufferVariations(currentIndexIndex,
                                               i,
                                               polygonElementCount,
                                               indexBufferBBox,
                                               directionPoints,
                                               currentResolutionIndexArray,
                                               sortingArray,
                                               preparedIndices,
                                               renderData);
        }
    }
}
    
void VegetationFixedGeometryGenerator::PrepareIndexBufferData(uint32 indexBufferIndex,
                        uint32 maxClusters,
                        uint32 maxClusterRowSize,
                        size_t resolutionIndex,
                        uint32 resolutionOffset,
                        Vector<uint32>& layerOffsets,
                        Vector<int16>& preparedIndices,
                        AABBox3& indexBufferBBox,
                        VegetationRenderData& renderData)
{
    preparedIndices.clear();
    
    Vector<VegetationVertex>& vertexData = renderData.GetVertices();
    
    uint32 startX = (indexBufferIndex % resolutionTilesPerRow[resolutionIndex]) * maxClusters * resolutionOffset;
    uint32 startY = (indexBufferIndex / resolutionTilesPerRow[resolutionIndex]) * maxClusters * resolutionOffset;
    uint32 endX = startX + maxClusters * resolutionOffset;
    uint32 endY = startY + maxClusters * resolutionOffset;
    
    for(uint32 layerIndex = 0; layerIndex < maxLayerTypes; ++layerIndex)
    {
        TextureSheetCell& cellData = textureSheet.cells[layerIndex];
        
        const int16* clusterIndices = VEGETATION_CLUSTER_INDICES[cellData.geometryId];
        uint32 clusterIndexCount = VEGETATION_CLUSTER_INDEX_SIZE[cellData.geometryId];
        uint32 clusterVertexCount = VEGETATION_CLUSTER_SIZE[cellData.geometryId];
        
        for(uint32 y = startY; y < endY; y += resolutionOffset)
        {
            for(uint32 x = startX; x < endX; x += resolutionOffset)
            {
                uint32 baseIndex = layerOffsets[layerIndex] + (y * maxClusterRowSize + x) * clusterVertexCount;
                
                for(uint32 clusterIndexIndex = 0; clusterIndexIndex < clusterIndexCount; ++clusterIndexIndex)
                {
                    size_t vertexIndex = baseIndex + clusterIndices[clusterIndexIndex];
                    
                    DVASSERT(vertexIndex >= 0 && vertexIndex < vertexData.size());
                    
                    VegetationVertex& vertex = vertexData[vertexIndex];
                    vertex.texCoord1.x = resolutionIndex;
                    
                    indexBufferBBox.AddPoint(vertex.coord);
                    
                    preparedIndices.push_back(vertexIndex);
                }
            }
        }
    }
}
    
void VegetationFixedGeometryGenerator::PrepareSortedIndexBufferVariations(size_t& currentIndexIndex,
                                    uint32 indexBufferIndex,
                                    size_t polygonElementCount,
                                    AABBox3& indexBufferBBox,
                                    Vector<Vector3>& directionPoints,
                                    Vector<Vector<SortedBufferItem> >& currentResolutionIndexArray,
                                    Vector<PolygonSortData>& sortingArray,
                                    Vector<int16>& preparedIndices,
                                    VegetationRenderData& renderData)
{
    size_t sortItemCount = preparedIndices.size() / polygonElementCount;
    sortingArray.resize(sortItemCount);
    for(size_t sortItemIndex = 0; sortItemIndex < sortItemCount; ++sortItemIndex)
    {
        PolygonSortData& sortData = sortingArray[sortItemIndex];
        
        sortData.indices[0] = preparedIndices[sortItemIndex * polygonElementCount + 0];
        sortData.indices[1] = preparedIndices[sortItemIndex * polygonElementCount + 1];
        sortData.indices[2] = preparedIndices[sortItemIndex * polygonElementCount + 2];
    }
    
    Vector<int16>& indexData = renderData.GetIndices();
    Vector<VegetationVertex>& vertexData = renderData.GetVertices();
    
    currentResolutionIndexArray.push_back(Vector<SortedBufferItem>());
    Vector<SortedBufferItem>& currentDirectionBuffers = currentResolutionIndexArray[indexBufferIndex];
    
    uint32 sortDirectionCount = (uint32)directionPoints.size();
    for(uint32 sortDirectionIndex = 0; sortDirectionIndex < sortDirectionCount; ++sortDirectionIndex)
    {
        Vector3 cameraPosition = directionPoints[sortDirectionIndex];
        
        for(size_t sortItemIndex = 0; sortItemIndex < sortItemCount; ++sortItemIndex)
        {
            PolygonSortData& sortData = sortingArray[sortItemIndex];
            sortData.cameraDistance = FLT_MAX;
            
            for(uint32 polygonIndex = 0; polygonIndex < polygonElementCount; ++polygonIndex)
            {
                float32 distance = (vertexData[sortData.indices[polygonIndex]].coord - cameraPosition).SquareLength();
                if(distance < sortData.cameraDistance)
                {
                    sortData.cameraDistance = distance;
                }
            }
        }
        
        std::stable_sort(sortingArray.begin(), sortingArray.end(), PolygonByDistanceCompareFunction);
        
        size_t prevIndexIndex = currentIndexIndex;
        for(size_t sortItemIndex = 0; sortItemIndex < sortItemCount; ++sortItemIndex)
        {
            PolygonSortData& sortData = sortingArray[sortItemIndex];
            
            DVASSERT(currentIndexIndex < indexData.size());
            
            indexData[currentIndexIndex] = sortData.indices[0];
            currentIndexIndex++;
            
            indexData[currentIndexIndex] = sortData.indices[1];
            currentIndexIndex++;
            
            indexData[currentIndexIndex] = sortData.indices[2];
            currentIndexIndex++;
        }
        
        RenderDataObject* indexBuffer = new RenderDataObject();
        indexBuffer->SetIndices(EIF_16, (uint8*)(&indexData[prevIndexIndex]), (currentIndexIndex - prevIndexIndex));
        
        SortedBufferItem sortedBufferItem;
        sortedBufferItem.SetRenderDataObject(indexBuffer);
        sortedBufferItem.sortDirection = indexBufferBBox.GetCenter() - cameraPosition;
        sortedBufferItem.sortDirection.Normalize();
        
        currentDirectionBuffers.push_back(sortedBufferItem);
    }
}
    
void VegetationFixedGeometryGenerator::GenerateRenderDataObjects(VegetationRenderData& renderData)
{
    renderData.CreateRenderData();
    
    RenderDataObject* vertexRenderDataObject = renderData.GetRenderDataObject();
    Vector<Vector<Vector<SortedBufferItem> > >& indexRenderDataObject = renderData.GetIndexBuffers();
    Vector<VegetationVertex>& vertexData = renderData.GetVertices();
    
    vertexRenderDataObject->SetStream(EVF_VERTEX, TYPE_FLOAT, 3, sizeof(VegetationVertex), &(vertexData[0].coord));
    vertexRenderDataObject->SetStream(EVF_NORMAL, TYPE_FLOAT, 3, sizeof(VegetationVertex), &(vertexData[0].normal));
    vertexRenderDataObject->SetStream(EVF_BINORMAL, TYPE_FLOAT, 3, sizeof(VegetationVertex), &(vertexData[0].binormal));
    vertexRenderDataObject->SetStream(EVF_TANGENT, TYPE_FLOAT, 3, sizeof(VegetationVertex), &(vertexData[0].tangent));
    vertexRenderDataObject->SetStream(EVF_TEXCOORD0, TYPE_FLOAT, 2, sizeof(VegetationVertex), &(vertexData[0].texCoord0));
    vertexRenderDataObject->SetStream(EVF_TEXCOORD1, TYPE_FLOAT, 2, sizeof(VegetationVertex), &(vertexData[0].texCoord1));
    vertexRenderDataObject->BuildVertexBuffer(vertexData.size(), true);
    
    size_t totalIndexObjectArrayCount = indexRenderDataObject.size();
    for(size_t indexArrayIndex = 0; indexArrayIndex < totalIndexObjectArrayCount; ++indexArrayIndex)
    {
        Vector<Vector<SortedBufferItem> >& indexObjectArray = indexRenderDataObject[indexArrayIndex];
        size_t totalIndexObjectCount = indexObjectArray.size();
        
        for(size_t i = 0; i < totalIndexObjectCount; ++i)
        {
            Vector<SortedBufferItem>& directionArray = indexObjectArray[i];
            size_t directionCount = directionArray.size();
            for(size_t directionIndex = 0; directionIndex < directionCount; ++directionIndex)
            {
                directionArray[directionIndex].rdo->BuildIndexBuffer(true);
                directionArray[directionIndex].rdo->AttachVertices(vertexRenderDataObject);
            }
        }
    }
    
}

bool VegetationFixedGeometryGenerator::PolygonByDistanceCompareFunction(const PolygonSortData& a, const PolygonSortData&  b)
{
    return a.cameraDistance > b.cameraDistance; //back to front order
}


};