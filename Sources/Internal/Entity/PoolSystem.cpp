#include "PoolSystem.h"

namespace DAVA
{

//Map<const char *, Pool *> Component::pools;
//
//
//Pool * PoolSystem::FindLastPoolForName(const char * name)
//{
//	Pool * ret = 0;
//
//	Map<const char *, Pool*>::iterator find = pools.find(name);
//	if(pools.end() != find)
//	{
//		ret = find->second;
//		while(ret->next)
//		{
//			ret = ret->next;
//		}
//	}
//
//	return ret;
//}
//
//void PoolSystem::AddPoolForName(Pool * pool, const char * name)
//{
//	Pool * lastPool = FindLastPoolForName(name);
//	if(lastPool)
//	{
//		lastPool->next = pool;
//	}
//	else
//	{
//		pools[name] = pool;
//	}
//}
//
};
