#ifndef __DAVAENGINE_ENTITY_POOL_H__
#define __DAVAENGINE_ENTITY_POOL_H__

#include "Base/BaseTypes.h"
#include "Debug/DVAssert.h"

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
    
    uint32 GetMaxCount()
    {
        return maxCount;
    }
    
    uint32 GetElementSize()
    {
        return typeSizeof;
    }
    
    void Resize(uint32 newSize)
    {
        DVASSERT(0 && "Should be called from subclass");
    }
    
    virtual Pool * CreateCopy(int32 _maxCount)
    {
        return 0;
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
    TemplatePool<T> * next;
    
	TemplatePool(int32 _maxCount)
	{
        next = 0;
		typeSizeof = sizeof(T);
		maxCount = _maxCount;
		length = 0;
		byteData = (uint8*)new T[maxCount];
	}
    
    T * GetHead()
    {
        return (T*)byteData;
    }
    
    T * GetPtr(uint32 i)
    {
        return &((T*)byteData)[i];
    }
    
    T & Get(uint32 i)
    {
        return ((T*)byteData)[i];
    }

    void Resize(uint32 newSize)
    {
        T * newArray = new T[newSize];
        memcpy(newArray, byteData, sizeof(T) * length);
        SafeDeleteArray((T*)(byteData));
        byteData = (uint8*)newArray;
        maxCount = newSize;
    }

    
    virtual Pool * CreateCopy(int32 _maxCount)
    {
        return new TemplatePool<T>(_maxCount);
    }
	
	~TemplatePool()
	{
        
		SafeDeleteArray(byteData);
	}
};

};

#endif // __DAVAENGINE_ENTITY_POOL_H__