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
