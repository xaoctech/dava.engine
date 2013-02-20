#ifndef __DAVAENGINE_COMPONENT_HELPERS_H__
#define __DAVAENGINE_COMPONENT_HELPERS_H__

#include "Base/BaseTypes.h"

namespace DAVA
{

class ParticleEmitter;
class SceneNode;
class RenderObject;
class Light;
class LandscapeNode;
class Camera;

ParticleEmitter * GetEmitter(SceneNode * fromEntity);
RenderObject * GetRenerObject(SceneNode * fromEntity);

Light *GetLight(SceneNode * fromEntity);
LandscapeNode *GetLandscape(SceneNode * fromEntity);

Camera * GetCamera(SceneNode * fromEntity);

}

#endif //__DAVAENGINE_COMPONENT_HELPERS_H__
