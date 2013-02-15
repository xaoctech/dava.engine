#ifndef __DAVAENGINE_SIRIALIZABLE__H__
#define __DAVAENGINE_SIRIALIZABLE__H__

#include "Base/BaseTypes.h"

namespace DAVA
{

class KeyedArchive;
class SceneFileV2;

class Serializable
{
public:
	virtual ~Serializable() {};

	virtual void Serialize(KeyedArchive *archive, SceneFileV2 *sceneFile) = 0;
	virtual void Deserialize(KeyedArchive *archive, SceneFileV2 *sceneFile) = 0;
};

} // namespace DAVA

#endif // __DAVAENGINE_SIRIALIZABLE__H__
