/*==================================================================================
    Copyright (c) 2008, DAVA Consulting, LLC
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are met:
    * Redistributions of source code must retain the above copyright
    notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
    notice, this list of conditions and the following disclaimer in the
    documentation and/or other materials provided with the distribution.
    * Neither the name of the DAVA Consulting, LLC nor the
    names of its contributors may be used to endorse or promote products
    derived from this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE DAVA CONSULTING, LLC AND CONTRIBUTORS "AS IS" AND
    ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL DAVA CONSULTING, LLC BE LIABLE FOR ANY
    DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
    ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
    SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

    Revision History:
        * Created by Vitaliy Borodovsky 
=====================================================================================*/
#include "FileSystem/FileSystem.h"

#if defined(__DAVAENGINE_MACOS__) || defined(__DAVAENGINE_IPHONE__)

#include "FileSystem/FileList.h"
#include "Debug/DVAssert.h"
#include "Utils/Utils.h"
#include "Utils/StringFormat.h"
#include "FileSystem/ResourceArchive.h"

#import <Foundation/NSString.h>
#import <Foundation/NSBundle.h>
#import <Foundation/NSArray.h>
#import <Foundation/NSPathUtilities.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/errno.h>
#include <copyfile.h>

namespace DAVA
{

	
#if defined(__DAVAENGINE_IPHONE__)
	NSString * FilepathRelativeToBundleObjC(const String &virtualBundlePath, NSString * relativePathname)
	{
		NSString * filePath;
		if(virtualBundlePath.empty())
		{
				//		NSString * bundlePath = [[[NSBundle mainBundle] bundlePath] stringByAppendingString: @""];
			NSString * bundlePath = [[[NSBundle mainBundle] bundlePath] stringByAppendingString: @"/Data"];
			filePath = [bundlePath stringByAppendingString: relativePathname];
		}
		else 
		{
			NSString * bundlePath = [NSString stringWithUTF8String: virtualBundlePath.c_str()];
			filePath = [bundlePath stringByAppendingString: relativePathname];
		}
		
		return filePath;
	}
#elif defined(__DAVAENGINE_MACOS__)
    NSString * FilepathRelativeToBundleObjC(const FilePath &virtualBundlePath,
                                            NSString * relativePathname)
	{
        NSString * filePath;
        if(virtualBundlePath.IsInitalized())
        {
            NSString * bundlePath = [NSString stringWithUTF8String: virtualBundlePath.GetAbsolutePathname().c_str()];
            filePath = [bundlePath stringByAppendingString: relativePathname];
        }
        else
        {
            NSString * bundlePath = [[[NSBundle mainBundle] bundlePath] stringByAppendingString: @"/Contents/Resources/Data"];
            filePath = [bundlePath stringByAppendingString: relativePathname];
        }
		
		return filePath;
	}
#endif	//#elif defined(__DAVAENGINE_MACOS__)
	
#if defined(__DAVAENGINE_IPHONE__) || defined(__DAVAENGINE_MACOS__)
	
    const char * FileSystem::FilepathRelativeToBundle(const char * relativePathname)
	{
		NSString * filePath = FilepathRelativeToBundleObjC(virtualBundlePath, [NSString stringWithUTF8String: relativePathname]);
		return [filePath UTF8String];
	}
	
    const FilePath FileSystem::GetUserDocumentsPath()
    {
        NSArray * paths = NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES);
        NSString * bundlePath = [paths objectAtIndex:0];
        NSString * filePath = [bundlePath stringByAppendingString: @"/"];
        return FilePath(String([filePath cStringUsingEncoding: NSUTF8StringEncoding]));
    }
    
    const FilePath FileSystem::GetPublicDocumentsPath()
    {
        return FilePath("/Users/Shared/");
    }

    const FilePath FileSystem::GetHomePath()
    {
        NSString * dirPath = NSHomeDirectory();
        return FilePath(String([[dirPath stringByAppendingString: @"/"] cStringUsingEncoding:NSUTF8StringEncoding]));
    }

#endif //#if defined(__DAVAENGINE_IPHONE__) || defined(__DAVAENGINE_MACOS__)	
	
}

#endif //#if defined(__DAVAENGINE_MACOS__) || defined(__DAVAENGINE_IPHONE__)




