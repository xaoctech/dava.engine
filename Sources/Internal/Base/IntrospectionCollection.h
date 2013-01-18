#ifndef __DAVAENGINE_INTROSPECTION_COLLECTION_H__
#define __DAVAENGINE_INTROSPECTION_COLLECTION_H__

#include "Base/IntrospectionBase.h"

namespace DAVA
{
	template<template <typename, typename> class C, typename T, typename A>
	class IntrospectionCollection : public IntrospectionMember, public IntrospectionCollectionBase
	{
	public:
		/*
		IntrospectionCollection(const char *_name, const char *_desc, const int _offset, const MetaInfo *_type, int _flags = 0)
			: IntrospectionMember(_name, _desc, _offset, _type, _flags)
		{ }
		*/

		DAVA::MetaInfo* CollectionType()
		{
			return DAVA::MetaInfo::Instance<C<T, A> >();
		}

		DAVA::MetaInfo* ValueType()
		{
			return DAVA::MetaInfo::Instance<T>();
		}

		int Size()
		{
			return collection->size();
		}

		void* Begin(void *object)
		{
			void* i = NULL;

			C<T, A>::iterator begin = collection->begin();
			C<T, A>::iterator end = collection->end();

			if(begin != end)
			{
				CollectionPos *pos = new CollectionPos();
				pos->curPos = begin;
				pos->endPos = end;

				i = (void*) pos;
			}

			return i;
		}

		void* Next(void* i)
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

		void Finish(void* i)
		{
			CollectionPos* pos = (CollectionPos *) i;
			if(NULL != pos)
			{
				delete pos;
			}
		}

		void ItemValueGet(void *object, void* i, void *itemDst)
		{
			CollectionPos* pos = (CollectionPos *) i;
			if(NULL != pos)
			{
				T *dstT = (T*) itemDst;
				*dstT = *(pos->curPos);
			}
		}

		void ItemValueSet(void *object, void* i, void *itemSrc)
		{
			CollectionPos* pos = (CollectionPos *) i;
			if(NULL != pos)
			{
				T *srcT = (T*) itemSrc;
				*(pos->curPos) = *srcT;
			}
		}

		void* ItemPointer(void *object, void *i)
		{
			void *p = NULL;
			CollectionPos* pos = (CollectionPos *) i;

			if(NULL != pos)
			{
				p = &(*(pos->curPos));
			}

			return p;
		}

	protected:
		struct CollectionPos
		{
			typename C<T, A>::iterator curPos;
			typename C<T, A>::iterator endPos;
		};

		//C<T, A> *collection;
	};

	template<template <typename, typename> class Container, class T, class A>
	static IntrospectionCollectionBase* CreateIntrospectionCollection(Container<T, A> *t, const char *_name, const char *_desc, const int _offset, const MetaInfo *_type, int _flags)
	{
		return new IntrospectionCollection<Container, T, A>(_name, _desc, _offset, _type, _flags);
	}

};

#endif
