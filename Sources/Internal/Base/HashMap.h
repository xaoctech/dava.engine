/*
    DAVA SDK
    HashMap
    Author: Sergey Zdanevich
 */

#ifndef __DAVAENGINE_HASH_MAP__
#define __DAVAENGINE_HASH_MAP__

#include "Base/BaseTypes.h"
#include "Base/Hash.h"
#include "Debug/DVAssert.h"

namespace DAVA
{

template <typename TKey, typename TValue>
class HashMap
{
private:
	template <typename K, typename V>
	struct HashMapItem
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
	HashMapItem<TKey, TValue>* *table;
	Hash<TKey> hashFn;
	TValue defaultValue;

	inline size_t GetIndex(const TKey &key)
	{
		// fast hashFn(key) % szTable
		return hashFn(key) & (szTable - 1);
	}

	inline const HashMapItem<TKey, TValue>* GetItem(const TKey &key)
	{
		size_t index = GetIndex(key);

		HashMapItem<TKey, TValue>* i = table[index];

		while(NULL != i)
		{
			if(hashFn.Compare(i->key, key))
			{
				break;
			}

			i = i->next;
		}

		return i;
	}

	inline void InsertItem(HashMapItem<TKey, TValue>* item)
	{
		size_t index = GetIndex(item->key);
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

public:
	HashMap(size_t hashSize = 128, TValue defaultValue = TValue())
		: sz(0)
		, szTable(hashSize)
		, defaultValue(defaultValue)
	{
		table = new HashMapItem<TKey, TValue>*[szTable];
		for(size_t i = 0; i < szTable; ++i)
		{
			table[i] = NULL;
		}
	}

	~HashMap()
	{
		Clear();
		delete[] table;
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
		InsertItem(item);
	}

	void Remove(const TKey &key)
	{
		size_t index = GetIndex(key);

		HashMapItem<TKey, TValue>* item = table[index];
		HashMapItem<TKey, TValue>* prev = NULL;

		while(NULL != item)
		{
			if(hashFn.Compare(item->key, key))
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

	bool HasKey(const TKey &key)
	{
		return (NULL != GetItem(key));
	}

	TValue Value(const TKey &key)
	{
		const HashMapItem<TKey, TValue>* item = GetItem(key);
		if(NULL != item)
		{
			return item->value;
		}

		return defaultValue;
	}

	TValue operator[](const TKey &key)
	{
		return Value(key);
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

	void Resize(size_t newSize)
	{
		HashMapItem<TKey, TValue>* item;
		HashMapItem<TKey, TValue>* next = NULL;

		// not 0 and power of 2
		DVASSERT(0 != newSize && 0 == (newSize & (newSize - 1)));

		HashMapItem<TKey, TValue> **oldTable = table;
		size_t szOld = szTable;

		szTable = newSize;
		table = new HashMapItem<TKey, TValue>*[szTable];

		for(size_t i = 0; i < szTable; ++i)
		{
			table[i] = NULL;
		}

		for(size_t i = 0; i < szOld; ++i)
		{
			item = oldTable[i];
			while(NULL != item)
			{
				next = item->next;

				item->next = NULL;
				InsertItem(item);

				item = next;
			}
		}

		delete[] oldTable;
	}
};

};

#endif
