#ifndef __DAVAENGINE_INTROSPECTION_H__
#define __DAVAENGINE_INTROSPECTION_H__

#include "Base/Meta.h"
#include "Base/FastName.h"
#include "FileSystem/VariantType.h"

namespace DAVA
{
	class IntrospectionMember
	{
		friend class IntrospectionInfo;

	public:
		IntrospectionMember(const char *_name, const char *_desc, const int _offset, const MetaInfo *_type, int _flags = 0)
			: name(_name), desc(_desc), offset(_offset), type(_type), flags(_flags)	
		{ }

		const char* Name() const
		{
			return *name;
		}

		const char* Desc() const
		{
			return desc;
		}

		const MetaInfo* Type() const
		{
			return type;
		}

		virtual VariantType Value(void *object) const
		{
			return VariantType::LoadData(((char *) object) + offset, type);
		}

		virtual void SetValue(void *object, const VariantType &val) const
		{
			VariantType::SaveData(((char *) object) + offset, type, val);
		}

	protected:
		const FastName name;
		const char *desc;
		const int offset;
		const MetaInfo* type;
		const int flags;
	};

	template<typename T, typename V>
	class IntrospectionProperty : public IntrospectionMember
	{
	public:
		typedef V	 (T::*GetterPtr)() const;
		typedef void (T::*SetterPtr)(const V &);

		IntrospectionProperty(const char *_name, const char *_desc, const int _offset, const MetaInfo *_type, GetterPtr _g, SetterPtr _s, int _flags)
			: IntrospectionMember(_name, _desc, 0, _type, _flags), getter(_g), setter(_s)
		{ }

		virtual VariantType Value(void *object) const
		{
			T* realObj = (T *) object;
			V realValue = (realObj->*getter)();
			return VariantType::LoadData(&realValue, DAVA::MetaInfo::Instance<V>());
		}

		virtual void SetValue(void *object, const VariantType &val) const
		{
			T* realObj = (T *) object;
			V realValue;
			VariantType::SaveData(&realValue, DAVA::MetaInfo::Instance<V>(), val);
			(realObj->*setter)(realValue);
		}

	protected:
		const GetterPtr getter;
		const SetterPtr setter;
	};

	template<typename T, typename V>
	class IntrospectionPropertyReturnRef : public IntrospectionMember
	{
	public:
		typedef V&	 (T::*GetterPtr)() const;
		typedef void (T::*SetterPtr)(const V &);

		IntrospectionPropertyReturnRef(const char *_name, const char *_desc, const int _offset, const MetaInfo *_type, GetterPtr _g, SetterPtr _s, int _flags)
			: IntrospectionMember(_name, _desc, 0, _type, _flags), getter(_g), setter(_s)
		{ }

		virtual VariantType Value(void *object) const
		{
			T* realObj = (T *) object;
			V& realValue = (realObj->*getter)();
			return VariantType::LoadData(&realValue, DAVA::MetaInfo::Instance<V>());
		}

		virtual void SetValue(void *object, const VariantType &val) const
		{
			T* realObj = (T *) object;
			V realValue;
			VariantType::SaveData(&realValue, DAVA::MetaInfo::Instance<V>(), val);
			(realObj->*setter)(realValue);
		}

	protected:
		const GetterPtr getter;
		const SetterPtr setter;
	};

	class IntrospectionInfo
	{
	public:
		IntrospectionInfo(const char *_name, const int _size, const IntrospectionMember **_members, const int _members_count, const MetaInfo *_meta)
			: name(_name)
			, size(_size)
			, base_info(NULL)
			, members(_members)
			, members_count(_members_count)
			, meta(_meta)
		{
			if(NULL != meta)
			{
				meta->introspection = this;
			}
		}

		IntrospectionInfo(const IntrospectionInfo *_base, const char *_name, const int _size, const IntrospectionMember **_members, const int _members_count, const MetaInfo *_meta)
			: name(_name)
			, size(_size)
			, base_info(_base)
			, members(_members)
			, members_count(_members_count)
		{ }

		~IntrospectionInfo()
		{
			for(int i = 0; i < members_count; ++i)
				delete members[i];
		}

