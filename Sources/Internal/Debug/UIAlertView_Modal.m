/*
 Copyright (c) 2012, José Expósito
 
 Permission is hereby granted, free of charge, to any
 person obtaining a copy of this software and associated
 documentation files (the "Software"), to deal in the
 Software without restriction, including without limitation
 the rights to use, copy, modify, merge, publish,
 distribute, sublicense, and/or sell copies of the
 Software, and to permit persons to whom the Software is
 furnished to do so, subject to the following conditions:
 
 The above copyright notice and this permission notice
 shall be included in all copies or substantial portions of
 the Software.
 
 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY
 KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR
 PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS
 OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR
 OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
 OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
#import "UIAlertView_Modal.h"

NSInteger returnButtonIndex;

@implementation UIAlertView (Modal)

- (NSInteger)showModal
{
	returnButtonIndex = -1;
    self.delegate = self;
    [self show];
	[self autorelease];

	// This view must be modal, so wait for result
	NSRunLoop *theRL = [NSRunLoop currentRunLoop];
	while (returnButtonIndex == -1)
	{
		[[NSRunLoop currentRunLoop] runUntilDate:[NSDate dateWithTimeIntervalSinceNow:0.1f]];
    }

    return returnButtonIndex;
}

- (void)alertView:(UIAlertView *)alertView clickedButtonAtIndex:(NSInteger)buttonIndex
{
    returnButtonIndex = buttonIndex;
}

@end
