#ifndef __DAVAENGINE_ENTITY_POOL_H__
#define __DAVAENGINE_ENTITY_POOL_H__

#include "Base/BaseTypes.h"

namespace DAVA 
{

class Pool
{
public:
	Pool()
	{
	}
	
	virtual ~Pool()
	{
	}

	int32 length;
	int32 maxCount;
	int32 typeSizeof;  
	uint8 * byteData;
	// String name; ???
};


template<class T>
class TemplatePool : public Pool
{
public:
	TemplatePool(int32 maxCount)
	{
		typeSizeof = sizeof(T);
		maxCount = _maxCount;
		length = 0;
		byteData = new T[maxCount];
	}
	
	~TemplatePool()
	{
		SafeDeleteArray(byteData)
	}
};

};

#endif // __DAVAENGINE_ENTITY_POOL_H__