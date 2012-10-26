#import <Foundation/Foundation.h>
#import "AssetsLibrary/AssetsLibrary.h"

#import "ScreenShotHelper.h"

UIImage* glToUIImage() {
	CGRect screenRect = [[UIScreen mainScreen] bounds];
	NSInteger screenWidth = screenRect.size.width;
	NSInteger screenHeight = screenRect.size.height;
	
    NSInteger myDataLength = screenWidth * screenHeight * 4;
	
    // allocate array and read pixels into it.
    GLubyte *buffer = (GLubyte *) malloc(myDataLength);
    glReadPixels(0, 0, screenWidth, screenHeight, GL_RGBA, GL_UNSIGNED_BYTE, buffer);
	
    // gl renders "upside down" so swap top to bottom into new array.
    // there's gotta be a better way, but this works.

//    GLubyte *buffer2 = (GLubyte *) malloc(myDataLength);
//    for(int y = 0; y < screenHeight; y++)
//    {
//        for(int x = 0; x < screenWidth * 4; x++)
//        {
//            buffer2[(screenHeight-1 - y) * screenWidth * 4 + x] = buffer[y * 4 * screenWidth + x];
//        }
//    }
	
    // make data provider with data.
    CGDataProviderRef provider = CGDataProviderCreateWithData(NULL, buffer, myDataLength, NULL);
	
    // prep the ingredients
    int bitsPerComponent = 8;
    int bitsPerPixel = 32;
    int bytesPerRow = 4 * screenWidth;
    CGColorSpaceRef colorSpaceRef = CGColorSpaceCreateDeviceRGB();
    CGBitmapInfo bitmapInfo = kCGBitmapByteOrderDefault;
    CGColorRenderingIntent renderingIntent = kCGRenderingIntentDefault;
	
    // make the cgimage
    CGImageRef imageRef = CGImageCreate(screenWidth, screenHeight, bitsPerComponent, bitsPerPixel, bytesPerRow, colorSpaceRef, bitmapInfo, provider, NULL, NO, renderingIntent);
	
    // then make the uiimage from that
    UIImage *myImage = [UIImage imageWithCGImage:imageRef scale:1.f orientation:UIImageOrientationLeftMirrored];
	
	return myImage;	
}

ScreenShotHelper ssh;

void ScreenShotHelper::MakeScreenShot() {
	isFinished = false;
	savedFileName = "";

	UIImage *img = glToUIImage();
	ALAssetsLibrary *library = [[ALAssetsLibrary alloc] init];
	
	[library writeImageToSavedPhotosAlbum:[img CGImage] orientation:(ALAssetOrientation)[img imageOrientation] completionBlock:^(NSURL *assetURL, NSError *error) {
		isFinished = true;

		if(error) {
		} else {
			savedFileName = [[assetURL absoluteString] UTF8String];
		}
	}];
	
	[library release];
}

bool ScreenShotHelper::IsFinished() {
	return  isFinished;
}

DAVA::String ScreenShotHelper::GetFileName() {
	return savedFileName;
}

