//
//  PreviewControl.cpp
//  ParticlesEditor
//
//  Created by Igor Solovey on 11/2/11.
//  Copyright (c) 2011 DAVA Consulting. All rights reserved.
//

#include "PreviewControl.h"

#include <algorithm>

REGISTER_CLASS(PreviewControl);

PreviewControl::PreviewControl(const Rect &rect, bool rectInAbsoluteCoordinates)
: UIControl(rect, rectInAbsoluteCoordinates)
{
    GetBackground()->SetDrawType(UIControlBackground::DRAW_FILL);
    GetBackground()->SetColor(Color(0, 0, 0, 1));
}

PreviewControl::~PreviewControl()
{
    Unload();
}

void PreviewControl::Load(const String &yamlPath)
{
    Unload();
    //TODO: add info control for each control loaded from yaml
    UIYamlLoader::Load(this, yamlPath);
    
    CreateInfoControls(this);
}

void PreviewControl::Unload()
{
    RemoveAllControls();
    DeleteInfoControls();
}

void PreviewControl::CreateInfoControls(UIControl* parentControl)
{
    if(!parentControl) return; // skip preview
    
    InfoControl* parentInfoControl = dynamic_cast<InfoControl*>(parentControl);
    if(parentInfoControl) return; // skip info controls
    
    const List<UIControl*>& parentChildren = parentControl->GetChildren();
    List<UIControl*>::const_iterator it = parentChildren.begin();
    
    if(parentControl != this)
    {
        bool isProcessed = false;
        
        InfoControl* infoControl = new InfoControl(parentControl);
        infoControls.push_back(infoControl);
        
        //TODO: button and some other controls may have specific children, need to consider it
        UIButton* parentButton = dynamic_cast<UIButton*>(parentControl);
        if(parentButton)
        {
            UIControl* lastButtonSpecificControl = NULL;
            for(; it != parentChildren.end(); ++it)
            {
                UIControl* child = *it;
                for (int32 st = UIControl::STATE_NORMAL; st < UIControl::STATE_COUNT; ++st) 
                {
                    if(parentButton->GetStateTextControl(st) == child)
                    {
                        lastButtonSpecificControl = child;
                    }
                }
            }
            
            //TODO: add after specific text controls
            if(lastButtonSpecificControl)
            {
                parentButton->InsertChildAbove(infoControl, lastButtonSpecificControl);
                isProcessed = true;
            }
        }
        else 
        {
            // UIList
            UIList* parentList = dynamic_cast<UIList*>(parentControl);
            if(parentList && parentList->GetParent())
            {
                infoControl->SetRect(parentList->GetRect());
                parentList->GetParent()->InsertChildBelow(infoControl, parentList);
                isProcessed = true;
            }
        }
        
        // add infoControl before other controls, so that input will go into it if not processed by children
        if(!isProcessed)
        {
            parentControl->AddControl(infoControl);
            parentControl->BringChildBack(infoControl);
        }
    }
    
    // for each child do the same
    for(; it != parentChildren.end(); ++it)
    {
        UIControl* child = *it;
        CreateInfoControls(child);
    }
}

void PreviewControl::DeleteInfoControls()
{
    std::for_each(infoControls.begin(), infoControls.end(), SafeRelease<InfoControl>);    
    infoControls.clear();
}

int32 PreviewControl::GetInfoControlsCount()
{
    return infoControls.size();
}

InfoControl* PreviewControl::GetInfoControl(int32 index)
{
    DVASSERT(index < infoControls.size());
    return infoControls[index];
}

void PreviewControl::Input(DAVA::UIEvent *touch)
{

}

void PreviewControl::Update(float32 timeElapsed)
{
    
}

void PreviewControl::Draw(const DAVA::UIGeometricData &geometricData)
{
    UIControl::Draw(geometricData);
}