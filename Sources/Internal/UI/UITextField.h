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

#ifndef __DAVAENGINE_UI_TEXT_FIELD_H__
#define __DAVAENGINE_UI_TEXT_FIELD_H__

#include "Base/BaseTypes.h"
#include "UI/UIControl.h"
#include "UI/UIStaticText.h"
#include "UI/UIControlSystem.h"

#ifdef __DAVAENGINE_IPHONE__
#include "UI/UITextFieldiPhone.h"
#endif

namespace DAVA 
{

class UITextField;
#ifdef __DAVAENGINE_ANDROID__
class UITextFieldAndroid;
#endif
/**
    \brief  The UITextFieldDelegate interface defines the messages sent to a text field delegate as part of the sequence of editing its text. 
            All the methods of the interface is optional.
 */
class UITextFieldDelegate
{
public:
	/*virtual void TextFieldShouldBeginEditing(UITextField * textField);
	virtual void TextFieldDidBeginEditing(UITextField * textField);
	virtual void TextFieldShouldEndEditing(UITextField * textField);
	virtual void TextFieldShouldDidEditing(UITextField * textField);*/
	
    /**
        \brief Asks the delegate if the text field should process the pressing of the return button.
        In this function you can check what you want to do with UITextField when return button pressed. 
     */
	virtual void TextFieldShouldReturn(UITextField * textField);

    /**
        \brief Asks the delegate if the text field should process the pressing of the ESC button.
        In this function you can check what you want to do with UITextField when ESC button pressed.
        Don't work on iOS for now.
     */
	virtual void TextFieldShouldCancel(UITextField * textField);
	virtual void TextFieldLostFocus(UITextField * textField);

	/**
        \brief Asks the delegate if the specified text should be changed.
        \param[in] textField The text field containing the text.
        \param[in] replacementLocation starting position of range of characters to be replaced
        \param[in] replacementLength ending position of range of characters to be replaced
        \param[in] replacementString the replacement string.
        \returns true if the specified text range should be replaced; otherwise, false to keep the old text. Default implementation returns true.
	 */
	virtual bool TextFieldKeyPressed(UITextField * textField, int32 replacementLocation, int32 replacementLength, const WideString & replacementString);
    
    virtual bool IsTextFieldShouldSetFocusedOnAppear(UITextField * textField);
    virtual bool IsTextFieldCanLostFocus(UITextField * textField);
};
    
/**
    \brief  A UITextField object is a control that displays editable text and sends an action message to a target object when the user presses the return button. 
            You typically use this class to gather small amounts of text from the user and perform some immediate action, such as a search operation, based on that text.
            A text field object supports the use of a delegate object to handle editing-related notifications. 
 */
class UITextField : public UIControl 
{
public:
	// TODO: fix big BOOLs(TRUE, FALSE) in code
	
	enum eReturnKeyType 
	{
		RETURN_KEY_RETURN = 0,
		RETURN_KEY_DONE
	};

    UITextField();
	
	UITextField(const Rect &rect, bool rectInAbsoluteCoordinates = false);
	virtual ~UITextField();
	
	virtual void WillAppear();
	virtual void DidAppear();
	virtual void WillDisappear();
	
    virtual void OnFocused();
    virtual void OnFocusLost(UIControl *newFocus);

	void SetDelegate(UITextFieldDelegate * delegate);
	UITextFieldDelegate * GetDelegate();

	virtual void Update(float32 timeElapsed);
	
	void OpenKeyboard();
	void CloseKeyboard();
	
	virtual void SetSpriteAlign(int32 align);
    
	const WideString & GetText();
	void SetText(const WideString & text);
    
    virtual WideString GetAppliedChanges(int32 replacementLocation, int32 replacementLength, const WideString & replacementString);


	void SetReturnKey(int32 returnType);

    virtual void Input(UIEvent *currentInput);

    virtual void LoadFromYamlNode(YamlNode * node, UIYamlLoader * loader);
    virtual YamlNode * SaveToYamlNode(UIYamlLoader * loader);
	
	/**
	 \brief Returns the font of control
	 \returns Font font of the control
	 */
    Font *GetFont();
	/**
	 \brief Returns the text color of control.
	 \returns Color color of control's text
	 */
	Color GetTextColor();
	/**
	 \brief Returns text shadow offset relative to base text.
	 \returns Vector2 with shadow offset for X and Y axis
	 */
	Vector2 GetShadowOffset();
	/**
	 \brief Returns color of text shadow.
	 \returns Color of text shadow.
	 */
	Color GetShadowColor();

	int32 GetTextAlign();

    void SetFocused()
    {
        UIControlSystem::Instance()->SetFocusedControl(this, true);
    }
    
    void ReleaseFocus();
    
    virtual bool IsLostFocusAllowed(UIControl *newFocus);
	
  	/**
	 \brief Sets the font of the control text.
	 \param[in] font font used for text draw of the states.
	 */  
    void SetFont(Font * font);
	/**
	 \brief Sets the color of the text.
	 \param[in] fontColor font used for text draw of the states.
	 */
    void SetTextColor(const Color& fontColor);
    DAVA_DEPRECATED(void SetFontColor(const Color& fontColor));
	/**
	 \brief Sets the size of the font.
	 \param[in] size font size to be set.
	 */
    void SetFontSize(float size);
	/**
	 \brief Sets shadow offset of text control.
	 \param[in] offset offset of text shadow relative to base text.
	 */
	void SetShadowOffset(const DAVA::Vector2 &offset);
	/**
	 \brief Sets shadow color of text control.
	 \param[in] color color of text shadow.
	 */
	void SetShadowColor(const Color& color);

	void SetTextAlign(int32 align);

    virtual void SetSize(const DAVA::Vector2 &newSize);
	
	/**
	 \brief Returns list of control children without internal controls.
	 \returns list of control children without internal controls.
	 */
	virtual List<UIControl* >& GetRealChildren();
	
	virtual UIControl *Clone();
	virtual void CopyDataFrom(UIControl *srcControl);

protected:
    bool needRedraw;
	WideString text;
	UITextFieldDelegate * delegate;
	float32	cursorBlinkingTime;
    Font * textFont;
//    Sprite *textSprite;

//    virtual void Draw(const UIGeometricData &geometricData);

    void RenderText();
private:
//    void InitAfterYaml();

#ifdef __DAVAENGINE_IPHONE__
	UITextFieldiPhone * textFieldiPhone;
#elif defined(__DAVAENGINE_ANDROID__)
	UITextFieldAndroid* textFieldAndroid;
#endif


    UIStaticText * staticText;
    float32 cursorTime;
    bool showCursor;

};

};

#endif // __DAVAENGINE_UI_TEXT_FIELD_H__
