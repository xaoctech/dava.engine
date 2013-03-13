#ifndef __DAVAENGINE_COMPONENT_HELPERS_H__
#define __DAVAENGINE_COMPONENT_HELPERS_H__

#include "Base/BaseTypes.h"

namespace DAVA
{

class ParticleEmitter;
class SceneNode;
class RenderObject;
class Light;
class Landscape;
class Camera;
class LodComponent;

ParticleEmitter * GetEmitter(SceneNode * fromEntity);
RenderObject * GetRenerObject(SceneNode * fromEntity);

Light *GetLight(SceneNode * fromEntity);
Landscape *GetLandscape(SceneNode * fromEntity);

Camera * GetCamera(SceneNode * fromEntity);

LodComponent * GetLodComponent(SceneNode *fromEntity);
void RecursiveProcessMeshNode(SceneNode * curr, void * userData, void(*process)(SceneNode*, void *));
void RecursiveProcessLodNode(SceneNode * curr, int32 lod, void * userData, void(*process)(SceneNode*, void*));

}

#endif //__DAVAENGINE_COMPONENT_HELPERS_H__
