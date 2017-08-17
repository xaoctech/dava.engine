#include "BeastDebug.h"

DAVA::String ConvertBeastString(ILBStringHandle h)
{
    int32 len = 0;
    ILBGetLength(h, &len);
    if (len < 2)
    {
        return DAVA::String();
    }

#if defined(ILB_STRING_UTF16)
    DAVA::WideString wResult(len - 1, 0);
    ILBCopy(h, &wResult.front(), len);
    DAVA::String result = DAVA::UTF8Utils::EncodeToUTF8(wResult);
#else
    DAVA::String result(len - 1, 0);
    ILBCopy(h, &result[0], len);
#endif

    ILBReleaseString(h);
    return result;
}

ILBMatrix4x4 ConvertDavaMatrix(const DAVA::Matrix4& davaMatrix)
{
    DAVA::Matrix4 transposed = davaMatrix;
    transposed.Transpose();
    ILBMatrix4x4 matrix;
    for (DAVA::int32 index = 0; index < 16; ++index)
    {
        matrix.m[index] = transposed.data[index];
    }

    return matrix;
}

DAVA::Matrix4 ConvertBeastMatrix(const ILBMatrix4x4& matrix)
{
    DAVA::Matrix4 davaMatrix;
    for (DAVA::int32 index = 0; index < 16; ++index)
    {
        davaMatrix.data[index] = matrix.m[index];
    }
    davaMatrix.Transpose();
    return davaMatrix;
}

ILBMatrix4x4 ConvertDavaMatrixNoTranspose(const DAVA::Matrix4& davaMatrix)
{
    DAVA::Matrix4 transposed = davaMatrix;
    //transposed.Transpose();
    ILBMatrix4x4 matrix;
    for (DAVA::int32 index = 0; index < 16; ++index)
    {
        matrix.m[index] = transposed.data[index];
    }

    return matrix;
}
