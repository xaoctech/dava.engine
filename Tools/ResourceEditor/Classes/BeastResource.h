#ifdef __DAVAENGINE_BEAST__

#ifndef __BEAST_RESOURCE__
#define __BEAST_RESOURCE__

#include "DAVAEngine.h"
#include "BeastTypes.h"

template<class T>
class BeastResource : public DAVA::BaseObject
{
public:
	T * CreateFromPointer(void * pointer, ILBManagerHandle manager);

	static const T* FindResourceByName(const DAVA::String & targetName);

protected:
	BeastResource(ILBManagerHandle manager);
	ILBManagerHandle manager;
	DAVA::String resourceName;

private:
	static DAVA::List<T*> resourceList;
	DAVA::String PointerToString(void * pointer);
};

template<class T>
T * BeastResource<T>::CreateFromPointer(void * pointer, ILBManagerHandle manager)
{
	const T * resource = FindResourceByName(PointerToString(pointer));
	if(resource)
	{
		resource->Retain();
	}
	else
	{
		resource = new T(void * pointer, ILBManagerHandle manager);
	}

	return resource;
}

template<class T>
BeastResource<T>::BeastResource(ILBManagerHandle _manager)
:	manager(_manager) 
{

}

template<class T>
const T* BeastResource<T>::FindResourceByName(const DAVA::String & targetName)
{
	DAVA::List<T*>::iterator iterator;
	for(iterator = resourceList.begin(); iterator < resourceList.end(); ++iterator)
	{
		if((*iterator)->resourceName == targetName)
		{
			return *iterator;
		}
	}

	return 0;
}

template<class T>
DAVA::String BeastResource<T>::PointerToString(void * pointer)
{
	size_t pointerSize = sizeof(void*);
	if(4 == pointerSize)
	{
		return DAVA::String(DAVA::Format("%08x", pointer));
	}
	else if(8 == pointerSize)
	{
		return DAVA::String(DAVA::Format("%016x", pointer));
	}
	else
	{
		DVASSERT(0);
	}
}

#endif //__BEAST_RESOURCE__
#endif //__DAVAENGINE_BEAST__
