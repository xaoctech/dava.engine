#import "Image.h"

#import <Foundation/Foundation.h>
#import <AssetsLibrary/AssetsLibrary.h>
#import <UIKit/UIKit.h>

#import "Render/Texture.h"

namespace DAVA
{
    
#ifdef __DAVAENGINE_IPHONE__
    
void Image::SaveToSystemPhotos(SaveToSystemPhotoCallbackReceiver* callback)
{
    DVASSERT(format == FORMAT_RGBA8888);

    size_t bitsPerComponent = 8;
    size_t bitsPerPixel = Texture::GetPixelFormatSizeInBits(format);
    size_t bytesPerPixel = Texture::GetPixelFormatSizeInBytes(format);
    size_t bytesPerRow = width * bytesPerPixel;
    CGColorSpaceRef colorSpaceRef = CGColorSpaceCreateDeviceRGB();
    CGBitmapInfo bitmapInfo = kCGBitmapByteOrderDefault;
    CGColorRenderingIntent renderingIntent = kCGRenderingIntentDefault;
    
    CGDataProviderRef provider = CGDataProviderCreateWithData(NULL, data, width * height * bytesPerPixel, NULL);
    
    CGImageRef imageRef = CGImageCreate(width, height, bitsPerComponent, bitsPerPixel, bytesPerRow, colorSpaceRef, bitmapInfo, provider, NULL, NO, renderingIntent);

    UIImage* image = [UIImage imageWithCGImage:imageRef];
    
    ALAssetsLibrary *library = [[ALAssetsLibrary alloc] init];

    if(callback != 0)
    {
        [library writeImageToSavedPhotosAlbum:[image CGImage] orientation:(ALAssetOrientation)[image imageOrientation] completionBlock:^(NSURL *assetURL, NSError *error)
         {
             callback->SaveToSystemPhotosFinished();
        }];
    }
    else
    {
        [library writeImageToSavedPhotosAlbum:[image CGImage] orientation:(ALAssetOrientation)[image imageOrientation] completionBlock:NULL];
    }

    [library release];
}

#endif

}