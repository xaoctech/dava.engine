/*==================================================================================
    Copyright (c) 2008, DAVA Consulting, LLC
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are met:
    * Redistributions of source code must retain the above copyright
    notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
    notice, this list of conditions and the following disclaimer in the
    documentation and/or other materials provided with the distribution.
    * Neither the name of the DAVA Consulting, LLC nor the
    names of its contributors may be used to endorse or promote products
    derived from this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE DAVA CONSULTING, LLC AND CONTRIBUTORS "AS IS" AND
    ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL DAVA CONSULTING, LLC BE LIABLE FOR ANY
    DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
    ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
    SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

    Revision History:
        * Created by Vitaliy Borodovsky 
=====================================================================================*/
#ifndef __DAVAENGINE_HASHMAP_H__
#define __DAVAENGINE_HASHMAP_H__

#include "Base/BaseTypes.h"
#include "Base/BaseObject.h"
#include "Base/TemplateHelpers.h"
#include "Base/FixedSizePoolAllocator.h"

namespace DAVA
{

template<class KeyType>
struct HashFunc
{
    uint32 operator() (KeyType & value);
};
    
template<class T>
struct HashFunc<T*>
{
    uint32 operator() (const T* value)
    {
        pointer_size ptr = (pointer_size)value;
        return (uint32)(ptr % 513);
    };
};


template<class ItemType, class KeyType, class ValueType>
struct HashMapIterator
{
    typedef typename Select<PointerTraits<ValueType>::result, ValueType, ValueType&>::Result ReturnType;
    HashMapIterator(ValueType _value)
    {
        value = _value;
    };
    
    bool operator == (const HashMapIterator<ItemType, KeyType, ValueType> & it) { return it.value == value; };
    bool operator != (const HashMapIterator<ItemType, KeyType, ValueType> & it) { return it.value != value; };
    
    inline ReturnType operator * () { return value; };
private:
    ValueType value;
};
    

/** 
    \ingroup baseobjects
    \brief Class to work with hash.
 */
template<class KeyType, class ValueType>
class HashMap
{
public:
    struct Item
    {
        KeyType key;
        ValueType value;
        Item * next;
    };

    typedef HashMapIterator<Item, KeyType, ValueType> Iterator;
    
    HashMap(uint32 expectedItemsCount = 512);
    ~HashMap();
    
    
    Iterator & End() 
    {  
        static Iterator end(0);
        return end;
    };
    
    bool HasKey(const KeyType & key) const;
    void Insert(KeyType key, const ValueType value);
    void Remove(const KeyType & key);
    Iterator Find(const KeyType & key) const;
    
    
    static const uint32 HASH_ARRAY_SIZE = 513;
    Item * hashArray[HASH_ARRAY_SIZE];
    uint32 count;
private:
    FixedSizePoolAllocator pool;
};
    
template<class KeyType, class ValueType>
HashMap<KeyType, ValueType>::HashMap(uint32 expectedItemsCount)
    : pool(sizeof(Item), expectedItemsCount)
    , count(0)
{
    for (uint32 k = 0; k < HASH_ARRAY_SIZE; ++k)
        hashArray[k] = 0;
}

template<class KeyType, class ValueType>
HashMap<KeyType, ValueType>::~HashMap()
{
    
}

template<class KeyType, class ValueType>
void HashMap<KeyType, ValueType>::Insert(KeyType key, const ValueType value)
{
    HashFunc<KeyType> func;
    uint32 itemHashIndex = func(key);
    
    Item * currentItem = hashArray[itemHashIndex];
    
    Item * newItem = (Item*)pool.New();
    newItem->key = key;
    newItem->value = value;
    newItem->next = currentItem;
    
    hashArray[itemHashIndex] = newItem;
}

template<class KeyType, class ValueType>
void HashMap<KeyType, ValueType>::Remove(const KeyType & key)
{
    HashFunc<KeyType> func;
    uint32 itemHashIndex = func(key);

    Item * currentItem = hashArray[itemHashIndex];
    Item * prevItem = 0;
    while(currentItem)
    {
        // TODO: think how to deal with char* strings, because pointers is not a good way to compare strings
        if (currentItem->key == key)    
        {
            if (prevItem == 0)
            {
                hashArray[itemHashIndex] = currentItem->next;
            }else
            {
                prevItem->next = currentItem->next; 
            }
        }
        prevItem = currentItem;
        currentItem = currentItem->next;
    }
}
    
template<class KeyType, class ValueType>
bool HashMap<KeyType, ValueType>::HasKey(const KeyType & key) const
{
    HashFunc<KeyType> func;
    uint32 itemHashIndex = func(key);
    Item * currentItem = hashArray[itemHashIndex];
    while(currentItem)
    {
        if (currentItem->key == key)return true;
        currentItem = currentItem->next;
    }
    return false;
}

template<class KeyType, class ValueType>
typename HashMap<KeyType, ValueType>::Iterator HashMap<KeyType, ValueType>::Find(const KeyType & key) const
{
    HashFunc<KeyType> func;
    uint32 itemHashIndex = func(key);
    Item * currentItem = hashArray[itemHashIndex];
    while(currentItem)
    {
        if (currentItem->key == key)return Iterator(currentItem->value);
        currentItem = currentItem->next;
    }
    // Think should I call End() function here? Is it make sense from performance reasons.
    return Iterator(0);
}


}; 


#endif // __DAVAENGINE_HASHMAP_H__

