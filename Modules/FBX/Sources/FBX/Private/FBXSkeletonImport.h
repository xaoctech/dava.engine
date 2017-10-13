#pragma once

#include "FBXUtils.h"

namespace DAVA
{
class SkeletonComponent;

namespace FBXImporterDetails
{
using VertexInfluence = std::pair<uint32, float32>; //[jointIndex, jointWeight]
using FbxControlPointInfluences = Vector<VertexInfluence>;

void ProcessSceneSkeletonsRecursive(FbxNode* fbxNode);
SkeletonComponent* ImportSkeleton(FbxSkin* fbxSkin, const FbxAMatrix& meshTransform, Vector<FbxControlPointInfluences>* controlPointsInfluences, uint32* outMaxInfluenceCount);

void ClearSkeletonCache();
};
};