#ifndef __DAVAENGINE_INTROSPECTION_BASE_H__
#define __DAVAENGINE_INTROSPECTION_BASE_H__

#include "Base/BaseTypes.h"

namespace DAVA
{
	class IntrospectionInfo;
	class IntrospectionCollectionBase;
	class KeyedArchive;
	struct MetaInfo;


	class IntrospectionMember
	{
		friend class IntrospectionInfo;

	public:
		IntrospectionMember(const char *_name, const char *_desc, const int _offset, const MetaInfo *_type, int _flags = 0)
			: name(_name), desc(_desc), offset(_offset), type(_type), flags(_flags)	
		{ }

		const char* Name() const
		{
			return name;
		}

		const char* Desc() const
		{
			return desc;
		}

		const MetaInfo* Type() const
		{
			return type;
		}

		void* Pointer(void *object) const
		{
			return (((char *) object) + offset);
		}

		virtual VariantType Value(void *object) const
		{
			return VariantType::LoadData(Pointer(object), type);
		}

		virtual void SetValue(void *object, const VariantType &val) const
		{
			VariantType::SaveData(Pointer(object), type, val);
		}

		virtual const IntrospectionCollectionBase* Collection() const
		{
			return NULL;
		}

		const int Flags() const
		{
			return flags;
		}

	protected:
		const char* name;
		const char *desc;
		const int offset;
		const MetaInfo* type;
		const int flags;
	};

	class IntrospectionCollectionBase : public IntrospectionMember
	{
	public:
		typedef void* Iterator;

		IntrospectionCollectionBase(const char *_name, const char *_desc, const int _offset, const MetaInfo *_type, int _flags = 0)
			: IntrospectionMember(_name, _desc, _offset, _type, _flags)
		{ }

		virtual MetaInfo* CollectionType() const = 0;
		virtual MetaInfo* ValueType() const = 0;
		virtual int Size(void *object) const = 0;
		virtual Iterator Begin(void *object) const = 0;
		virtual Iterator Next(Iterator i) const = 0;
		virtual void Finish(Iterator i) const = 0;
		virtual void ItemValueGet(Iterator i, void *itemDst) const = 0;
		virtual void ItemValueSet(Iterator i, void *itemSrc) = 0;
		virtual void* ItemPointer(Iterator i) const = 0;
	};

	template<typename T> 
	class HasIntrospection
	{
		class yes {	char m; }; 
		class no { yes m[2]; };

		struct BaseTest
		{ 
			const IntrospectionInfo* GetTypeInfo();
		}; 

		struct Base : public T, public BaseTest 
		{}; 

		template <typename C, C c> struct Helper
		{}; 

		template <typename U> 
		static no Test(U*, Helper<const IntrospectionInfo* (BaseTest::*)(), &U::GetTypeInfo>* = 0); 
		static yes Test(...);

	public: 
		static const bool result = (sizeof(yes) == sizeof(Test((Base*)(0))));
		static const bool resultByObject(const T &t)
		{
			return HasIntrospection<T>::result;
		}
	};

	template<> class HasIntrospection<void> { public: static const bool result = false; };
	template<> class HasIntrospection<bool> { public: static const bool result = false; };
	template<> class HasIntrospection<char8> { public: static const bool result = false; };
	template<> class HasIntrospection<char16> { public: static const bool result = false; };
	template<> class HasIntrospection<int8> { public: static const bool result = false; };
	template<> class HasIntrospection<uint8> { public: static const bool result = false; };
	template<> class HasIntrospection<int16> { public: static const bool result = false; };
	template<> class HasIntrospection<uint16> { public: static const bool result = false; };
	template<> class HasIntrospection<int32> { public: static const bool result = false; };
	template<> class HasIntrospection<uint32> { public: static const bool result = false; };
	template<> class HasIntrospection<int64> { public: static const bool result = false; };
	template<> class HasIntrospection<uint64> { public: static const bool result = false; };
	template<> class HasIntrospection<float32> { public: static const bool result = false; };
	template<> class HasIntrospection<float64> { public: static const bool result = false; };
	template<> class HasIntrospection<KeyedArchive *> { public: static const bool result = false; };

	template<bool C, typename T = void>
	struct EnableIf
	{
		typedef T type;
	};

	template<typename T>
	struct EnableIf<false, T> 
	{ };

	template<typename T> 
	typename EnableIf<HasIntrospection<T>::result, const IntrospectionInfo*>::type GetIntrospection() 
	{
		return T::TypeInfo();
	}

	template<typename T>
	typename EnableIf<!HasIntrospection<T>::result, const IntrospectionInfo*>::type GetIntrospection() 
	{
		return NULL;
	}

	template<typename T> 
	typename EnableIf<HasIntrospection<T>::result, const IntrospectionInfo*>::type GetIntrospection(const T *t) 
	{
		const IntrospectionInfo* ret = NULL;

		if(NULL != t)
		{
			ret = t->GetTypeInfo();
		}

		return ret;
	}

	template<typename T>
	typename EnableIf<!HasIntrospection<T>::result, const IntrospectionInfo*>::type GetIntrospection(const T *t) 
	{
		return NULL;
	}
};

#endif // __DAVAENGINE_INTROSPECTION_BASE_H__
