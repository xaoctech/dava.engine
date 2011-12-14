#ifdef __DAVAENGINE_BEAST__

#ifndef __BEAST_NAME_GENERATOR__
#define __BEAST_NAME_GENERATOR__

#include "DAVAEngine.h"

#define DECLARE_BEAST_NAME(name) \
	static DAVA::String GetBeastName() { return DAVA::String(#name); }

#define GENERATE_BEAST_NAME(type) \
	BeastNameGenerator::GenerateName<type>().c_str()

class BeastNameGenerator
{
public:
	template<typename T>
	static DAVA::String GenerateName();

private:
	static DAVA::int32 nameIndex;
};

template<typename T>
DAVA::String BeastNameGenerator::GenerateName()
{
	DAVA::String name = DAVA::Format("%s_%d", T::GetBeastName().c_str(), nameIndex);
	nameIndex++;

	return name;
}

#endif //__BEAST_NAME_GENERATOR__
#endif //__DAVAENGINE_BEAST__