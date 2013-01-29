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
		IntrospectionMember(const char *_name, const char *_desc, const int _offset, const MetaInfo *_type)
			: name(_name), desc(_desc), offset(_offset), type(_type)	
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
	};

	template<typename T, typename V>
	class IntrospectionProperty : public IntrospectionMember
	{
	public:
		typedef V	 (T::*GetterPtr)();
		typedef void (T::*SetterPtr)(const V &);

		IntrospectionProperty(const char *_name, const char *_desc, const int _offset, const MetaInfo *_type, GetterPtr _g, SetterPtr _s)
			: IntrospectionMember(_name, _desc, 0, _type), getter(_g), setter(_s)
		{ }

		virtual VariantType Value(void *object) const
		{
			T* realObj = (T *) object;
			V reavValue = (realObj->*getter)();
			return VariantType::LoadData(&reavValue, DAVA::MetaInfo::Instance<V>());
		}

		virtual void SetValue(void *object, const VariantType &val) const
		{
			T* realObj = (T *) object;
			V reavValue;
			VariantType::SaveData(&reavValue, DAVA::MetaInfo::Instance<V>(), val);
			(realObj->*setter)(reavValue);
		}

	protected:
		const GetterPtr getter;
		const SetterPtr setter;
	};

	template<typename TT, typename VV>
	DAVA::IntrospectionProperty<TT, VV>* CreateIntrospectionProperty(const char *_name, const char *_desc, const MetaInfo *_type, VV (TT::*_g)(), void (TT::*_s)(const VV&))
	{
		return new IntrospectionProperty<TT,VV>(_name, _desc, 0, _type, _g, _s);
	}

	class IntrospectionInfo
	{
	public:
		IntrospectionInfo(const char *_name, const int _size, const IntrospectionMember **_members, const int _members_count)
			: name(_name)
			, size(_size)
			, base_info(NULL)
			, members(_members)
			, members_count(_members_count)
		{ }

		IntrospectionInfo(const IntrospectionInfo *_base, const char *_name, const int _size, const IntrospectionMember **_members, const int _members_count)
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
	};
};

#define INTROSPECTION(_type, _members) \
	static const DAVA::IntrospectionInfo* Info() \
	{ \
		typedef _type ObjectT; \
		static const DAVA::IntrospectionMember* data[] = { _members }; \
		static DAVA::IntrospectionInfo info = DAVA::IntrospectionInfo(#_type, sizeof(_type), data, sizeof(data)/sizeof(data[0])); \
		return &info; \
	}

#define  INTROSPECTION_EXTEND(_type, _base_type, _members) \
	static const DAVA::IntrospectionInfo* Info() \
	{ \
		typedef _type ObjectT; \
		static const DAVA::IntrospectionMember* data[] = { _members }; \
		static DAVA::IntrospectionInfo info = DAVA::IntrospectionInfo(_base_type::Info(), #_type, sizeof(_type), data, sizeof(data)/sizeof(data[0])); \
		return &info; \
	}

#define MEMBER(_name, _desc) \
	new DAVA::IntrospectionMember(#_name, _desc, (int) ((long int) &((ObjectT *) 0)->_name), DAVA::MetaInfo::Instance(&ObjectT::_name)),

#define PROPERTY(_name, _desc, _getter, _setter) \
	DAVA::CreateIntrospectionProperty(#_name, _desc, DAVA::MetaInfo::Instance(&ObjectT::_name), &ObjectT::_getter, &ObjectT::_setter),

#endif // __DAVAENGINE_INTROSPECTION_H__
