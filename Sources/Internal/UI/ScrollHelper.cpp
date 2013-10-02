/*==================================================================================
    Copyright (c) 2008, binaryzebra
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright
    notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
    notice, this list of conditions and the following disclaimer in the
    documentation and/or other materials provided with the distribution.
    * Neither the name of the binaryzebra nor the
    names of its contributors may be used to endorse or promote products
    derived from this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE binaryzebra AND CONTRIBUTORS "AS IS" AND
    ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL binaryzebra BE LIABLE FOR ANY
    DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
    ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
    SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
=====================================================================================*/



#include "UI/ScrollHelper.h"
#include "FileSystem/Logger.h"

namespace DAVA 
{
	ScrollHelper::ScrollHelper()
	:	BaseObject()
	,	position(0.f, 0.f)
	,	elementSize(0.f, 0.f)
	,	totalDeltaTime(0)
	,	totalDeltaMove(0)
	,   speed(0)
	{
		slowDown = 0.25f;
		backward = 0.3f;
	}

	void ScrollHelper::SetPosition(float32 pos)
	{
		position.x = pos;
        position.x = Min(position.x, 0.f);
        position.x = Max(position.x, -elementSize.x);
	}
	
	void ScrollHelper::SetPosition(const Vector2& pos)
	{
		position = pos;
        position.x = Min(position.x, 0.f);
        position.x = Max(position.x, -elementSize.x);
        position.y = Min(position.y, 0.f);
        position.y = Max(position.y, -elementSize.y);
	}
	
	void ScrollHelper::SetElementSize(float32 newSize)
	{
		elementSize.x = newSize;
		virtualViewSize.x = viewSize.x;
		if(viewSize.x > elementSize.x)
		{
			virtualViewSize.x = elementSize.x;
		}
	}
	
	float ScrollHelper::GetPosition()
	{
		return position.x;
	}
	
	void ScrollHelper::SetViewSize(float32 size)
	{
		viewSize.x = size;
		virtualViewSize.x = viewSize.x;
		if(viewSize.x > elementSize.x)
		{
			virtualViewSize.x = elementSize.x;
		}
	}
    
    float32 ScrollHelper::GetViewSize()
    {
        return viewSize.x;
    }
    
    float32 ScrollHelper::GetElementSize()
    {
        return elementSize.x;
    }

	float32 ScrollHelper::GetCurrentSpeed()
	{
		return speed;
	}

	void ScrollHelper::SetSlowDownTime(float newValue)
	{
		slowDown = newValue;
	}
	
	void ScrollHelper::SetBorderMoveModifer(float newValue)
	{
		backward = newValue;
	}

	void ScrollHelper::Impulse(float impulseSpeed)
	{
		if((position.x > 0 && impulseSpeed > 0) || (position.x + elementSize.x < virtualViewSize.x && impulseSpeed < 0))
		{
			return;
		}
		speed = impulseSpeed;
	}

	
	float ScrollHelper::GetPosition(float positionDelta, float timeDelta, bool isPositionLocked)
	{
		if(isPositionLocked)
		{
			if(position.x + positionDelta > 0)
			{
				positionDelta *= (1.0f - position.x / virtualViewSize.x) * backward;
			}
			if(position.x + elementSize.x + positionDelta  < virtualViewSize.x)
			{
                
				positionDelta *= (1.0f - (virtualViewSize.x - (position.x + elementSize.x)) / virtualViewSize.x) * backward;
			}
			position.x += positionDelta;
			speed = 0;
			MovesDelta m;
			m.deltaMove = positionDelta;
			m.deltaTime = timeDelta;
			moves.push_back(m);
			totalDeltaTime += timeDelta;
			totalDeltaMove += positionDelta;
			if(totalDeltaTime >= 0.4)
			{
				List<MovesDelta>::iterator it = moves.begin();
				totalDeltaTime -= it->deltaTime;
				totalDeltaMove -= it->deltaMove;
				moves.erase(it);
			}
		}
		else
		{
			if(totalDeltaMove != 0)
			{
				speed = totalDeltaMove / totalDeltaTime;
				speed = Min(speed, virtualViewSize.x * 2);
				speed = Max(speed, -virtualViewSize.x * 2);
			}
			
			if(position.x > 0)
			{
				if(backward != 0 && slowDown != 0)
				{
					if(slowDown != 0)
					{
						speed -= virtualViewSize.x * timeDelta / slowDown / backward;
					}
					else
					{
						speed -= virtualViewSize.x * timeDelta * 4 / backward;
					}
					position.x += speed * timeDelta;
					if(position.x < 0)
					{
						position.x = 0;
						speed = 0;
					}
				}
				else
				{
					position.x = 0;
					speed = 0;
				}
			}
			else if(position.x + elementSize.x < virtualViewSize.x)
			{
				if(backward != 0)
				{
					if(slowDown != 0)
					{
						speed += virtualViewSize.x * timeDelta / slowDown / backward;
					}
					else
					{
						speed += virtualViewSize.x * timeDelta * 4 / backward;
					}
					position.x += speed * timeDelta;
					if(position.x + elementSize.x > virtualViewSize.x)
					{
						position.x = virtualViewSize.x - elementSize.x;
						speed = 0;
					}
				}
				else
				{
					position.x = virtualViewSize.x - elementSize.x;
					speed = 0;
				}
			}
			else if(speed != 0)
			{
				if(slowDown != 0)
				{
					float oldSpd = speed;
					speed = speed - speed / slowDown * timeDelta;
					if((oldSpd > 0 && speed < 1.0) || (oldSpd < 0 && speed > -1.0))
					{
						speed = 0;
					}
					position.x += speed * timeDelta;
				}
				else
				{
					speed = 0;
				}
			}
			
			moves.clear();
			totalDeltaTime = 0;
			totalDeltaMove = 0;
		}
		
		return position.x;
	}
}