		const char* Name() const
		{
			return *name;
		}

		int Size() const
		{
			return size;
		}

		int MembersCount() const
		{
			return members_count;
		}

		const IntrospectionMember* Member(int index) const
		{
			const IntrospectionMember *member = NULL;

			if(index < members_count)
				member = members[index];

			return member;
		}

		const IntrospectionMember* Member(const char* name) const
		{
			const IntrospectionMember *member = NULL;

			for(int i = 0; i < members_count; ++i)
			{
				if(members[i]->name == name)
				{
					member = members[i];
					break;
				}
			}

			return member;
		}

		const MetaInfo* Meta() const
		{
			return meta;
		}

		const IntrospectionInfo* BaseInfo() const
		{
			return base_info;
		}

	protected:
		FastName name;
		const int size;

		const IntrospectionInfo *base_info;
		const IntrospectionMember **members;
		const int members_count;
		const MetaInfo* meta;
	};

	/*
	template<typename T> 
	class HasIntrospection
	{ 
		class yes { char m;}; 
		class no { yes m[2];};

		struct BaseTest
		{ 
			const DAVA::IntrospectionInfo* GetTypeInfo()
			{
				return NULL;
			} 
		}; 

		struct Base : public T, public BaseTest 
		{}; 

		template <typename C, C c> class Helper{}; 

		template <typename U> 
		static no Check(U*, Helper<const DAVA::IntrospectionInfo* (BaseTest::*)(), &U::GetTypeInfo>* = 0); 

		static yes Check(...); 

		template <typename U> 
		static const DAVA::IntrospectionInfo* GetInfo()
		{

		}

	public: 
		static const bool result = (sizeof(yes) == sizeof(Check((Base*)(0))));
		//static const DAVA::IntrospectionInfo* info = Helper<const DAVA::IntrospectionInfo* (BaseTest::*)(), &U::GetTypeInfo>()->GetTypeInfo();
	};
	*/

	template<typename TT, typename VV>
	DAVA::IntrospectionMember* CreateIntrospectionProperty(const char *_name, const char *_desc, const MetaInfo *_type, VV (TT::*_g)(), void (TT::*_s)(const VV&), int _flags)
	{
		return new IntrospectionProperty<TT,VV>(_name, _desc, 0, _type, (VV (TT::*)() const) _g, _s, _flags);
	}

	template<typename TT, typename VV>
	DAVA::IntrospectionMember* CreateIntrospectionProperty(const char *_name, const char *_desc, const MetaInfo *_type, VV (TT::*_g)() const, void (TT::*_s)(const VV&), int _flags)
	{
		return new IntrospectionProperty<TT,VV>(_name, _desc, 0, _type, _g, _s, _flags);
	}

	template<typename TT, typename VV>
	DAVA::IntrospectionMember* CreateIntrospectionProperty(const char *_name, const char *_desc, const MetaInfo *_type, const VV (TT::*_g)(), void (TT::*_s)(const VV&), int _flags)
	{
		return new IntrospectionProperty<TT,VV>(_name, _desc, 0, _type, (VV (TT::*)() const) _g, _s, _flags);
	}

	template<typename TT, typename VV>
	DAVA::IntrospectionMember* CreateIntrospectionProperty(const char *_name, const char *_desc, const MetaInfo *_type, const VV (TT::*_g)() const, void (TT::*_s)(const VV&), int _flags)
	{
		return new IntrospectionProperty<TT,VV>(_name, _desc, 0, _type, (VV (TT::*)() const) _g, _s, _flags);
	}

	template<typename TT, typename VV>
	DAVA::IntrospectionMember* CreateIntrospectionProperty(const char *_name, const char *_desc, const MetaInfo *_type, VV& (TT::*_g)(), void (TT::*_s)(const VV&), int _flags)
	{
		return new IntrospectionPropertyReturnRef<TT,VV>(_name, _desc, 0, _type, (VV& (TT::*)() const) _g, _s, _flags);
	}

