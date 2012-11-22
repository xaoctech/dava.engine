#ifndef __DAVAENGINE_META_H__
#define __DAVAENGINE_META_H__

#define META_USE_TYPEID

#ifdef META_USE_TYPEID
#include <typeinfo.h>
#endif

namespace DAVA
{
	template <typename T>
	struct MetaType;

	struct MetaInfo
	{
		MetaInfo(const char *_type_name, int _type_size)
			: type_name(_type_name)
			, type_size(_type_size)
		{ }

		template <typename MetaT>
		static MetaInfo *Instance()
		{
#ifdef META_USE_TYPEID
			static MetaInfo metaInfo(typeid(MetaT).name(), sizeof(MetaT));
#else
			static MetaInfo metaInfo(MetaType<MetaT>::name, sizeof(MetaT));
#endif
			return &metaInfo;
		}

		template <typename ClassT, typename MemberT>
		static MetaInfo* Instance(MemberT ClassT::*var)
		{
			return MetaInfo::Instance<MemberT>();
		}

		inline int GetSize()
		{
			return type_size;
		}

	private:
		const int type_size;
		const char *type_name;
	};
};

#ifndef META_USE_TYPEID

#define REGISTER_META_TYPE(_type) \
	template<> struct DAVA::MetaType<_type> { static const char* name; }; const char* DAVA::MetaType<_type>::name = #_type

REGISTER_META_TYPE(int);
REGISTER_META_TYPE(unsigned int);
REGISTER_META_TYPE(const int);
REGISTER_META_TYPE(const unsigned int);
REGISTER_META_TYPE(int*);
REGISTER_META_TYPE(unsigned int*);
REGISTER_META_TYPE(const int*);
REGISTER_META_TYPE(const unsigned int*);
REGISTER_META_TYPE(std::string);
REGISTER_META_TYPE(const std::string);
REGISTER_META_TYPE(std::string *);
REGISTER_META_TYPE(const std::string *);

#endif // META_USE_TYPEID

#endif // __DAVAENGINE_META_H__
