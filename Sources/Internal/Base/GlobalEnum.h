#ifndef __DAVAENGINE_GLOBAL_ENUM_H__
#define __DAVAENGINE_GLOBAL_ENUM_H__

#include "Base/BaseTypes.h"
#include "Base/StaticSingleton.h"
#include "Base/Meta.h"

class GlobalEnum : public DAVA::StaticSingleton<GlobalEnum>
{
public:
	GlobalEnum();
	~GlobalEnum();

	template<typename T> void Add(int value, const char *str);

	template<typename T> int StringToValue(const char *str);
	template<typename T> const char* ValueToString(int value);

	template<typename T> int GetCount() const;
	template<typename T> int GetValue(int index) const ;
	template<typename T> const char* GetString(int index) const;

protected:
	struct EnumValues
	{
		DAVA::Map<int, const char *> valueToString;
	};

	void Add(const DAVA::MetaInfo *meta, int value, const char *str);

	int StringToValue(const DAVA::MetaInfo *meta, const char *str);
	const char* ValueToString(const DAVA::MetaInfo *meta, int value);

	int GetCount(const DAVA::MetaInfo *meta) const;
	int GetValue(const DAVA::MetaInfo *meta, int index) const ;
	const char* GetString(const DAVA::MetaInfo *meta, int index) const;

	DAVA::Map<const DAVA::MetaInfo *, EnumValues*> enumValues;
};

#define DECLARE_ENUM 


#endif // __DAVAENGINE_GLOBAL_ENUM_H__
