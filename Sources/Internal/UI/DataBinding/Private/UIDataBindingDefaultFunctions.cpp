#include "UIDataBindingDefaultFunctions.h"

#include "Math/MathHelpers.h"
#include "FileSystem/LocalizationSystem.h"

namespace DAVA
{
int32 UIDataBindingDefaultFunctions::IntMin(int32 a, int32 b)
{
    return Min(a, b);
}

float32 UIDataBindingDefaultFunctions::FloatMin(float32 a, float32 b)
{
    return Min(a, b);
}

int32 UIDataBindingDefaultFunctions::IntMax(int32 a, int32 b)
{
    return Max(a, b);
}

float32 UIDataBindingDefaultFunctions::FloatMax(float32 a, float32 b)
{
    return Max(a, b);
}

int32 UIDataBindingDefaultFunctions::IntClamp(int32 val, int32 a, int32 b)
{
    return Clamp(val, a, b);
}

float32 UIDataBindingDefaultFunctions::FloatClamp(float32 val, float32 a, float32 b)
{
    return Clamp(val, a, b);
}

int32 UIDataBindingDefaultFunctions::IntAbs(int32 a)
{
    return Abs(a);
}

float32 UIDataBindingDefaultFunctions::FloatAbs(float32 a)
{
    return Abs(a);
}

float32 UIDataBindingDefaultFunctions::RadToDeg(float32 f)
{
    return DAVA::RadToDeg(f);
}

float32 UIDataBindingDefaultFunctions::DegToRad(float32 f)
{
    return DAVA::DegToRad(f);
}

// Strings
String UIDataBindingDefaultFunctions::IntToStr(int32 a)
{
    return Format("%d", a);
}

String UIDataBindingDefaultFunctions::IntToStr1000Separated(int32 value)
{
    String resultStr;

    bool negative = value < 0;
    value = Abs(value);

    while (value >= 1000)
    {
        resultStr = Format(" %03d", value % 1000) + resultStr;
        value /= 1000;
    }

    value = negative ? -value : value;
    resultStr = Format("%d", value) + resultStr;

    return resultStr;
}

String UIDataBindingDefaultFunctions::FloatToStr(float32 a)
{
    return Format("%f", a);
}

String UIDataBindingDefaultFunctions::FloatToStrWithPrecision(float32 a, int32 precision)
{
    return Format("%.*f", precision, a);
}

String UIDataBindingDefaultFunctions::Float64ToStr(float64 a)
{
    return Format("%f", a);
}

String UIDataBindingDefaultFunctions::Float64ToStrWithPrecision(float64 a, int32 precision)
{
    return Format("%.*f", precision, a);
}

String UIDataBindingDefaultFunctions::Vector2ToStr(const Vector2& value)
{
    return Format("%.2f, %.2f", value.x, value.y);
}

String UIDataBindingDefaultFunctions::Localize(const String& key)
{
    return LocalizedUtf8String(key);
}
}
