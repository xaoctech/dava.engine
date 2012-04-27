#include "LodDistanceControl.h"
#include "ControlsFactory.h"

LodDistanceControl::LodDistanceControl(LodDistanceControlDelegate *newDelegate, const Rect &rect, bool rectInAbsoluteCoordinates)
    :   UIControl(rect, rectInAbsoluteCoordinates)
{
    count = 0;

    maxDistance = 0;
    
    zones = NULL;
    sliders = NULL;
    
    delegate = newDelegate;
    
    mainTouch = -1;
    oldPos = newPos = 0.f;
    
    activeSlider = NULL;
    leftZone = NULL;
    rightZone = NULL;
    
    activeLodIndex = -1;
    
    for(int32 iDist = 0; iDist < LodNode::MAX_LOD_LAYERS; ++iDist)
    {
        distanceText[iDist] = new UIStaticText(Rect(0, (iDist+1) * ControlsFactory::BUTTON_HEIGHT, 
                                                    rect.dx / 2, ControlsFactory::BUTTON_HEIGHT));
        
        distanceText[iDist]->SetAlign(ALIGN_LEFT | ALIGN_VCENTER);
        distanceText[iDist]->SetFont(ControlsFactory::GetFontLight());
        
        
        distanceTextValues[iDist] = new UITextField(Rect(rect.dx / 2, (iDist+1) * ControlsFactory::BUTTON_HEIGHT, 
                                                         rect.dx / 2, ControlsFactory::BUTTON_HEIGHT));
        ControlsFactory::CustomizeEditablePropertyCell(distanceTextValues[iDist]);
        distanceTextValues[iDist]->SetFont(ControlsFactory::GetFontLight());
        distanceTextValues[iDist]->SetDelegate(this);
        distanceTextValues[iDist]->SetInputEnabled((0 != iDist), false);
    }
}

LodDistanceControl::~LodDistanceControl()
{
    for(int32 iDist = 0; iDist < LodNode::MAX_LOD_LAYERS; ++iDist)
    {
        SafeRelease(distanceText[iDist]);
        SafeRelease(distanceTextValues[iDist]);
    }
    
    ReleaseControls();
}


void LodDistanceControl::WillDisappear()
{
    ReleaseControls();    

    UIControl::WillDisappear();
}

void LodDistanceControl::ReleaseControls()
{
    for(int32 iZone = 0; iZone < count; ++iZone)
    {
        RemoveControl(zones[iZone]);
        SafeRelease(zones[iZone]);
    }
    SafeDeleteArray(zones);
    
    for(int32 iSlider = 0; iSlider < count - 1; ++iSlider)
    {
        RemoveControl(sliders[iSlider]);
        SafeRelease(sliders[iSlider]);
    }
    SafeDeleteArray(sliders);
    
    count = 0;
    
    activeSlider = NULL;
    leftZone = NULL;
    rightZone = NULL;
    
    activeLodIndex = -1;
}

