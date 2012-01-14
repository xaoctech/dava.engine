/*
    Copyright (c) 2008, DAVA Consulting, LLC
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are met:
    * Redistributions of source code must retain the above copyright
    notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
    notice, this list of conditions and the following disclaimer in the
    documentation and/or other materials provided with the distribution.
    * Neither the name of the <organization> nor the
    names of its contributors may be used to endorse or promote products
    derived from this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
    ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL <COPYRIGHT HOLDER> BE LIABLE FOR ANY
    DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
    ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
    SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 
 
    * Created by Vitaliy Borodovsky
*/

#include "CameraController.h"

namespace DAVA 
{

CameraController::CameraController()
    : camera(0)
{

}
CameraController::~CameraController()
{
    SafeRelease(camera);
}

void CameraController::SetCamera(Camera * _camera)
{
    SafeRelease(camera);
    camera = SafeRetain(_camera);
}

void CameraController::Input(UIEvent * event)
{
    
}
    
WASDCameraController::WASDCameraController(float32 _speed)
    : speed(_speed)
    , viewZAngle(0)
    , viewYAngle(0)
	, selection(0)
{

}
void WASDCameraController::SetSpeed(float32 _speed)
{
    speed = _speed;
}

WASDCameraController::~WASDCameraController()
{
    
}
    
void WASDCameraController::Update(float32 timeElapsed)
{
    UITextField *tf = dynamic_cast<UITextField *>(UIControlSystem::Instance()->GetFocusedControl());
    if(!tf && camera)
    {
        float32 moveSpeed = speed * timeElapsed;        

        {
            
            bool moveUp = (InputSystem::Instance()->GetKeyboard()->IsKeyPressed(DVKEY_UP) | 
                           InputSystem::Instance()->GetKeyboard()->IsKeyPressed(DVKEY_W));
            if(moveUp)
            {
                Vector3 pos = camera->GetPosition();
                Vector3 direction = camera->GetDirection();
                //Logger::Debug("oldpos: %f %f %f olddir: %f %f %f", pos.x, pos.y, pos.z, direction.x, direction.y, direction.z);
                
                direction.Normalize();
                pos += direction * moveSpeed * SystemTimer::Instance()->FrameDelta();
                camera->SetPosition(pos);
                camera->SetDirection(direction);    // right now required because camera rebuild direction to target, and if position & target is equal after set position it produce wrong results
                
                //Logger::Debug("newpos: %f %f %f", pos.x, pos.y, pos.z);
            }
 
            bool moveLeft = (InputSystem::Instance()->GetKeyboard()->IsKeyPressed(DVKEY_LEFT) | 
                           InputSystem::Instance()->GetKeyboard()->IsKeyPressed(DVKEY_A));
            if(moveLeft)
            {
                Vector3 pos = camera->GetPosition();
                Vector3 dir = camera->GetDirection();
                Vector3 left = camera->GetLeft();
                
                pos -= left * moveSpeed * SystemTimer::Instance()->FrameDelta();
                camera->SetPosition(pos);
                camera->SetDirection(dir);
            }
            
            
            bool moveDown = (InputSystem::Instance()->GetKeyboard()->IsKeyPressed(DVKEY_DOWN) | 
                             InputSystem::Instance()->GetKeyboard()->IsKeyPressed(DVKEY_S));
            if(moveDown)
            {
                Vector3 pos = camera->GetPosition();
                Vector3 direction = camera->GetDirection();
                //Logger::Debug("oldpos: %f %f %f olddir: %f %f %f", pos.x, pos.y, pos.z, direction.x, direction.y, direction.z);
                
                pos -= direction * moveSpeed * SystemTimer::Instance()->FrameDelta();
                camera->SetPosition(pos);
                camera->SetDirection(direction);    // right now required because camera rebuild direction to target, and if position & target is equal after set position it produce wrong results
                //Logger::Debug("newpos: %f %f %f olddir: %f %f %f", pos.x, pos.y, pos.z, direction.x, direction.y, direction.z);
            }

            
            bool moveRight = (InputSystem::Instance()->GetKeyboard()->IsKeyPressed(DVKEY_RIGHT) | 
                             InputSystem::Instance()->GetKeyboard()->IsKeyPressed(DVKEY_D));
            if(moveRight)
            {
                Vector3 pos = camera->GetPosition();
                Vector3 dir = camera->GetDirection();
                Vector3 left = camera->GetLeft();
                
                pos += left * moveSpeed * SystemTimer::Instance()->FrameDelta();
                camera->SetPosition(pos);
                camera->SetDirection(dir);
            }
        }
    }
    
    CameraController::Update(timeElapsed);
}
 
#define MAX_ANGLE 89
	
void WASDCameraController::Input(UIEvent * event)
{
    if (!camera)return;
    if (event->phase == UIEvent::PHASE_KEYCHAR)
    {   
        switch (event->tid) 
        {
            case DVKEY_Z:
            {
				if (selection)
				{
					AABBox3 box = selection->GetWTMaximumBoundingBox();						
					float32 boxSize = ((box.max - box.min).Length());
					
					const Vector3 & pos = camera->GetPosition();
					const Vector3 & targ = camera->GetTarget();
					
					Vector3 dir = targ - pos;
					dir.Normalize();
					
					const Vector3 & c = box.GetCenter();
					
					camera->SetTarget(c);
					camera->SetPosition(c - (dir * boxSize));
				}					
				break;					
			}
            case DVKEY_T:
            {
				if (!camera)return;
				
				viewZAngle = 0;
				viewYAngle = -MAX_ANGLE;
				
				Matrix4 mt, mt2;
				mt.CreateRotation(Vector3(0,0,1), DegToRad(viewZAngle));
				mt2.CreateRotation(Vector3(1,0,0), DegToRad(viewYAngle));
				mt2 *= mt;
				Vector3 vect = Vector3(0,0, 200);
				
				Vector3 position = vect - Vector3(0, 10, 0) * mt2;
				
				camera->SetTarget(position);
				camera->SetPosition(vect);					
				break;					
			}
				
                
            default:
                break;
        }
    } 

	bool altBut3 = (selection && event->tid == UIEvent::BUTTON_3 && InputSystem::Instance()->GetKeyboard()->IsKeyPressed(DVKEY_ALT));
	
	
	if(UIEvent::PHASE_BEGAN == event->phase)
	{
		startPt = stopPt = event->point;			

		if (altBut3)
		{
			const Vector3 & pos = camera->GetPosition();
			AABBox3 box = selection->GetWTMaximumBoundingBox();
			center = box.GetCenter();
			radius = (center - pos).Length();
		}
	}
	else if(UIEvent::PHASE_DRAG == event->phase || UIEvent::PHASE_ENDED == event->phase)
	{
		startPt = stopPt;
		stopPt = event->point;

		if (event->tid == UIEvent::BUTTON_1)
		{
			UpdateCam1But();
		}
		else if (altBut3)
		{
			UpdateCamAlt3But();			
		}
		else if (event->tid == UIEvent::BUTTON_3)
		{
			UpdateCam3But();
		}
	}	
}

