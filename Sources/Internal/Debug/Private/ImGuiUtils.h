#pragma once

#include "Base/BaseTypes.h"
#include "Math/Vector.h"

namespace DAVA
{
class ImGuiUtils
{
public:
    /** Set ImGui interface scale. Useful for mobile devices with small screen. */
    static void SetScale(float32 scale);

    /** Get current ImGui interface scale */
    static float32 GetScale();

    /** Set ImGui interface scale from next frame. */
    static void SetScaleFromNextFrame(float32 scale);

    /** Set Imgui virtual screen size. Default: 1024x768. */
    static void SetImGuiVirtualScreenSize(const Vector2& virtualScreenSize);

    /** Get ImGui virtual screen size. */
    static Vector2 GetImGuiVirtualScreenSize();

    /** Get ImGui virtual screen size to physical screen size scale. */
    static float32 GetImGuiVirtualToPhysicalScreenSizeScale();

    /** Convert framework input coords from to ImGui virtual coords. */
    static Vector2 ConvertInputCoordsToImGuiVirtualCoords(const Vector2& inputCoords);

    /** Convert physical coords to ImGui virtual coords. */
    static Vector2 ConvertPhysicalCoordsToImGuiVirtualCoords(const Vector2& physicalCoords);

    /** Convert framework virtual coords to ImGui virtual coords. */
    static Vector2 ConvertVirtualCoordsToImGuiVirtualCoords(const Vector2& virtualCoords);

    /** Convert ImGui virtual coords to physical coords. */
    static Vector2 ConvertImGuiVirtualCoordsToPhysicalCoords(const Vector2& virtualCoords);

    /** Convert ImGui virtual coords to framework virtual coords. */
    static Vector2 ConvertImGuiVirtualCoordsToVirtualCoords(const Vector2& virtualCoords);
};
} // namespace DAVA