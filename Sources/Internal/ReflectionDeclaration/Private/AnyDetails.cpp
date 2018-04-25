#include "ReflectionDeclaration/Private/AnyDetails.h"

#include "Base/Any.h"
#include "Base/BaseTypes.h"
#include "Base/FastName.h"
#include "FileSystem/FilePath.h"
#include "Math/Matrix2.h"
#include "Math/Matrix3.h"
#include "Math/Matrix4.h"
#include "Render/RenderBase.h"
#include "Utils/StringFormat.h"
#include "Utils/UTF8Utils.h"

#include <cstdlib>

namespace DAVA
{
namespace AnyCastsDetails
{
String FastNameAsString(const Any& value)
{
    const FastName& v = value.Get<FastName>();
    return v.IsValid() ? v.c_str() : String();
}

const char* FastNameAsCharPointer(const Any& value)
{
    const FastName& v = value.Get<FastName>();
    return v.IsValid() ? v.c_str() : "";
}

const char* StringAsCharPointer(const Any& value)
{
    const String& v = value.Get<String>();
    return v.c_str();
}
}

Any FastNameToCharPointer(const Any& value)
{
    const FastName& v = value.Get<FastName>();
    if (v.IsValid() == false)
    {
        return nullptr;
    }
    return v.c_str();
}

Any Matrix2ToString(const Any& value)
{
    Matrix2 matrix = value.Get<Matrix2>();
    return Format("[%f, %f]\n[%f, %f]",
                  matrix._data[0][0], matrix._data[0][1],
                  matrix._data[1][0], matrix._data[1][1]);
}

Any Matrix3ToString(const Any& value)
{
    Matrix3 matrix = value.Get<Matrix3>();
    return Format("[%f, %f, %f]\n[%f, %f, %f]\n[%f, %f, %f]",
                  matrix._data[0][0], matrix._data[0][1], matrix._data[0][2],
                  matrix._data[1][0], matrix._data[1][1], matrix._data[1][2],
                  matrix._data[2][0], matrix._data[2][1], matrix._data[2][2]
                  );
}

Any Matrix4ToString(const Any& value)
{
    Matrix4 matrix = value.Get<Matrix4>();
    return Format("[%f, %f, %f, %f]\n[%f, %f, %f, %f]\n[%f, %f, %f, %f]\n[%f, %f, %f, %f]",
                  matrix._data[0][0], matrix._data[0][1], matrix._data[0][2], matrix._data[0][3],
                  matrix._data[1][0], matrix._data[1][1], matrix._data[1][2], matrix._data[1][3],
                  matrix._data[2][0], matrix._data[2][1], matrix._data[2][2], matrix._data[2][3],
                  matrix._data[3][0], matrix._data[3][1], matrix._data[3][2], matrix._data[3][3]
                  );
}

void RegisterAnyCasts()
{
    AnyCast<const char*, String>::Register([](const Any& v) -> Any { return String(v.Get<const char*>()); });
    AnyCast<const char*, FastName>::Register([](const Any& v) -> Any { return FastName(v.Get<const char*>()); });

    // from string
    AnyCast<String, const char*>::Register([](const Any& v) -> Any { return v.Get<String>().c_str(); });
    AnyCast<String, FastName>::Register([](const Any& v) -> Any { return FastName(v.Get<String>().c_str()); });
    AnyCast<String, FilePath>::Register([](const Any& v) -> Any { return FilePath(v.Get<String>()); });
    AnyCast<String, WideString>::Register([](const Any& v) -> Any { return UTF8Utils::EncodeToWideString(v.Get<String>()); });
    AnyCast<String, int8>::Register([](const Any& v) -> Any { return static_cast<int8>(std::strtol(AnyCastsDetails::StringAsCharPointer(v), nullptr, 10)); });
    AnyCast<String, int16>::Register([](const Any& v) -> Any { return static_cast<int16>(std::strtol(AnyCastsDetails::StringAsCharPointer(v), nullptr, 10)); });
    AnyCast<String, int32>::Register([](const Any& v) -> Any { return static_cast<int32>(std::strtol(AnyCastsDetails::StringAsCharPointer(v), nullptr, 10)); });
    AnyCast<String, int64>::Register([](const Any& v) -> Any { return static_cast<int64>(std::strtoll(AnyCastsDetails::StringAsCharPointer(v), nullptr, 10)); });
    AnyCast<String, uint8>::Register([](const Any& v) -> Any { return static_cast<uint8>(std::strtoul(AnyCastsDetails::StringAsCharPointer(v), nullptr, 10)); });
    AnyCast<String, uint16>::Register([](const Any& v) -> Any { return static_cast<uint16>(std::strtoul(AnyCastsDetails::StringAsCharPointer(v), nullptr, 10)); });
    AnyCast<String, uint32>::Register([](const Any& v) -> Any { return static_cast<uint32>(std::strtoul(AnyCastsDetails::StringAsCharPointer(v), nullptr, 10)); });
    AnyCast<String, uint64>::Register([](const Any& v) -> Any { return static_cast<uint64>(std::strtoull(AnyCastsDetails::StringAsCharPointer(v), nullptr, 10)); });
    AnyCast<String, float32>::Register([](const Any& v) -> Any { return static_cast<float32>(std::strtof(AnyCastsDetails::StringAsCharPointer(v), nullptr)); });
    AnyCast<String, float64>::Register([](const Any& v) -> Any { return static_cast<float64>(std::strtod(AnyCastsDetails::StringAsCharPointer(v), nullptr)); });

    // from fast name
    AnyCast<FastName, const char*>::Register(&FastNameToCharPointer);
    AnyCast<FastName, String>::Register([](const Any& v) -> Any { return AnyCastsDetails::FastNameAsString(v); });
    AnyCast<FastName, int8>::Register([](const Any& v) -> Any { return static_cast<int8>(std::strtol(AnyCastsDetails::FastNameAsCharPointer(v), nullptr, 10)); });
    AnyCast<FastName, int16>::Register([](const Any& v) -> Any { return static_cast<int16>(std::strtol(AnyCastsDetails::FastNameAsCharPointer(v), nullptr, 10)); });
    AnyCast<FastName, int32>::Register([](const Any& v) -> Any { return static_cast<int32>(std::strtol(AnyCastsDetails::FastNameAsCharPointer(v), nullptr, 10)); });
    AnyCast<FastName, int64>::Register([](const Any& v) -> Any { return static_cast<int64>(std::strtoll(AnyCastsDetails::FastNameAsCharPointer(v), nullptr, 10)); });
    AnyCast<FastName, uint8>::Register([](const Any& v) -> Any { return static_cast<uint8>(std::strtoul(AnyCastsDetails::FastNameAsCharPointer(v), nullptr, 10)); });
    AnyCast<FastName, uint16>::Register([](const Any& v) -> Any { return static_cast<uint16>(std::strtoul(AnyCastsDetails::FastNameAsCharPointer(v), nullptr, 10)); });
    AnyCast<FastName, uint32>::Register([](const Any& v) -> Any { return static_cast<uint32>(std::strtoul(AnyCastsDetails::FastNameAsCharPointer(v), nullptr, 10)); });
    AnyCast<FastName, uint64>::Register([](const Any& v) -> Any { return static_cast<uint64>(std::strtoull(AnyCastsDetails::FastNameAsCharPointer(v), nullptr, 10)); });
    AnyCast<FastName, float32>::Register([](const Any& v) -> Any { return static_cast<float32>(std::strtof(AnyCastsDetails::FastNameAsCharPointer(v), nullptr)); });
    AnyCast<FastName, float64>::Register([](const Any& v) -> Any { return static_cast<float64>(std::strtod(AnyCastsDetails::FastNameAsCharPointer(v), nullptr)); });

    // to string
    AnyCast<int8, String>::Register([](const Any& v) -> Any { return std::to_string(v.Get<int8>()); });
    AnyCast<int16, String>::Register([](const Any& v) -> Any { return std::to_string(v.Get<int16>()); });
    AnyCast<int32, String>::Register([](const Any& v) -> Any { return std::to_string(v.Get<int32>()); });
    AnyCast<int64, String>::Register([](const Any& v) -> Any { return std::to_string(v.Get<int64>()); });
    AnyCast<uint8, String>::Register([](const Any& v) -> Any { return std::to_string(v.Get<uint8>()); });
    AnyCast<uint16, String>::Register([](const Any& v) -> Any { return std::to_string(v.Get<uint16>()); });
    AnyCast<uint32, String>::Register([](const Any& v) -> Any { return std::to_string(v.Get<uint32>()); });
    AnyCast<uint64, String>::Register([](const Any& v) -> Any { return std::to_string(v.Get<uint64>()); });
    AnyCast<float32, String>::Register([](const Any& v) -> Any { return std::to_string(v.Get<float32>()); });
    AnyCast<float64, String>::Register([](const Any& v) -> Any { return std::to_string(v.Get<float64>()); });
    AnyCast<size_t, String>::Register([](const Any& v) -> Any { return std::to_string(v.Get<size_t>()); });

    AnyCast<WideString, String>::Register([](const Any& v) -> Any { return UTF8Utils::EncodeToUTF8(v.Get<WideString>()); });
    AnyCast<FilePath, String>::Register([](const Any& v) -> Any { return v.Get<FilePath>().GetAbsolutePathname(); });
    AnyCast<Matrix2, String>::Register(&Matrix2ToString);
    AnyCast<Matrix3, String>::Register(&Matrix3ToString);
    AnyCast<Matrix4, String>::Register(&Matrix4ToString);

    // to fast name
    AnyCast<int8, FastName>::Register([](const Any& v) -> Any { return FastName(std::to_string(v.Get<int8>()).c_str()); });
    AnyCast<int16, FastName>::Register([](const Any& v) -> Any { return FastName(std::to_string(v.Get<int16>()).c_str()); });
    AnyCast<int32, FastName>::Register([](const Any& v) -> Any { return FastName(std::to_string(v.Get<int32>()).c_str()); });
    AnyCast<int64, FastName>::Register([](const Any& v) -> Any { return FastName(std::to_string(v.Get<int64>()).c_str()); });
    AnyCast<uint8, FastName>::Register([](const Any& v) -> Any { return FastName(std::to_string(v.Get<uint8>()).c_str()); });
    AnyCast<uint16, FastName>::Register([](const Any& v) -> Any { return FastName(std::to_string(v.Get<uint16>()).c_str()); });
    AnyCast<uint32, FastName>::Register([](const Any& v) -> Any { return FastName(std::to_string(v.Get<uint32>()).c_str()); });
    AnyCast<uint64, FastName>::Register([](const Any& v) -> Any { return FastName(std::to_string(v.Get<uint64>()).c_str()); });
    AnyCast<float32, FastName>::Register([](const Any& v) -> Any { return FastName(std::to_string(v.Get<float32>()).c_str()); });
    AnyCast<float64, FastName>::Register([](const Any& v) -> Any { return FastName(std::to_string(v.Get<float64>()).c_str()); });
    AnyCast<size_t, FastName>::Register([](const Any& v) -> Any { return FastName(std::to_string(v.Get<size_t>()).c_str()); });

    // float/integral
    AnyCast<float64, float32>::RegisterDefault();
    AnyCast<float32, float64>::RegisterDefault();
    AnyCast<int8, float32>::RegisterDefault();
    AnyCast<int8, float64>::RegisterDefault();
    AnyCast<int16, float32>::RegisterDefault();
    AnyCast<int16, float64>::RegisterDefault();
    AnyCast<int32, float32>::RegisterDefault();
    AnyCast<int32, float64>::RegisterDefault();
    AnyCast<int64, float32>::RegisterDefault();
    AnyCast<int64, float64>::RegisterDefault();
    AnyCast<uint8, float32>::RegisterDefault();
    AnyCast<uint8, float64>::RegisterDefault();
    AnyCast<uint16, float32>::RegisterDefault();
    AnyCast<uint16, float64>::RegisterDefault();
    AnyCast<uint32, float32>::RegisterDefault();
    AnyCast<uint32, float64>::RegisterDefault();
    AnyCast<uint64, float32>::RegisterDefault();
    AnyCast<uint64, float64>::RegisterDefault();
    AnyCast<size_t, float32>::RegisterDefault();
    AnyCast<size_t, float64>::RegisterDefault();
    AnyCast<float32, int8>::RegisterDefault();
    AnyCast<float64, int8>::RegisterDefault();
    AnyCast<float32, int16>::RegisterDefault();
    AnyCast<float64, int16>::RegisterDefault();
    AnyCast<float32, int32>::RegisterDefault();
    AnyCast<float64, int32>::RegisterDefault();
    AnyCast<float32, int64>::RegisterDefault();
    AnyCast<float64, int64>::RegisterDefault();
    AnyCast<float32, uint8>::RegisterDefault();
    AnyCast<float64, uint8>::RegisterDefault();
    AnyCast<float32, uint16>::RegisterDefault();
    AnyCast<float64, uint16>::RegisterDefault();
    AnyCast<float32, uint32>::RegisterDefault();
    AnyCast<float64, uint32>::RegisterDefault();
    AnyCast<float32, uint64>::RegisterDefault();
    AnyCast<float64, uint64>::RegisterDefault();
    AnyCast<float32, size_t>::RegisterDefault();
    AnyCast<float64, size_t>::RegisterDefault();
}

void RegisterAnyHashes()
{
    AnyHash<float32>::RegisterDefault();
    AnyHash<float64>::RegisterDefault();
    AnyHash<pointer_size>::RegisterDefault();
    AnyHash<String>::RegisterDefault();
    AnyHash<FastName>::Register([](const Any& any) { return std::hash<const char*>()(any.Get<FastName>().c_str()); });
}

} // namespace DAVA