	void WASDCameraController::UpdateCam1But(void)
	{
		Vector2 dp = startPt - stopPt;
		dp *= 8.f;
		viewZAngle -= dp.x * SystemTimer::Instance()->FrameDelta() * 1.5f;
		viewYAngle += dp.y * SystemTimer::Instance()->FrameDelta();
		
		if(viewYAngle < -MAX_ANGLE)
			viewYAngle = -MAX_ANGLE;
		
		if(viewYAngle > MAX_ANGLE)
			viewYAngle = MAX_ANGLE;			
		
		Matrix4 mt, mt2;
		mt.CreateRotation(Vector3(0,0,1), DegToRad(viewZAngle));
		mt2.CreateRotation(Vector3(1,0,0), DegToRad(viewYAngle));
		mt2 *= mt;
		
		Vector3 dir = Vector3(0, -10, 0) * mt2;
		camera->SetDirection(dir);		
	}
	
	void WASDCameraController::UpdateCamAlt3But(void)
	{
		if (!camera)return;
		
		viewZAngle += (stopPt.x - startPt.x);
		viewYAngle -= (stopPt.y - startPt.y);
		
		
		if(viewYAngle < -MAX_ANGLE)
			viewYAngle = -MAX_ANGLE;
		
		if(viewYAngle > MAX_ANGLE)
			viewYAngle = MAX_ANGLE;


		Matrix4 mt, mt2;
		mt.CreateRotation(Vector3(0,0,1), DegToRad(viewZAngle));
		mt2.CreateRotation(Vector3(1,0,0), DegToRad(viewYAngle));
		mt2 *= mt;
		
		Vector3 position = center + Vector3(0, radius, 0) * mt2;
		
		camera->SetTarget(center);
		camera->SetPosition(position);
	}

	void WASDCameraController::UpdateCam3But(void)
	{
		if (!camera)return;		
		Vector2 dp = (stopPt - startPt) * 0.1f;

		Matrix4 mt, mt1, mt2, mt3;
		mt.Identity();
		mt1.CreateTranslation(Vector3(dp.x, 0, dp.y));
		mt2.CreateRotation(Vector3(0,0,1), DegToRad(viewZAngle));
		mt3.CreateRotation(Vector3(1,0,0), DegToRad(viewYAngle));
		
		mt *= mt1;
		mt *= mt2;
		mt *= mt3;
		
		
		Vector3 pos = camera->GetPosition() + (Vector3(0, 0, 0) * mt);
		Vector3 dir = camera->GetDirection();
		camera->SetPosition(pos);
		camera->SetDirection(dir);
	}
    
};