void LodDistanceControl::SetDistances(float32 *newDistances, int32 newCount)
{
    Vector2 newSize = GetSize();
    newSize.y = (newCount + 1) * ControlsFactory::BUTTON_HEIGHT;
    SetSize(newSize);
    
    ReleaseControls();
    
    if(1 < newCount)
    {
        count = newCount;
        Memcpy(distances, newDistances, count * sizeof(float32));
        
        zones = new UIControl*[count];
        
        maxDistance = (distances[count - 1] < LodNode::MAX_LOD_DISTANCE) 
                                        ? LodNode::MAX_LOD_DISTANCE 
                                        : (LodNode::MAX_LOD_DISTANCE * 1.1f);
        
        Rect fullRect = GetRect();
        
        Color initialColor(0.8f, 0.5f, 0.1f, 1.0f);
        
        for(int32 iZone = 0; iZone < count; ++iZone)
        {
            float32 x = (iZone) ? distances[iZone] : 0.f;
            float32 dx = (iZone != count - 1) ? (distances[iZone + 1] - x) : (maxDistance - distances[iZone]);

            x = x * fullRect.dx / maxDistance;
            dx = dx * fullRect.dx / maxDistance;
            
            
            zones[iZone] = new UIControl(Rect(x, 0, dx, ControlsFactory::BUTTON_HEIGHT));
            zones[iZone]->SetInputEnabled(false, false);
            zones[iZone]->GetBackground()->SetDrawType(UIControlBackground::DRAW_FILL);
            
            float32 color = distances[iZone] / maxDistance;
            
            Color zoneColor;
            zoneColor.r = initialColor.r + color;
            zoneColor.g = initialColor.g + color;
            zoneColor.b = initialColor.b + color;
            zoneColor.a = 1.0f;
            
            if(1.0f < zoneColor.r) zoneColor.r -= (int32)zoneColor.r;
            if(1.0f < zoneColor.g) zoneColor.g -= (int32)zoneColor.g;
            if(1.0f < zoneColor.b) zoneColor.b -= (int32)zoneColor.b;
            
            zones[iZone]->GetBackground()->SetColor(zoneColor);
            
            AddControl(zones[iZone]);
        }
        
        sliders = new UIControl*[count - 1];
        for(int32 iSlider = 0; iSlider < count - 1; ++iSlider)
        {
            float32 x = distances[iSlider + 1] / maxDistance * fullRect.dx;
            sliders[iSlider] = new UIControl(Rect(x - 1, 0, 3, ControlsFactory::BUTTON_HEIGHT));
            sliders[iSlider]->SetInputEnabled(false, false);
            sliders[iSlider]->GetBackground()->SetDrawType(UIControlBackground::DRAW_FILL);
            sliders[iSlider]->GetBackground()->SetColor(Color(1.0f, 0.0f, 0.0f, 1.0f));
            
            AddControl(sliders[iSlider]);
        }
    }
    
    for(int32 iDist = 0; iDist < LodNode::MAX_LOD_LAYERS; ++iDist)
    {
        RemoveControl(distanceText[iDist]);
        RemoveControl(distanceTextValues[iDist]);
        
        if(iDist < count)
        {
            AddControl(distanceText[iDist]);
            distanceText[iDist]->SetText(Format(L"Distance_%d:", iDist));

            AddControl(distanceTextValues[iDist]);
            distanceTextValues[iDist]->SetText(Format(L"%3.0f", distances[iDist]));
        }
    }
}

void LodDistanceControl::Input(UIEvent *currentInput)
{
    if(-1 == mainTouch && UIEvent::PHASE_BEGAN == currentInput->phase)
    {
        mainTouch =  currentInput->tid;
    }

    
    if(mainTouch == currentInput->tid)
    {
        newPos = currentInput->point.x;
        Vector2 point = currentInput->point - GetPosition(true);

        switch (currentInput->phase) 
        {
            case UIEvent::PHASE_BEGAN:
            {
                leftEdge = Vector2(0.0f, 0.0f);
                rightEdge = Vector2(GetRect().dx, 0.0f);
                
                for(int32 iSlider = 0; iSlider < count - 1; ++iSlider)
                {
                    if(sliders[iSlider]->GetRect().PointInside(point))
                    {
                        activeSlider = sliders[iSlider];
                        
                        if(iSlider)
                        {
                            leftEdge.x = sliders[iSlider - 1]->GetRect().x + sliders[iSlider - 1]->GetRect().dx;
                        }
                        if(iSlider < count - 2)
                        {
                            rightEdge.x = sliders[iSlider + 1]->GetRect().x;
                        }
                        
                        leftZone = zones[iSlider];
                        rightZone = zones[iSlider + 1];
                        
                        activeLodIndex = iSlider + 1;
                        
                        break;
                    }
                }
                
                oldPos = newPos;
                break;
            }
            case UIEvent::PHASE_DRAG:
            {
                UpdateSliderPos();

                break;
            }
            case UIEvent::PHASE_ENDED:
            {
                UpdateSliderPos();

                activeSlider = NULL;
                leftZone = rightZone = NULL;
                
                mainTouch = -1;
                activeLodIndex = -1;
                break;
            }
        }
    }
}

