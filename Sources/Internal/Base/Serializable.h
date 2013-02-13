#ifndef __DAVAENGINE_SIRIALIZABLE__H__
#define __DAVAENGINE_SIRIALIZABLE__H__

#include "Base/BaseTypes.h"

namespace DAVA
{

class KeyedArchive;

class Serializable
{
public:
	virtual ~Serializable() {};

	virtual void Serialize(KeyedArchive *archive) = 0;
	virtual void Deserialize(KeyedArchive *archive) = 0;
};

} // namespace DAVA

#endif // __DAVAENGINE_SIRIALIZABLE__H__
