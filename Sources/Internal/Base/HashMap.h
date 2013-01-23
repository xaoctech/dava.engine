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
        * Created by Sergey Zdanevich 
=====================================================================================*/

#ifndef __DAVAENGINE_HASH_MAP__
#define __DAVAENGINE_HASH_MAP__

#include "Base/BaseTypes.h"
#include "Base/Hash.h"
#include "Base/TemplateHelpers.h"
#include "Debug/DVAssert.h"

namespace DAVA
{

template <typename TKey, typename TValue>
class HashMap
{
protected:
	struct HashMapItemBase
	{ };

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
			item->next = i->next;
			i->next = item;
		}
		else
		{
			table[index] = item;
		}

		sz++;
	}

public:
	template <typename K, typename V>
	struct HashMapIterator
	{
		size_t szTable;
		HashMapItem<TKey, TValue>* *table;

		size_t current_index;
		HashMapItem<K, V> *current_item;

		HashMapIterator()
			: szTable(0)
			, table(NULL)
			, current_index(-1)
			, current_item(NULL)
		{ }

		HashMapIterator(const HashMapIterator<K, V> &i)
			: szTable(i.szTable)
			, table(i.table)
			, current_index(i.current_index)
			, current_item(NULL)
		{ }

		HashMapIterator(const HashMap *map)
			: szTable(map->szTable)
			, table(map->table)
			, current_index(-1)
			, current_item(NULL)
		{
			this->operator++();
		}

		bool operator==(const HashMapIterator<K, V> &i)
		{
			return (szTable == i.szTable &&
				table == i.table &&
				current_index == i.current_index &&
				current_item == i.current_item);
		}

		bool operator!=(const HashMapIterator<K, V> &i)
		{
			return (szTable == i.szTable &&
				table == i.table &&
				current_index == i.current_index &&
				current_item == i.current_item);
		}

		HashMapIterator<K, V>& operator++()
		{
			if(NULL != current_item->next)
			{
				current_item = current_item->next;
			}
			else
			{
				while(NULL == current_item && (-1 == current_index || current_index < szTable))
				{
					current_item = table[++current_index];
				}
			}

			return *this;
		}

		HashMapIterator<K, V> operator++(int count)
		{
			HashMapIterator<K, V> tmp = *this;

			while(0 < count--)
			{
				++tmp;
			}

			return tmp;
		}

		K Key()
		{
			return current_item->key;
		}

		V Value()
		{
			return current_item->value;
		}
	};

public:
	typedef HashMapIterator<TKey, TValue> Iterator;

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
	}

	Iterator Begin()
	{
		return Iterator(this);
	}

	Iterator End()
	{
		Iterator i(this);

		i.current_item = NULL;
		i.current_index = szTable;

		return i;
	}
};

};

#endif
