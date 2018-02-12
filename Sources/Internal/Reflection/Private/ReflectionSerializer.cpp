#include "Base/BaseTypes.h"
#include "Base/FastName.h"
#include "Base/String.h"
#include "Reflection/ReflectionSerializer.h"

#if 0

namespace DAVA
{
/*
enum DefaultBinaryDataTypes
{
    type_pointer    = 'p',
    type_char8      = 'c',
    type_char16     = 'C',
    type_int8       = 'd',
    type_uint8      = 'D',
    type_int16      = 'w',
    type_uint16     = 'W',
    type_int32      = 'i',
    type_uint32     = 'I',
    type_int64      = 'j',
    type_uint64     = 'J',
    type_float32    = 'f',
    type_float64    = 'F',
    type_string     = 's',
    type_fastname   = 'n',
};
*/

std::pair<char, const Type*> supportedTypes[] = {
    { 'b', Type::Instance<bool>() },
    { 'c', Type::Instance<char8>() },
    { 'C', Type::Instance<char16>() },
    { 'd', Type::Instance<int8>() },
    { 'D', Type::Instance<uint8>() },
    { 'w', Type::Instance<int16>() },
    { 'W', Type::Instance<uint16>() },
    { 'i', Type::Instance<int32>() },
    { 'I', Type::Instance<uint32>() },
    { 'j', Type::Instance<int64>() },
    { 'J', Type::Instance<uint64>() },
    { 'f', Type::Instance<float32>() },
    { 'F', Type::Instance<float64>() },
    { 's', Type::Instance<char*>() },
    { 'S', Type::Instance<String>() },
    { 'n', Type::Instance<FastName>() },
    { 'l', Type::Instance<Color>() },
    { 'p', Type::Instance<void*>() },
};

uint32_t GetTypeChar(const Type* type)
{
    static const size_t typesCount = sizeof(supportedTypes) / sizeof(std::pair<char, const Type*>);

    uint32_t ret = 0;

    for (size_t i = 0; i < typesCount; ++i)
    {
        if (supportedTypes[i].second == type)
        {
            ret = supportedTypes[i].first;
            break;
        }
    }

    if (0 == ret && type->IsPointer())
    {
        ret = 'p';
    }

    DVASSERT(ret != 0);
    return ret;
}

void DefaultBinarySerializer::Write(std::ostream& out, const Any& any)
{
    //DAVA::float64
    const Type* type = any.GetType();
    if (type->Is<String>())
    {
        const String* str = static_cast<const String*>(any.GetData());
        uint32_t typeSize = static_cast<uint32_t>(str->size());
        uint32_t typeChar = GetTypeChar(type->Decay());
        out.write(reinterpret_cast<const char*>(&typeSize), sizeof(typeSize));
        out.write(reinterpret_cast<const char*>(&typeChar), sizeof(typeChar));
        out.write(str->data(), typeSize);
    }
    else if (type->Is<FastName>())
    {
        const FastName* fname = static_cast<const FastName*>(any.GetData());
        uint32_t typeSize = 0;

        if (fname->IsValid())
        {
            typeSize = static_cast<uint32_t>(::strlen(fname->c_str()));
        }

        uint32_t typeChar = GetTypeChar(type->Decay());
        out.write(reinterpret_cast<const char*>(&typeSize), sizeof(typeSize));
        out.write(reinterpret_cast<const char*>(&typeChar), sizeof(typeChar));
        out.write(fname->c_str(), typeSize);
    }
    else if (type->Is<const char*>())
    {
        char* const* str = static_cast<char* const*>(any.GetData());
        uint32_t typeSize = 0;

        if (nullptr != *str)
        {
            typeSize = static_cast<uint32_t>(::strlen(*str));
        }

        uint32_t typeChar = GetTypeChar(type->Decay());
        out.write(reinterpret_cast<const char*>(&typeSize), sizeof(typeSize));
        out.write(reinterpret_cast<const char*>(&typeChar), sizeof(typeChar));
        out.write(*str, typeSize);
    }
    else if (type->IsTriviallyCopyable())
    {
        uint32_t typeSize = static_cast<uint32_t>(type->GetSize());
        uint32_t typeChar = GetTypeChar(type->Decay());
        out.write(reinterpret_cast<const char*>(&typeSize), sizeof(typeSize));
        out.write(reinterpret_cast<const char*>(&typeChar), sizeof(typeChar));
        out.write(reinterpret_cast<const char*>(any.GetData()), typeSize);
    }
}

void DefaultBinarySerializer::Read(std::istream& in, Any& any)
{
}

void ReflectionSerializer::Save(std::ostream& out, const Any& key, const Reflection& ref, void* context, bool (*filter)(void*, const Reflection::Field&), PublicSerializer* saver)
{
    static DefaultBinarySerializer defaultBinarySerializer;

    if (nullptr == saver)
    {
        saver = &defaultBinarySerializer;
    }

    saver->Write(out, key);
    saver->Write(out, ref.GetValue());

    if (ref.HasFields())
    {
        Vector<Reflection::Field> fields = ref.GetFields();
        for (Reflection::Field& field : fields)
        {
            bool needSave = true;

            if (nullptr != filter)
            {
                needSave = (*filter)(context, field);
            }

            if (needSave)
            {
                Save(out, field.key, field.ref, context, filter, saver);
            }
        }
    }
}

} // namespace DAVA

#endif