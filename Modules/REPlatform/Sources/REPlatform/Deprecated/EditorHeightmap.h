#pragma once

#include "DAVAEngine.h"

namespace DAVA
{
class EditorHeightmap : public BaseObject
{
    static const DAVA::int32 VALUE_NOT_CHANGED = 0;
    static const DAVA::int32 VALUE_WAS_CHANGED = 1;

public:
    EditorHeightmap(DAVA::Heightmap* heightmap);
    virtual ~EditorHeightmap();

    Heightmap* GetHeightmap();

    void HeghtWasChanged(const DAVA::Rect& changedRect);

    void DrawRelativeRGBA(DAVA::Image* src, DAVA::int32 x, DAVA::int32 y, DAVA::int32 width, DAVA::int32 height, DAVA::float32 k);
    void DrawAverageRGBA(DAVA::Image* mask, DAVA::int32 x, DAVA::int32 y, DAVA::int32 width, DAVA::int32 height, DAVA::float32 k);
    void DrawAbsoluteRGBA(DAVA::Image* mask, DAVA::int32 x, DAVA::int32 y, DAVA::int32 width, DAVA::int32 height, DAVA::float32 time, DAVA::float32 dstHeight);
    void DrawCopypasteRGBA(DAVA::Image* mask, const DAVA::Vector2& posFrom, const DAVA::Vector2& posTo, DAVA::int32 width, DAVA::int32 height, DAVA::float32 koef);

    static void DrawCopypasteRGBA(DAVA::Image* src, DAVA::Image* dst, DAVA::Image* mask, const DAVA::Vector2& posFrom, const DAVA::Vector2& posTo, DAVA::int32 width, DAVA::int32 height);

    static bool Clipping(DAVA::int32& srcOffset, DAVA::int32& dstOffset, DAVA::int32& dstX, DAVA::int32& dstY,
                         DAVA::int32 dstWidth, DAVA::int32 dstHeight, DAVA::int32& width, DAVA::int32& height,
                         DAVA::int32& yAddSrc, DAVA::int32& xAddDst, DAVA::int32& yAddDst);

protected:
    void InitializeTableOfChanges();

    Heightmap* heightmap;
    int32 size = 0;
    uint8* tableOfChanges = nullptr;
};
} // namespace DAVA
