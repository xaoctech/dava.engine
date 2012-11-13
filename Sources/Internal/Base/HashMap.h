/*
    DAVA SDK
    HashMap
    Author: Sergey Zdanevich
 */

#ifndef __DAVAENGINE_HASH_MAP__
#define __DAVAENGINE_HASH_MAP__

#include "Base/BaseTypes.h"
#include "Hash.h"

namespace DAVA
{

// size = 2 ^ HASHMAP_DEFAULT_TABLE_SIZE_BITS
#define HASHMAP_DEFAULT_TABLE_SIZE_BITS 10
#define HASHMAP_DEFAULT_TABLE_SIZE (1 << HASHMAP_DEFAULT_TABLE_SIZE_BITS)

template <typename TKey, typename TValue>
class HashMapN
{
private:
	struct HashMapItemBase
	{
	};

	template <typename K, typename V>
	struct HashMapItem : public HashMapItemBase
	{
		K key;
		V value;
		HashMapItem<K, V> *next;

		HashMapItem(K k, V v)
		{
			next = NULL;
			key = k;
			value = v;
		}
	};

	size_t sz;
	size_t szTable;
	HashMapItem<TKey, TValue>* table[HASHMAP_DEFAULT_TABLE_SIZE];
	Hash<TKey> hashFn;
	TValue defaultValue;

	inline size_t getIndex(const TKey &key)
	{
		return hashFn(key) & (~(0xFFFFFFFF << HASHMAP_DEFAULT_TABLE_SIZE_BITS));
	}

	inline const HashMapItem<TKey, TValue>* getItem(const TKey &key)
	{
		size_t index = getIndex(key);

		HashMapItem<TKey, TValue>* i = table[index];

		while(NULL != i)
		{
			if(hashFn.compare(i->key, key))
			{
				break;
			}

			i = i->next;
		}

		return i;
	}

public:
	HashMapN(TValue defaultValue = TValue())
		: sz(0)
		, szTable(HASHMAP_DEFAULT_TABLE_SIZE)
		, defaultValue(defaultValue)
	{
		for(size_t i = 0; i < szTable; ++i)
		{
			table[i] = NULL;
		}
	}

	~HashMapN()
	{
		Clear();
	}

	size_t Size()
	{
		return sz;
	}

	bool Empty()
	{
		return (0 == sz);
	}

	void Insert(const TKey &key, const TValue &value)
	{
		HashMapItem<TKey, TValue>* item = new HashMapItem<TKey, TValue>(key, value);

		size_t index = getIndex(key);
		if(NULL != table[index])
		{
			HashMapItem<TKey, TValue>* i = table[index];

			while(NULL != i->next)
			{
				i = i->next;
			}

			i->next = item;
		}
		else
		{
			table[index] = item;
		}

		sz++;
	}

	void Remove(const TKey &key)
	{
		size_t index = getIndex(key);

		HashMapItem<TKey, TValue>* item = table[index];
		HashMapItem<TKey, TValue>* prev = NULL;

		while(NULL != item)
		{
			if(hashFn.compare(item->key, key))
			{
				break;
			}

			prev = item;
			item = item->next;
		}
		
		if(NULL != item)
		{
			if(NULL != prev)
			{
				prev->next = item->next;
			}
			else
			{
				table[index] = NULL;
			}

			sz--;
			delete item;
		}
	}

	void Clear()
	{
		const HashMapItem<TKey, TValue>* item;
		const HashMapItem<TKey, TValue>* next = NULL;

		for(size_t i = 0; i < szTable; ++i)
		{
			item = table[i];
			table[i] = NULL;
			while(NULL != item)
			{
				next = item->next;
				delete item;
				item = next;
			}
		}
	}

	bool HasKey(const TKey &key)
	{
		return (NULL != getItem(key));
	}

	TValue Value(const TKey &key)
	{
		const HashMapItem<TKey, TValue>* item = getItem(key);
		if(NULL != item)
		{
			return item->value;
		}

		return defaultValue;
	}

	TValue operator[](const TKey &key)
	{
		return value(key);
	}
};

};

#endif
