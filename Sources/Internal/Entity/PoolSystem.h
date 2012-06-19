#ifndef __DAVAENGINE_POOL_SYSTEM_H__
#define __DAVAENGINE_POOL_SYSTEM_H__

#include "Base/BaseTypes.h"
#include "Base/Singleton.h"
#include "Pool.h"

namespace DAVA
{

/*class Pool;

class PoolManager
{
public:
	template<class T>
	void CreatePool(const char * name);

private:
	static Map<const char *, Pool*> pools;

	Pool * FindLastPoolForName(const char * name);
	void AddPoolForName(Pool * pool, const char * name);
};

template<class T>
void PoolSystem::CreatePool(const char * name)
{
	Pool * pool = new TemplatePool<T>(100);
	AddPoolForName(pool, name);
}*/

};

#endif //__DAVAENGINE_POOL_SYSTEM_H__
