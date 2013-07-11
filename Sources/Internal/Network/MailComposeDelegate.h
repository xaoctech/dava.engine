#import <UIKit/UIKit.h>
#import <MessageUI/MFMailComposeViewController.h>

@interface MailComposeDelegate: NSObject <MFMailComposeViewControllerDelegate>
{
@protected
	
	UIViewController* controller;
	NSString* msgEmail;
	NSString* msgSubj;
	NSString* msgBody;
}

-(id) initWithController:(UIViewController*) _controller
				   email:(NSString*) _msgEmail
				 subject:(NSString*) _msgSubj
				 message:(NSString*) _msgBody;

-(BOOL) show;

@end