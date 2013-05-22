/*==================================================================================
    Copyright (c) 2008, DAVA, INC
    All rights reserved.

    Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:
    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the DAVA, INC nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE DAVA CONSULTING, LLC AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL DAVA CONSULTING, LLC BE LIABLE FOR ANY
    DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
=====================================================================================*/

#include "BaseTypes.h"

#if defined(__DAVAENGINE_IPHONE__)

#include <UIKit/UIKit.h>
#include "UI/UITextField.h"
#include "UI/UITextFieldiPhone.h"
#include "Core/Core.h"

float GetUITextViewSizeDivider()
{
    float divider = 1.f;
    if (DAVA::Core::IsAutodetectContentScaleFactor()) 
    {
        if ([::UIScreen instancesRespondToSelector: @selector(scale) ]
            && [::UIView instancesRespondToSelector: @selector(contentScaleFactor) ]) 
        {
            divider = [[::UIScreen mainScreen] scale];
        }
    }
    
    return divider;
}

@interface UITextFieldHolder : UIView < UITextFieldDelegate >
{
@public
	UITextField * textField;
	DAVA::UITextField * cppTextField;
}
- (id) init : (DAVA::UITextField  *) tf;
- (void) dealloc;
- (BOOL)textFieldShouldReturn:(UITextField *)textField;
- (BOOL)textField:(UITextField *)_textField shouldChangeCharactersInRange:(NSRange)range replacementString:(NSString *)string;

@end

@implementation UITextFieldHolder

- (id) init : (DAVA::UITextField  *) tf
{
	if (self = [super init])
	{
        float divider = GetUITextViewSizeDivider();
		self.transform = CGAffineTransformMakeRotation(DAVA::DegToRad(-90.0f));
		self.bounds = CGRectMake(0.0f, 0.0f, DAVA::Core::Instance()->GetPhysicalScreenHeight()/divider, DAVA::Core::Instance()->GetPhysicalScreenWidth()/divider);
		self.center = CGPointMake(DAVA::Core::Instance()->GetPhysicalScreenWidth()/2/divider, DAVA::Core::Instance()->GetPhysicalScreenHeight()/2/divider);	
		self.userInteractionEnabled = FALSE;
		
		cppTextField = tf;
		DAVA::Rect rect = tf->GetRect();
		textField = [[UITextField alloc] initWithFrame: CGRectMake((rect.x - DAVA::Core::Instance()->GetVirtualScreenXMin()) 
                                                                   * DAVA::Core::GetVirtualToPhysicalFactor()
																   , (rect.y - DAVA::Core::Instance()->GetVirtualScreenYMin()) * DAVA::Core::GetVirtualToPhysicalFactor()
																   , rect.dx * DAVA::Core::GetVirtualToPhysicalFactor()
																   , rect.dy * DAVA::Core::GetVirtualToPhysicalFactor())];
		textField.frame = CGRectMake((rect.x - DAVA::Core::Instance()->GetVirtualScreenXMin()) * DAVA::Core::GetVirtualToPhysicalFactor()
									 , (rect.y - DAVA::Core::Instance()->GetVirtualScreenYMin()) * DAVA::Core::GetVirtualToPhysicalFactor()
									 , rect.dx * DAVA::Core::GetVirtualToPhysicalFactor()
									 , rect.dy * DAVA::Core::GetVirtualToPhysicalFactor());
        
		textField.delegate = self;
		
		[self addSubview:textField];
	}
	return self;
}


- (void) dealloc
{
	[textField release];
	textField = 0;
	[super dealloc];
}


- (BOOL)textFieldShouldReturn:(UITextField *)textField
{
	if (cppTextField)
	{
		if (cppTextField->GetDelegate() != 0)
			cppTextField->GetDelegate()->TextFieldShouldReturn(cppTextField);
	}
	return TRUE;
}

