#ifndef __DAVAENGINE_ENTITY_POOL_H__
#define __DAVAENGINE_ENTITY_POOL_H__

#include "Base/BaseTypes.h"
#include "Debug/DVAssert.h"
#include "Entity/DumpVariable.h"

namespace DAVA 
{

class EntityFamily;
class Pool
{
public:
	Pool()
	:	entityFamily(0)
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
    virtual void MoveElement(uint32 oldIndex, uint32 newIndex) = 0;
    virtual void MoveElement(uint32 oldIndex, Pool * newPool, uint32 newIndex) = 0;
	virtual void SetNext(Pool * next) = 0;
	virtual Pool * GetNext() = 0;
    
    virtual void Resize(uint32 newSize) = 0;
    
    virtual Pool * CreateCopy(int32 _maxCount)
    {
        return 0;
    }

	virtual void DumpElement(int32 index) = 0;

    void SetEntityFamily(EntityFamily * _entityFamily) { entityFamily = _entityFamily; }
	EntityFamily * GetEntityFamily() { return entityFamily; }

	int32 length;
	int32 maxCount;
	int32 typeSizeof;  
	uint8 * byteData;

	EntityFamily * entityFamily;
};


template<class T>
class TemplatePool : public Pool
{
	TemplatePool(int32 _maxCount)
	{
        next = 0;
		typeSizeof = sizeof(T);
		maxCount = _maxCount;
		length = 0;
		byteData = (uint8*)new T[maxCount];
	}

public:
    TemplatePool<T> * next;
    
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
    
    virtual void MoveElement(uint32 oldIndex, uint32 newIndex)
    {
        memcpy(&((T*)byteData)[newIndex], &((T*)byteData)[oldIndex],  sizeof(T));
    }
    
    virtual void MoveElement(uint32 oldIndex, Pool * oldPool, uint32 newIndex)
    {
        TemplatePool<T> * tPool = dynamic_cast<TemplatePool<T> *>(oldPool);
        
        memcpy(&((T*)byteData)[oldIndex], &((T*)tPool->byteData)[newIndex], sizeof(T));
    }

	virtual void DumpElement(int32 index)
	{
		T & element = ((T*)byteData)[index];
		DumpVariable(element);
	}

	virtual void SetNext(Pool * _next)
	{
		next = reinterpret_cast<TemplatePool<T>*>(_next);
	}

	virtual Pool * GetNext()
	{
		return next;
	}

    virtual void Resize(uint32 newSize)
    {
        T * newArray = new T[newSize];
        memcpy(newArray, byteData, sizeof(T) * length);
        SafeDeleteArray(byteData);//SafeDeleteArray((T*)(byteData));
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
    
    friend class EntityManager;
};

};

#endif // __DAVAENGINE_ENTITY_POOL_H__
