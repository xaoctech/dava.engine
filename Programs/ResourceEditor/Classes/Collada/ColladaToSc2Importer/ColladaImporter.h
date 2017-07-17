#pragma once

#include "Collada/ColladaToSc2Importer/ImportLibrary.h"
#include "Collada/ColladaErrorCodes.h"

namespace DAVA
{
class Entity;
class ColladaSceneNode;
class ImportLibrary;

class ColladaImporter
{
public:
    ColladaImporter();
    eColladaErrorCodes SaveSC2(ColladaScene* colladaScene, const FilePath& scenePath);
    eColladaErrorCodes SaveAnimations(ColladaScene* colladaScene, const FilePath& dir);

private:
    struct SkinnedTriangle
    {
        Set<int32> usedJoints;
        int32 indices[3];
    };

    void ImportAnimation(ColladaSceneNode* colladaNode, Entity* nodeEntity);
    void ImportSkeleton(ColladaSceneNode* colladaNode, Entity* node);
    void LoadMaterialParents(ColladaScene* colladaScene);
    void LoadAnimations(ColladaScene* colladaScene);
    bool VerifyColladaMesh(ColladaMeshInstance* mesh, const FastName& nodeName);
    eColladaErrorCodes VerifyDavaMesh(RenderObject* mesh, const FastName name);
    eColladaErrorCodes ImportMeshes(const Vector<ColladaMeshInstance*>& meshInstances, Entity* node);
    eColladaErrorCodes BuildSceneAsCollada(Entity* root, ColladaSceneNode* colladaNode);
    RenderObject* GetMeshFromCollada(ColladaMeshInstance* mesh, const FastName& name);

    //split geometry by max available joint count per draw-call
    //returns [PolygonGroup, [jointOffset, jointCount]]
    Vector<std::pair<PolygonGroup*, Vector<int32>>> SplitSkinnedMeshGeometry(PolygonGroup* dataSource, uint32 maxJointCount);

    ImportLibrary library;
};
};