- (BOOL)textField:(UITextField *)_textField shouldChangeCharactersInRange:(NSRange)range replacementString:(NSString *)string
{
	if (cppTextField)
	{
		if (cppTextField->GetDelegate() != 0)
		{
			int length = [string length];
			
			DAVA::WideString repString;
			repString.resize(length); 
			for (int i = 0; i < length; i++) 
			{
				unichar uchar = [string characterAtIndex:i];
				repString[i] = (wchar_t)uchar;
			}
			return cppTextField->GetDelegate()->TextFieldKeyPressed(cppTextField, range.location, range.length, repString);
		}
	}
	return TRUE;
}


@end

void CreateTextField(DAVA::UITextField  * tf)
{
	///[textFieldHolder->textField becomeFirstResponder];
}

void ReleaseTextField()
{
}

void OpenKeyboard()
{
}

void CloseKeyboard()
{
}

namespace DAVA 
{
    UITextFieldiPhone::UITextFieldiPhone(void  * tf)
    {
        UITextFieldHolder * textFieldHolder = [[UITextFieldHolder alloc] init: (DAVA::UITextField*)tf];
        objcClassPtr = textFieldHolder;
    }
    UITextFieldiPhone::~UITextFieldiPhone()
    {
        UITextFieldHolder * textFieldHolder = (UITextFieldHolder*)objcClassPtr;
        [textFieldHolder removeFromSuperview];
        [textFieldHolder release];
        textFieldHolder = 0;
    }
	
    void UITextFieldiPhone::SetTextColor(const DAVA::Color &color)
    {
        UITextFieldHolder * textFieldHolder = (UITextFieldHolder*)objcClassPtr;
        textFieldHolder->textField.textColor = [UIColor colorWithRed:color.r green:color.g blue:color.b alpha:color.a];
        
    }
    void UITextFieldiPhone::SetFontSize(float size)
    {
        UITextFieldHolder * textFieldHolder = (UITextFieldHolder*)objcClassPtr;
        float scaledSize = size * Core::GetVirtualToPhysicalFactor();
        
        if( [[::UIScreen mainScreen] respondsToSelector:@selector(displayLinkWithTarget:selector:)])
        {
            scaledSize /= [::UIScreen mainScreen].scale;
        }
        textFieldHolder->textField.font = [UIFont systemFontOfSize:scaledSize];
    }
    
    void UITextFieldiPhone::SetTextAlign(DAVA::int32 align)
    {
        UITextFieldHolder * textFieldHolder = (UITextFieldHolder*)objcClassPtr;
        if (align & ALIGN_LEFT)
            textFieldHolder->textField.contentHorizontalAlignment = UIControlContentHorizontalAlignmentLeft;
        else if (align & ALIGN_HCENTER)
            textFieldHolder->textField.contentHorizontalAlignment = UIControlContentHorizontalAlignmentCenter;
        else if (align & ALIGN_RIGHT)
            textFieldHolder->textField.contentHorizontalAlignment = UIControlContentHorizontalAlignmentRight;

        if (align & ALIGN_TOP)
            textFieldHolder->textField.contentVerticalAlignment = UIControlContentVerticalAlignmentTop;
        else if (align & ALIGN_VCENTER)
            textFieldHolder->textField.contentVerticalAlignment = UIControlContentVerticalAlignmentCenter;
        else if (align & ALIGN_BOTTOM)
            textFieldHolder->textField.contentVerticalAlignment = UIControlContentVerticalAlignmentBottom;
    }
	
    DAVA::int32 UITextFieldiPhone::GetTextAlign()
    {
        UITextFieldHolder * textFieldHolder = (UITextFieldHolder*)objcClassPtr;
        
        DAVA::int32 retValue = 0;
        
        
        UIControlContentHorizontalAlignment horAligment = textFieldHolder->textField.contentHorizontalAlignment;
        UIControlContentVerticalAlignment   verAligment = textFieldHolder->textField.contentVerticalAlignment;
        
        switch (horAligment)
        {
            case UIControlContentHorizontalAlignmentLeft:
                retValue |= ALIGN_LEFT;
                break;
            case UIControlContentHorizontalAlignmentCenter:
                retValue |= ALIGN_HCENTER;
                break;
            case UIControlContentHorizontalAlignmentRight:
                retValue |= ALIGN_RIGHT;
                break;
        }
        
        switch (verAligment)
        {
            case UIControlContentVerticalAlignmentTop:
                retValue |= ALIGN_TOP;
                break;
            case UIControlContentVerticalAlignmentCenter:
                retValue |= ALIGN_VCENTER;
                break;
            case UIControlContentVerticalAlignmentBottom:
                retValue |= ALIGN_BOTTOM;
                break;
        }
        
    return retValue;
    }
    
