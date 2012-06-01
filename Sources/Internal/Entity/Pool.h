#ifndef __DAVAENGINE_ENTITY_POOL_H__
#define __DAVAENGINE_ENTITY_POOL_H__

#include "Base/BaseTypes.h"

namespace DAVA 
{

class Pool
{
public:
	Pool()
	: next(0)
	{
	}
	
	virtual ~Pool()
	{
	}
    
    uint32 GetCount()
    {
        return length;
    }

	int32 length;
	int32 maxCount;
	int32 typeSizeof;  
	uint8 * byteData;
	
	Pool * next;
};


template<class T>
class TemplatePool : public Pool
{
public:
	TemplatePool(int32 _maxCount)
	{
		typeSizeof = sizeof(T);
		maxCount = _maxCount;
		length = 0;
		byteData = (uint8*)new T[maxCount];
	}
    
    T * GetPtr(uint32 i)
    {
        return &((T*)byteData)[i];
    }
    
    T & Get(uint32 i)
    {
        return ((T*)byteData)[i];
    }
	
	~TemplatePool()
	{
		SafeDeleteArray(byteData);
	}
};

};

#endif // __DAVAENGINE_ENTITY_POOL_H__