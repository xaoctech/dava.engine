#ifndef __DAVAENGINE_INTROSPECTION_H__
#define __DAVAENGINE_INTROSPECTION_H__

#include "Base/Meta.h"
#include "FileSystem/VariantType.h"

namespace DAVA
{
	struct IntrospectionMember
	{
		IntrospectionMember(const char *_name, const char *_desc, const int _offset, const MetaInfo *_type)
			: name(_name), desc(_desc), offset(_offset), type(_type) 
		{ }

		const char *name;
		const char *desc;
		const int offset;
		const MetaInfo* type;

		VariantType GetValue(const void *object) const
		{
			return VariantType::LoadData(((char *) object) + offset, type);
		}

		void SetValue(const void *object, const VariantType &val) const
		{
			VariantType::SaveData(((char *) object) + offset, type, val);
		}
	};

	struct IntrospectionInfo
	{
		IntrospectionInfo(const char *_name, const int _size, const IntrospectionMember *_members, const int _members_count)
			: name(_name)
			, size(_size)
			, base_info(NULL)
			, members(_members)
			, members_count(_members_count)
		{ }

		IntrospectionInfo(const IntrospectionInfo *_base, const char *_name, const int _size, const IntrospectionMember *_members, const int _members_count)
			: name(_name)
			, size(_size)
			, base_info(_base)
			, members(_members)
			, members_count(_members_count)
		{ }

		const char *name;
		const int size;

		int MembersCount() const
		{
			return members_count;
		}

		const IntrospectionMember* Member(int index) const
		{
			const IntrospectionMember *member = NULL;

			if(index < members_count)
				member = &members[index];

			return member;
		}

		const IntrospectionInfo* BaseInfo() const
		{
			return base_info;
		}

	private:
		const IntrospectionInfo *base_info;
		const IntrospectionMember *members;
		const int members_count;
	};
};

#define INTROSPECTION(_type, _members) \
	static const DAVA::IntrospectionInfo* Info() \
	{ \
		typedef _type ObjectT; \
		static DAVA::IntrospectionMember data[] = { _members }; \
		static DAVA::IntrospectionInfo info = DAVA::IntrospectionInfo(#_type, sizeof(_type), data, sizeof(data)/sizeof(data[0])); \
		return &info; \
	}

#define  INTROSPECTION_EXTEND(_type, _base_type, _members) \
	static const DAVA::IntrospectionInfo* Info() \
	{ \
		typedef _type ObjectT; \
		static DAVA::IntrospectionMember data[] = { _members }; \
		static DAVA::IntrospectionInfo info = DAVA::IntrospectionInfo(_base_type::Info(), #_type, sizeof(_type), data, sizeof(data)/sizeof(data[0])); \
		return &info; \
	}

#define MEMBER(_name, _desc) \
	DAVA::IntrospectionMember(#_name, _desc, (int) ((long int) &((ObjectT *) 0)->_name), DAVA::MetaInfo::Instance(&ObjectT::_name)),

#endif // __DAVAENGINE_INTROSPECTION_H__
