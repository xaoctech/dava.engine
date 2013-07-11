#import "MailComposeDelegate.h"

#import <MessageUI/MFMailComposeViewController.h>

@implementation MailComposeDelegate


-(void)mailComposeController:(MFMailComposeViewController*)controller
		 didFinishWithResult:(MFMailComposeResult)result
					   error:(NSError*)error

{
	[controller dismissModalViewControllerAnimated:YES];
	[self release];
}

-(id) initWithController:(UIViewController*) _controller
				   email:(NSString*) _msgEmail
				 subject:(NSString*) _msgSubj
				 message:(NSString*) _msgBody
{
	if ( self = [super init] )
	{
		controller = _controller;
		msgEmail = _msgEmail;
		msgSubj = _msgSubj;
		msgBody =_msgBody;
	}
	return self;
}

-(BOOL) show
{
	if ([MFMailComposeViewController canSendMail])
	{
		MFMailComposeViewController *mailViewController = [[MFMailComposeViewController alloc] init];
		mailViewController.mailComposeDelegate = self;
		
		NSArray *toRecipients = [NSArray arrayWithObjects:msgEmail, nil];
		[mailViewController setToRecipients:toRecipients];
		[mailViewController setSubject:msgSubj];
		[mailViewController setMessageBody:msgBody isHTML:NO];
		
		[controller presentModalViewController:mailViewController animated:YES];
		[mailViewController release];
		return true;
	}
	else
	{
		NSLog(@"Device is unable to send email in its current state.");
		[self release];
		return false;
	}
}
@end