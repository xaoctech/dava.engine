#ifdef __DAVAENGINE_BEAST__

#ifndef __BEAST_RESOURCE__
#define __BEAST_RESOURCE__

#include "DAVAEngine.h"
#include "BeastTypes.h"

class BeastManager;

template <class T>
class BeastResource : public DAVA::BaseObject
{
public:
    static T* CreateWithName(const DAVA::String& name, BeastManager* manager);
    static T* FindResourceByName(const DAVA::String& targetName);
    static DAVA::String PointerToString(void* pointer);
    static void* StringToPointer(const DAVA::String& str);
    static const DAVA::List<T*>& GetResourceList();
    static void ReleaseResources();

    const DAVA::String& GetName();
    virtual ~BeastResource();

protected:
    BeastResource(const DAVA::String& name, BeastManager* manager);
    DAVA::String resourceName;
    BeastManager* manager = nullptr;

private:
    static DAVA::List<T*> resourceList;
};

template <class T>
void BeastResource<T>::ReleaseResources()
{
    for (auto i : resourceList)
    {
        i->Release();
    }
    resourceList.clear();
}

template <class T>
const DAVA::String& BeastResource<T>::GetName()
{
    return resourceName;
}

template <class T>
const DAVA::List<T*>& BeastResource<T>::GetResourceList()
{
    return resourceList;
}

template <typename T>
DAVA::List<T*> BeastResource<T>::resourceList = DAVA::List<T*>();

template <class T>
T* BeastResource<T>::CreateWithName(const DAVA::String& name, BeastManager* manager)
{
    T* resource = FindResourceByName(name);
    if (resource == nullptr)
    {
        resource = new T(name, manager);
        resourceList.push_back(resource);
    }
    return resource;
}

template <class T>
BeastResource<T>::BeastResource(const DAVA::String& name, BeastManager* _manager)
    : resourceName(name)
    ,
    manager(_manager)
{
}

template <class T>
BeastResource<T>::~BeastResource()
{
}

template <class T>
T* BeastResource<T>::FindResourceByName(const DAVA::String& targetName)
{
    for (auto i : resourceList)
    {
        if (i->resourceName == targetName)
        {
            return i;
        }
    }
    return nullptr;
}

template <class T>
DAVA::String BeastResource<T>::PointerToString(void* pointer)
{
    DAVA::StringStream ss;
    ss << pointer;
    return ss.str();
}

template <class T>
void* BeastResource<T>::StringToPointer(const DAVA::String& str)
{
    DAVA::StringStream ss(str);
    void* pointer;
    ss >> pointer;
    return pointer;
}

#endif //__BEAST_RESOURCE__
#endif //__DAVAENGINE_BEAST__
