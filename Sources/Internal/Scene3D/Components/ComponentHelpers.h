#ifndef __DAVAENGINE_COMPONENT_HELPERS_H__
#define __DAVAENGINE_COMPONENT_HELPERS_H__

#include "Base/BaseTypes.h"

namespace DAVA
{

class ParticleEmitter;
class SceneNode;

ParticleEmitter * GetEmitter(SceneNode * fromEntity);

}

#endif //__DAVAENGINE_COMPONENT_HELPERS_H__