void LodDistanceControl::UpdateSliderPos()
{
    if(activeSlider)
    {
        float32 deltax = newPos - oldPos;
        oldPos = newPos;

        Vector2 newPosition = activeSlider->GetPosition() + Vector2(deltax, 0.0f);
        newPosition.x = Max(newPosition.x, leftEdge.x);
        newPosition.x = Min(newPosition.x, rightEdge.x - 3);

        Vector2 deltaPosition = activeSlider->GetPosition() - newPosition;
        activeSlider->SetPosition(newPosition);
        
        Rect leftRect = leftZone->GetRect();
        leftRect.dx -= deltaPosition.x;
        leftZone->SetRect(leftRect);

        Rect rightRect = rightZone->GetRect();
        rightRect.x -= deltaPosition.x;
        rightRect.dx += deltaPosition.x;
        rightZone->SetRect(rightRect);
        
        float32 newDistance = ((newPosition.x + 1) / GetRect().dx * maxDistance);
        if(delegate)
        {
            delegate->DistanceChanged(this, activeLodIndex, newDistance);
        }
        
        distances[activeLodIndex] = newDistance;
        distanceTextValues[activeLodIndex]->SetText(Format(L"%3.0f", newDistance));
    }
}

#pragma mark  --UITextFieldDelegate
void LodDistanceControl::TextFieldShouldReturn(UITextField * textField)
{
    textField->ReleaseFocus();
}

void LodDistanceControl::TextFieldShouldCancel(UITextField * textField)
{
    textField->ReleaseFocus();
}

void LodDistanceControl::TextFieldLostFocus(UITextField * textField)
{
    for(int32 iText = 0; iText < count; ++iText)
    {
        if(textField == distanceTextValues[iText])
        {
            float32 newDistance = atof(WStringToString(textField->GetText()).c_str());
            
            newDistance = Max(newDistance, 0.f);
            newDistance = Min(newDistance, (float32)LodNode::MAX_LOD_DISTANCE);
            if(iText)
            {
                newDistance = Max(newDistance, distances[iText-1]);
            }
            if(iText < count - 1)
            {
                newDistance = Min(newDistance, distances[iText + 1]);
            }

            //TODO: udpate sliders
            float32 deltaX = ((newDistance - distances[iText]) / maxDistance * GetRect().dx);
            if(iText)
            {
                Vector2 sz = zones[iText - 1]->GetSize();
                sz.x += deltaX; 
                zones[iText-1]->SetSize(sz);
                
                Vector2 pos = sliders[iText - 1]->GetPosition();
                pos.x += deltaX;
                sliders[iText - 1]->SetPosition(pos);
            }
            
            Rect r = zones[iText]->GetRect();
            r.x += deltaX; 
            r.dx -= deltaX;
            zones[iText]->SetRect(r);

            textField->SetText(Format(L"%3.0f", newDistance));
            distances[iText] = newDistance;
            if(delegate)
            {
                delegate->DistanceChanged(this, iText, newDistance);
            }
            
            break;
        }
    }
}

bool LodDistanceControl::TextFieldKeyPressed(UITextField * textField, int32 replacementLocation, int32 replacementLength, const WideString & replacementString)
{
    if (replacementLength < 0) 
    {
        return true;
    }
    
    WideString newText = textField->GetAppliedChanges(replacementLocation, replacementLength, replacementString);
    bool allOk;
    int pointsCount = 0;
    for (int i = 0; i < newText.length(); i++) 
    {
        allOk = false;
        if (newText[i] == L'-' && i == 0)
        {
            allOk = true;
        }
        else if(newText[i] >= L'0' && newText[i] <= L'9')
        {
            allOk = true;
        }
        else if(newText[i] == L'.' && pointsCount == 0)
        {
            allOk = true;
            pointsCount++;
        }
        if (!allOk) 
        {
            return false;
        }
    }
    return true;
};

