#ifndef __DAVAENGINE_COMPONENT_HELPERS_H__
#define __DAVAENGINE_COMPONENT_HELPERS_H__

#include "Base/BaseTypes.h"

namespace DAVA
{

class ParticleEmitter;
class Entity;
class RenderObject;
class Light;
class Landscape;
class Camera;
class LodComponent;

ParticleEmitter * GetEmitter(Entity * fromEntity);
RenderObject * GetRenerObject(Entity * fromEntity);

Light *GetLight(Entity * fromEntity);
Landscape *GetLandscape(Entity * fromEntity);

Camera * GetCamera(Entity * fromEntity);

LodComponent * GetLodComponent(Entity *fromEntity);
void RecursiveProcessMeshNode(Entity * curr, void * userData, void(*process)(Entity*, void *));
void RecursiveProcessLodNode(Entity * curr, int32 lod, void * userData, void(*process)(Entity*, void*));

}

#endif //__DAVAENGINE_COMPONENT_HELPERS_H__
