#pragma once

#include "Base/BaseMath.h"
#include "Base/BaseTypes.h"
#include "Base/RefPtr.h"
#include "Functional/Function.h"
#include "Render/RHI/rhi_Public.h"
#include "Render/RenderBase.h"

namespace DAVA
{
class UIControl;
class UI3DView;
class Texture;

/**
 * \brief Class for creating screenshot of UIControl to Texture
 */
class UIScreenshoter
{
public:
    /**
    * \brief Default c-tor
    */
    UIScreenshoter();

    /**
    * \brief D-tor
    */
    ~UIScreenshoter();

    /**
    * \brief Call each frame to control internal resources and render flow
    */
    void OnFrame();

    /**
     * \brief Render control to texture and return it immediately (but not rendered) 
     * 
     * \param control pointer to source UIControl
     * \param format PixelFormat
     * \return target texture
     */
    Asset<Texture> MakeScreenshot(UIControl* control, const PixelFormat format, bool clearAlpha = false, bool prepareControl = true);

    /**
     * \brief Render control to texture and call callback when it will rendered
     * 
     * \param control pointer to source UIControl
     * \param format PixelFormat
     * \param callback function which be called after render
     */
    Asset<Texture> MakeScreenshot(UIControl* control, const PixelFormat format, Function<void(const Asset<Texture>&)> callback, bool clearAlpha = false, bool prepareControl = true);

    /**
     * \brief Render control to target texture
     * 
     * \param control pointer to source UIControl
     * \param screenshot pointer to target Texture
     */
    void MakeScreenshot(UIControl* control, const Asset<Texture>& screenshot, const Asset<Texture>& depthTarget, bool clearAlpha = false, bool prepareControl = true);

    /**
    * \brief Render control to target texture
    *
    * \param control pointer to source UIControl
    * \param screenshot pointer to target Texture
    * \param callback function which be called after render
    */
    void MakeScreenshot(UIControl* control, const Asset<Texture>& screenshot, const Asset<Texture>& depthTarget,
                        Function<void(const Asset<Texture>&)> callback, bool clearAlpha = false, bool prepareControl = true,
                        const rhi::Viewport& viewport = rhi::Viewport());

    /**
     * \brief Unsubscribe callback by texture pointer after making screenshot 
     * 
     * \param screenshot pointer to screenshot texture
     */
    void Unsubscribe(const Asset<Texture>& screenshot);

private:
    struct ScreenshotWaiter
    {
        Asset<Texture> texture;
        Asset<Texture> depth;
        rhi::HSyncObject syncObj;
        Function<void(const Asset<Texture>&)> callback;
    };

    void MakeScreenshotInternal(UIControl* control, Asset<Texture> screenshot, Asset<Texture> depthBuffer, Function<void(Asset<Texture>)> callback, bool clearAlpha, bool prepareControl, const rhi::Viewport& viewport = rhi::Viewport());

    List<ScreenshotWaiter> waiters;
};
};
