#include "FileSystem/AnySerializer.h"
#include "Base/FastName.h"

namespace DAVA
{
using PrinterFn = void (*)(std::ostringstream&, const Any&);
using PrintersTable = UnorderedMap<const Type*, PrinterFn>;

const PrintersTable printers =
{
  { Type::Instance<int8>(), [](std::ostringstream& out, const Any& any) { out << any.Get<int8>(); } },
  { Type::Instance<uint8>(), [](std::ostringstream& out, const Any& any) { out << any.Get<uint8>(); } },
  { Type::Instance<int16>(), [](std::ostringstream& out, const Any& any) { out << any.Get<int16>(); } },
  { Type::Instance<uint16>(), [](std::ostringstream& out, const Any& any) { out << any.Get<uint16>(); } },
  { Type::Instance<int32>(), [](std::ostringstream& out, const Any& any) { out << any.Get<int32>(); } },
  { Type::Instance<uint32>(), [](std::ostringstream& out, const Any& any) { out << any.Get<uint32>(); } },
  { Type::Instance<int64>(), [](std::ostringstream& out, const Any& any) { out << any.Get<int64>(); } },
  { Type::Instance<uint64>(), [](std::ostringstream& out, const Any& any) { out << any.Get<uint64>(); } },
  { Type::Instance<float32>(), [](std::ostringstream& out, const Any& any) { out << any.Get<float32>(); } },
  { Type::Instance<float64>(), [](std::ostringstream& out, const Any& any) { out << any.Get<float64>(); } },
  { Type::Instance<String>(), [](std::ostringstream& out, const Any& any) { out << any.Get<String>().c_str(); } },
  { Type::Instance<WideString>(), [](std::ostringstream& out, const Any& any) { out << any.Get<WideString>().c_str(); } },
  { Type::Instance<FastName>(), [](std::ostringstream& out, const Any& any) { out << any.Get<FastName>().c_str(); } },
  { Type::Instance<size_t>(), [](std::ostringstream& out, const Any& any) { out << any.Get<size_t>(); } },
  { Type::Instance<bool>(), [](std::ostringstream& out, const Any& any) { out << (any.Get<bool>() ? "true" : "false"); } },
};

String AnySerializer::AnyToString(const Any& any)
{
    const Type* type = any.GetType();
    PrinterFn printFunction = nullptr;
    if (type->IsPointer())
    {
        DVASSERT(0 && "Pointers not yet supported");
        return String();
    }

    if (nullptr != type->Decay())
        type = type->Decay();

    auto it = printers.find(type);
    if (it != printers.end())
    {
        printFunction = it->second;
    }

    std::ostringstream resultStream;
    if (printFunction != nullptr)
    {
        (*printFunction)(resultStream, any);
    }
    return resultStream.str();
}
};
