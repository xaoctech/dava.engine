#pragma once

#include "Beast/BeastTypes.h"

#include <Base/BaseTypes.h>
#include "Utils/UTF8Utils.h"

#define DECLARE_BEAST_NAME(name) \
	static DAVA::String GetBeastName() { return DAVA::String(#name); }

#if defined(ILB_STRING_UTF16)
#define GENERATE_BEAST_NAME(type) DAVA::UTF8Utils::EncodeToWideString(BeastNameGenerator::GenerateName<type>()).c_str()
#define STRING_TO_BEAST_STRING(value) DAVA::UTF8Utils::EncodeToWideString(DAVA::String(value)).c_str()
#define CONST_STRING_TO_BEAST_STRING(value) L##value
#else
#define GENERATE_BEAST_NAME(type) BeastNameGenerator::GenerateName<type>().c_str()
#define STRING_TO_BEAST_STRING(value) value.c_str()
#define CONST_STRING_TO_BEAST_STRING(value) value
#endif

class BeastNameGenerator
{
public:
    template <typename T>
    static DAVA::String GenerateName();

private:
    static DAVA::int32 nameIndex;
};

template <typename T>
DAVA::String BeastNameGenerator::GenerateName()
{
    DAVA::String name = DAVA::Format("%s_%d", T::GetBeastName().c_str(), nameIndex);
    nameIndex++;

    return name;
}
