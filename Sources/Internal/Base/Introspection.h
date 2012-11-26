#ifndef __DAVAENGINE_INTROSPECTION_H__
#define __DAVAENGINE_INTROSPECTION_H__

#include "Base/Meta.h"
#include "FileSystem/VariantType.h"

namespace DAVA
{
	struct IntrospectionMember
	{
		IntrospectionMember(const char *_name, const char *_desc, const int _offset, const MetaInfo *_meta)
			: name(_name), desc(_desc), offset(_offset), meta(_meta) 
		{ }

		const char *name;
		const char *desc;
		const int offset;
		const MetaInfo* meta;

		VariantType GetValue(const void *object) const
		{
			return VariantType::LoadData(((char *) object) + offset, meta);
		}

		void SetValue(const void *object, const VariantType &val) const
		{
			VariantType::SaveData(((char *) object) + offset, meta, val);
		}
	};

	struct IntrospectionInfo
	{
		IntrospectionInfo(const char *_name, const int _size, const IntrospectionMember *_members, const int _members_count)
			: name(_name)
			, size(_size)
			, members(_members)
			, members_count(_members_count)
		{ }

		const char *name;
		const int size;
		const IntrospectionMember *members;
		const int members_count;
	};
};

#define INTROSPECTION(_type, _members) \
	static const DAVA::IntrospectionInfo& Info() \
	{ \
		typedef _type ObjectT; \
		static DAVA::IntrospectionMember data[] = { _members }; \
		static DAVA::IntrospectionInfo info = DAVA::IntrospectionInfo(#_type, sizeof(_type), data, sizeof(data)/sizeof(data[0])); \
		return info; \
	}

#define MEMBER(_name, _desc) \
	DAVA::IntrospectionMember(#_name, _desc, (int) &((ObjectT *) 0)->_name, DAVA::MetaInfo::Instance(&ObjectT::_name)),

#endif // __DAVAENGINE_INTROSPECTION_H__
