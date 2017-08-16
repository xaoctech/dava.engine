#ifdef __DAVAENGINE_BEAST__

#include "BeastDebug.h"

DAVA::String ConvertBeastString(DAVA_BEAST::ILBStringHandle h)
{
    DAVA_BEAST::int32 len = 0;
    DAVA_BEAST::ILBGetLength(h, &len);
    if (len < 2)
    {
        return DAVA::String();
    }

#if defined(ILB_STRING_UTF16)
    DAVA::WideString wResult(len - 1, 0);
    DAVA_BEAST::ILBCopy(h, &wResult.front(), len);
    DAVA::String result = DAVA::UTF8Utils::EncodeToUTF8(wResult);
#else
    DAVA::String result(len - 1, 0);
    DAVA_BEAST::ILBCopy(h, &result[0], len);
#endif

    DAVA_BEAST::ILBReleaseString(h);
    return result;
}

DAVA_BEAST::ILBMatrix4x4 ConvertDavaMatrix(const DAVA::Matrix4& davaMatrix)
{
    DAVA::Matrix4 transposed = davaMatrix;
    transposed.Transpose();
    DAVA_BEAST::ILBMatrix4x4 matrix;
    for (DAVA::int32 index = 0; index < 16; ++index)
    {
        matrix.m[index] = transposed.data[index];
    }

    return matrix;
}

DAVA::Matrix4 ConvertBeastMatrix(const DAVA_BEAST::ILBMatrix4x4& matrix)
{
    DAVA::Matrix4 davaMatrix;
    for (DAVA::int32 index = 0; index < 16; ++index)
    {
        davaMatrix.data[index] = matrix.m[index];
    }
    davaMatrix.Transpose();
    return davaMatrix;
}

DAVA_BEAST::ILBMatrix4x4 ConvertDavaMatrixNoTranspose(const DAVA::Matrix4& davaMatrix)
{
    DAVA::Matrix4 transposed = davaMatrix;
    //transposed.Transpose();
    DAVA_BEAST::ILBMatrix4x4 matrix;
    for (DAVA::int32 index = 0; index < 16; ++index)
    {
        matrix.m[index] = transposed.data[index];
    }

    return matrix;
}

#endif //__DAVAENGINE_BEAST__
