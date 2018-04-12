#pragma once

#include "Base/BaseTypes.h"
#include "Math/Vector.h"

namespace DAVA
{
class UIDataBindingDefaultFunctions
{
public:
    // Math
    static int32 IntMin(int32 a, int32 b);
    static float32 FloatMin(float32 a, float32 b);

    static int32 IntMax(int32 a, int32 b);
    static float32 FloatMax(float32 a, float32 b);

    static int32 IntClamp(int32 val, int32 a, int32 b);
    static float32 FloatClamp(float32 val, float32 a, float32 b);

    static int32 IntAbs(int32 a);
    static float32 FloatAbs(float32 a);

    static float32 RadToDeg(float32 f);
    static float32 DegToRad(float32 f);

    // Strings
    static String IntToStr(int32 a);
    static String IntToStr1000Separated(int32 a);
    static String FloatToStr(float32 a);
    static String FloatToStrWithPrecision(float32 a, int32 precision);
    static String Float64ToStr(float64 a);
    static String Float64ToStrWithPrecision(float64 a, int32 precision);
    static String Vector2ToStr(const Vector2& value);
    static String Localize(const String& key);
};
}
