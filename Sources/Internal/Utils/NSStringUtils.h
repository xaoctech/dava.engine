//
//  NSStringUtils.h
//  Framework
//
//  Created by Aleksei Kanash on 9/16/14.
//
//

#import <Foundation/Foundation.h>
#include "Base/BaseTypes.h"

@interface NSStringUtils : NSObject
+ (NSString *) NSStringFromWideString:(const DAVA::WideString &)str;
@end
