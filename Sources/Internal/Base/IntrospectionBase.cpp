#include "Base/IntrospectionBase.h"
#include "Base/Meta.h"

namespace DAVA
{

IntrospectionDesription::IntrospectionDesription(const char *_text)
	: text(_text)
{}

IntrospectionDesription::IntrospectionDesription(const char *_text, const EnumMap* _enumMap)
	: text(_text)
	, enumMap(_enumMap)
{ }

IntrospectionMember::IntrospectionMember(const char *_name, const char *_desc, const int _offset, const MetaInfo *_type, int _flags /* = 0 */)
	: name(_name), desc(_desc), offset(_offset), type(_type), flags(_flags)
{ }

const char* IntrospectionMember::Name() const
{
	return name;
}

const char* IntrospectionMember::Desc() const
{
	return desc;
}

const MetaInfo* IntrospectionMember::Type() const
{
	return type;
}

void* IntrospectionMember::Pointer(void *object) const
{
	return (((char *) object) + offset);
}

void* IntrospectionMember::Data(void *object) const
{
	if(type->IsPointer())
	{
		return *(void **) Pointer(object);
	}
	else
	{
		return Pointer(object);
	}
}

VariantType IntrospectionMember::Value(void *object) const
{
	return VariantType::LoadData(Pointer(object), type);
}

void IntrospectionMember::SetValue(void *object, const VariantType &val) const
{
	VariantType::SaveData(Pointer(object), type, val);
}

const IntrospectionCollection* IntrospectionMember::Collection() const
{
	return NULL;
}

int IntrospectionMember::Flags() const
{
	return flags;
}

};
