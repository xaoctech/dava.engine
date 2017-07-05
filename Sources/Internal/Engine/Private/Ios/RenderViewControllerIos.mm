#include "Engine/Private/Ios/RenderViewControllerIos.h"

#if defined(__DAVAENGINE_IPHONE__)

#include "Engine/Private/Ios/WindowNativeBridgeIos.h"

@implementation RenderViewController

- (id)initWithBridge:(DAVA::Private::WindowNativeBridge*)nativeBridge
{
    self = [super init];
    if (self != nil)
    {
        bridge = nativeBridge;
    }
    return self;
}

- (void)loadView
{
    bridge->LoadView();
}

- (void)viewWillTransitionToSize:(CGSize)size withTransitionCoordinator:(id<UIViewControllerTransitionCoordinator>)coordinator
{
    bridge->ViewWillTransitionToSize(size.width, size.height);
    [super viewWillTransitionToSize:size withTransitionCoordinator:coordinator];
}

- (UIInterfaceOrientationMask)supportedInterfaceOrientations
{
    return UIInterfaceOrientationMaskLandscape;
}

@end

#endif // __DAVAENGINE_IPHONE__
