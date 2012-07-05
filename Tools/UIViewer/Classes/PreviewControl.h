//
//  PreviewControl.h
//  ParticlesEditor
//
//  Created by Igor Solovey on 11/2/11.
//  Copyright (c) 2011 DAVA Consulting. All rights reserved.
//

#ifndef __PREVIEW_CONTROL_H__
#define __PREVIEW_CONTROL_H__

#include "DAVAEngine.h"

#include "InfoControl.h"

using namespace DAVA;

class PreviewControl : public UIControl
{
public:
    PreviewControl(const Rect &rect = Rect(), bool rectInAbsoluteCoordinates = false);
    virtual ~PreviewControl();
    
    virtual void Load(const String &yamlPath);
    virtual void Unload();
    
    virtual void Update(float32 timeElapsed);
	virtual void Draw(const UIGeometricData &geometricData);
	
	virtual void Input(UIEvent * touch);
    
    int32 GetInfoControlsCount();
    InfoControl* GetInfoControl(int32 index);
    
protected:
    void CreateInfoControls(UIControl* parentControl);
    void DeleteInfoControls();
    
    Vector<InfoControl*> infoControls; // clickable controls to get control information
};

#endif
