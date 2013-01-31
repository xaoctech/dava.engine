#ifndef __DAVAENGINE_INTROSPECTION_COLLECTION_H__
#define __DAVAENGINE_INTROSPECTION_COLLECTION_H__

#include "Base/IntrospectionBase.h"

namespace DAVA
{
	// Класс представляет расширение базового класса IntrospectionMember и описывает члена интроспекции, как коллекцию
	// Поддерживаемые коллекци - контейнеры с одним шаблонным параметром: Vector, List, Set
	template<template <typename> class C, typename T>
	class IntrospectionCollection : public IntrospectionCollectionBase
	{
	public:
		typedef C<T> CollectionT;

		IntrospectionCollection(const char *_name, const char *_desc, const int _offset, const MetaInfo *_type, int _flags = 0)
			: IntrospectionCollectionBase(_name, _desc, _offset, _type, _flags)
		{ }

		DAVA::MetaInfo* CollectionType() const
		{
			return DAVA::MetaInfo::Instance<CollectionT >();
		}

		DAVA::MetaInfo* ValueType() const
		{
			return DAVA::MetaInfo::Instance<T>();
		}

		int Size(void *object) const
		{
			int size = 0;

			if(NULL != object)
			{
				size = ((CollectionT *) object)->size();
			}

			return size;
		}

		Iterator Begin(void *object) const
		{
			Iterator i = NULL;

			if(NULL != object)
			{
                CollectionT *collection = (CollectionT *) object;

				typename CollectionT::iterator begin = collection->begin();
				typename CollectionT::iterator end = collection->end();

				if(begin != end)
				{
					CollectionPos *pos = new CollectionPos();
					pos->curPos = begin;
					pos->endPos = end;

					i = (Iterator) pos;
				}
			}

			return i;
		}

		Iterator Next(Iterator i) const
		{
			CollectionPos* pos = (CollectionPos *) i;

			if(NULL != pos)
			{
				pos->curPos++;

				if(pos->curPos == pos->endPos)
				{
					delete pos;
					i = NULL;
				}
			}

			return i;
		}

		void Finish(Iterator i) const
		{
			CollectionPos* pos = (CollectionPos *) i;
			if(NULL != pos)
			{
				delete pos;
			}
		}

		void ItemValueGet(Iterator i, void *itemDst) const
		{
			CollectionPos* pos = (CollectionPos *) i;
			if(NULL != pos)
			{
				T *dstT = (T*) itemDst;
				*dstT = *(pos->curPos);
			}
		}

		void ItemValueSet(Iterator i, void *itemSrc)
		{
			CollectionPos* pos = (CollectionPos *) i;
			if(NULL != pos)
			{
				T *srcT = (T*) itemSrc;
				*(pos->curPos) = *srcT;
			}
		}

		void* ItemPointer(Iterator i) const 
		{
			void *p = NULL;
			CollectionPos* pos = (CollectionPos *) i;

			if(NULL != pos)
			{
				p = &(*(pos->curPos));
			}

			return p;
		}

		const IntrospectionCollectionBase* Collection() const
		{
			return this;
		}

	protected:
		struct CollectionPos
		{
			typename CollectionT::iterator curPos;
			typename CollectionT::iterator endPos;
		};
	};

	// Функция создает IntrospectionCollection, типы выводятся автоматически
	template<template <typename> class Container, class T>
	static IntrospectionCollectionBase* CreateIntrospectionCollection(Container<T> *t, const char *_name, const char *_desc, const int _offset, const MetaInfo *_type, int _flags)
	{
		return new IntrospectionCollection<Container, T>(_name, _desc, _offset, _type, _flags);
	}
};

#endif