	template<typename TT, typename VV>
	DAVA::IntrospectionMember* CreateIntrospectionProperty(const char *_name, const char *_desc, const MetaInfo *_type, VV& (TT::*_g)() const, void (TT::*_s)(const VV&), int _flags)
	{
		return new IntrospectionPropertyReturnRef<TT,VV>(_name, _desc, 0, _type, (VV& (TT::*)() const) _g, _s, _flags);
	}

	template<typename TT, typename VV>
	DAVA::IntrospectionMember* CreateIntrospectionProperty(const char *_name, const char *_desc, const MetaInfo *_type, const VV& (TT::*_g)(), void (TT::*_s)(const VV&), int _flags)
	{
		return new IntrospectionPropertyReturnRef<TT,VV>(_name, _desc, 0, _type, (VV& (TT::*)() const) _g, _s, _flags);
	}

	template<typename TT, typename VV>
	DAVA::IntrospectionMember* CreateIntrospectionProperty(const char *_name, const char *_desc, const MetaInfo *_type, const VV& (TT::*_g)() const, void (TT::*_s)(const VV&), int _flags)
	{
		return new IntrospectionPropertyReturnRef<TT,VV>(_name, _desc, 0, _type, (VV& (TT::*)() const) _g, _s, _flags);
	}
};

#define INTROSPECTION(_type, _members) \
	static const DAVA::IntrospectionInfo* TypeInfo() \
	{ \
		typedef _type ObjectT; \
		static const DAVA::IntrospectionMember* data[] = { _members }; \
		static DAVA::IntrospectionInfo info = DAVA::IntrospectionInfo(#_type, sizeof(_type), data, sizeof(data)/sizeof(data[0]), MetaInfo::Instance<_type>()); \
		return &info; \
	} \
	virtual const DAVA::IntrospectionInfo* GetTypeInfo() \
	{ \
		return _type::TypeInfo(); \
	}

#define  INTROSPECTION_EXTEND(_type, _base_type, _members) \
	static const DAVA::IntrospectionInfo* TypeInfo() \
	{ \
		typedef _type ObjectT; \
		static const DAVA::IntrospectionMember* data[] = { _members }; \
		static DAVA::IntrospectionInfo info = DAVA::IntrospectionInfo(_base_type::TypeInfo(), #_type, sizeof(_type), data, sizeof(data)/sizeof(data[0]), MetaInfo::Instance<_type>()); \
		return &info; \
	} \
	virtual const DAVA::IntrospectionInfo* GetTypeInfo() \
	{ \
		return _type::TypeInfo(); \
	}

#define INTROSPECTION_EMPTY(_type) \
	static const DAVA::IntrospectionInfo* TypeInfo() \
	{ \
		static DAVA::IntrospectionInfo info = DAVA::IntrospectionInfo(#_type, sizeof(_type), NULL, 0, MetaInfo::Instance<_type>()); \
		return &info; \
	} \
	virtual const DAVA::IntrospectionInfo* GetTypeInfo() \
	{ \
		return _type::TypeInfo(); \
	}

#define  INTROSPECTION_EXTEND_EMPTY(_type, _base_type) \
    static const DAVA::IntrospectionInfo* TypeInfo() \
    { \
        static DAVA::IntrospectionInfo info = DAVA::IntrospectionInfo(_base_type::TypeInfo(), #_type, sizeof(_type), NULL, 0, MetaInfo::Instance<_type>()); \
        return &info; \
    } \
    virtual const DAVA::IntrospectionInfo* GetTypeInfo() \
    { \
        return _type::TypeInfo(); \
    }


#define MEMBER(_name, _desc, _flags) \
	new DAVA::IntrospectionMember(#_name, _desc, (int) ((long int) &((ObjectT *) 0)->_name), DAVA::MetaInfo::Instance(&ObjectT::_name), _flags),

#define PROPERTY(_name, _desc, _getter, _setter, _flags) \
	DAVA::CreateIntrospectionProperty(#_name, _desc, DAVA::MetaInfo::Instance(&ObjectT::_name), &ObjectT::_getter, &ObjectT::_setter, _flags),

#endif // __DAVAENGINE_INTROSPECTION_H__