    void UITextFieldiPhone::SetReturnKey(int32 returnType)
    {
        UITextFieldHolder * textFieldHolder = (UITextFieldHolder*)objcClassPtr;
        switch (returnType) 
        {
            case UITextField::RETURN_KEY_RETURN:
            {
                textFieldHolder->textField.returnKeyType = UIReturnKeyRoute;
            }
                break;
            case UITextField::RETURN_KEY_DONE:
            {
                textFieldHolder->textField.returnKeyType = UIReturnKeyDone;
            }
                break;
        }
        
    }
    
    
    void UITextFieldiPhone::OpenKeyboard()
    {
        UITextFieldHolder * textFieldHolder = (UITextFieldHolder*)objcClassPtr;
        [textFieldHolder->textField becomeFirstResponder];
    }
    
    void UITextFieldiPhone::CloseKeyboard()
    {
        UITextFieldHolder * textFieldHolder = (UITextFieldHolder*)objcClassPtr;
        [textFieldHolder->textField resignFirstResponder];
    }
    
    void UITextFieldiPhone::ShowField()
    {
        UITextFieldHolder * textFieldHolder = (UITextFieldHolder*)objcClassPtr;
        [[[UIApplication sharedApplication] keyWindow] addSubview: textFieldHolder];
    }
    
    void UITextFieldiPhone::HideField()
    {
        UITextFieldHolder * textFieldHolder = (UITextFieldHolder*)objcClassPtr;
        [textFieldHolder removeFromSuperview];
    }
    
    void UITextFieldiPhone::UpdateRect(const Rect & rect)
    {
        float divider = GetUITextViewSizeDivider();
        UITextFieldHolder * textFieldHolder = (UITextFieldHolder*)objcClassPtr;
        CGRect cgRect = CGRectMake((rect.x - DAVA::Core::Instance()->GetVirtualScreenXMin()) * DAVA::Core::GetVirtualToPhysicalFactor()/divider
                                   , (rect.y - DAVA::Core::Instance()->GetVirtualScreenYMin()) * DAVA::Core::GetVirtualToPhysicalFactor()/divider
                                   , rect.dx * DAVA::Core::GetVirtualToPhysicalFactor()/divider
                                   , rect.dy * DAVA::Core::GetVirtualToPhysicalFactor()/divider);
        textFieldHolder->textField.frame = cgRect;
    }
	
    void UITextFieldiPhone::SetText(std::wstring & string)
    {
        UITextFieldHolder * textFieldHolder = (UITextFieldHolder*)objcClassPtr;
        textFieldHolder->textField.text = [[ [ NSString alloc ]  
                                            initWithBytes : (char*)string.data()   
                                            length : string.size() * sizeof(wchar_t)   
                                            encoding : CFStringConvertEncodingToNSStringEncoding ( kCFStringEncodingUTF32LE ) ] autorelease]; 
    }
	
    void UITextFieldiPhone::GetText(std::wstring & string)
    {
        UITextFieldHolder * textFieldHolder = (UITextFieldHolder*)objcClassPtr;
        int length = [textFieldHolder->textField.text length];
		
        string.resize(length); 
        for (int i = 0; i < length; i++) 
        {
            unichar uchar = [textFieldHolder->textField.text characterAtIndex:i];
            string[i] = (wchar_t)uchar;
        }
    }
    
    
}

#endif
