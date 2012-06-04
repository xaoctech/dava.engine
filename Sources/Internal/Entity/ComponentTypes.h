#ifndef __DAVAENGINE_ENTITY_COMPONENT_TYPE_H__
#define __DAVAENGINE_ENTITY_COMPONENT_TYPE_H__

#include "Entity/Pool.h"

namespace DAVA 
{

class ComponentType
{
public:
    static uint64 globalBit;
    static uint64 globalIndex;
    uint64 bit;
    uint64 index;
    
    ComponentType()
    {
        Init();
    }

    void Init();
    
    uint64 GetBit() const { return bit; } ; 
    uint64 GetIndex() const { return index; };
    
    inline friend bool operator < (const ComponentType & f1, const ComponentType & f2);
};
    
inline bool operator < (const ComponentType & f1, const ComponentType & f2) { return f1.index < f2.index; };

/*
    Entity Family Should be unique ID and pointer or reference to previous Entity Family. 
    TODO: Refactor everything to make it possible to use unlimited number of EntityFamilies, and ComponentTypes.
 */
class EntityFamilyType
{
public:
    uint64 bit;
    
    EntityFamilyType(uint64 _bit = 0) { bit = _bit; };
    EntityFamilyType(EntityFamilyType oldFamily, ComponentType type) { bit = oldFamily.GetBit() | type.GetBit(); };
    
    uint64 GetBit() const { return bit; };
    bool IsEmpty() const { return bit == 0; };
    
    
    inline friend bool operator < (const EntityFamilyType & f1, const EntityFamilyType & f2);
};

inline bool operator < (const EntityFamilyType & f1, const EntityFamilyType & f2) { return f1.bit < f2.bit; };
    
};


#endif // __DAVAENGINE_ENTITY_COMPONENT_H__
