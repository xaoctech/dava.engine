/*==================================================================================
    Copyright (c) 2008, DAVA Consulting, LLC
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are met:
    * Redistributions of source code must retain the above copyright
    notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
    notice, this list of conditions and the following disclaimer in the
    documentation and/or other materials provided with the distribution.
    * Neither the name of the DAVA Consulting, LLC nor the
    names of its contributors may be used to endorse or promote products
    derived from this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE DAVA CONSULTING, LLC AND CONTRIBUTORS "AS IS" AND
    ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL DAVA CONSULTING, LLC BE LIABLE FOR ANY
    DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
    ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
    SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

    Revision History:
        * Created by Ivan Petrochenko
=====================================================================================*/
#include "Debug/Stats.h"
#include "Platform/SystemTimer.h"

namespace DAVA
{

Stats::Stats()
#if defined(__DAVAENGINE_ENABLE_DEBUG_STATS__)
    : cache(500)
#endif
{
#if defined(__DAVAENGINE_ENABLE_DEBUG_STATS__)
    globalId = 0;
    skipFrameCount = -1;
    frame = 0;
#endif
}

Stats::~Stats()
{
    
}
    
bool CompareFunc(std::pair<String, uint32> & a, std::pair<String, uint32> & b)
{
    if (a.first < b.first)
        return true;
    else return false;
}
    
void Stats::RegisterEvent(const String & eventName, const String & eventDescription)
{
#if defined(__DAVAENGINE_ENABLE_DEBUG_STATS__)
    
    Map<String, uint32>::iterator it = eventIds.find(eventName);
    if (it != eventIds.end())
    {
        // if event already exists return.
        return;
    }

    EventDescription eventDesc;
    
    eventDesc.name = eventName;
    eventDesc.description = eventDescription;
    eventDesc.id = globalId++;
    
    eventMap[eventDesc.id] = eventDesc;
    eventIds[eventName] = eventDesc.id;
    
    // recalc parent map
    const Map<String, uint32>::iterator & end = eventIds.end();
    for (Map<String, uint32>::iterator parent = eventIds.begin(); parent != end; ++parent)
    {
        eventMap[parent->second].parent = (uint32)-1;
        eventMap[parent->second].childs.clear();
    }
    
//    Logger::Debug("start");
    for (Map<String, uint32>::iterator parent = eventIds.begin(); parent != end; ++parent)
    {
        const String & parentName = parent->first;
        for (Map<String, uint32>::iterator child = eventIds.begin(); child != end; ++child)
        {
            const String & childName = child->first;
            
//            Logger::Debug("compare: %s - %s", parentName.c_str(), childName.c_str());
            
            if ((childName.find(parentName) == 0) && (childName != parentName))
            {
//                Logger::Debug("found");
                eventMap[parent->second].childs.push_back(child->second);
                eventMap[child->second].parent = parent->second;
                //break;
            }
        }
    }
    
    int32 cnt = 0;
    sortedNames.resize(eventIds.size());
    for (Map<String, uint32>::iterator it = eventIds.begin(); it != end; ++it)
    {
        sortedNames[cnt++] = std::pair<String, uint32>(it->first, it->second);
    }
    std::sort(sortedNames.begin(), sortedNames.end());
#endif
}
    
void Stats::BeginTimeMeasure(const String & eventName, BaseObject * owner)
{
#if defined(__DAVAENGINE_ENABLE_DEBUG_STATS__)
    Map<String, uint32>::iterator it = eventIds.find(eventName);
    if (it != eventIds.end())
    {
        Event * event = cache.New();
        event->type = Event::TYPE_EVENT_BEGIN;
        event->id = it->second;
        event->time = SystemTimer::Instance()->AbsoluteMS();
        event->owner = owner;
        events.push_back(event);
    }
#endif
}
    
void Stats::EndTimeMeasure(const String & eventName, BaseObject * owner)
{
#if defined(__DAVAENGINE_ENABLE_DEBUG_STATS__)
    Map<String, uint32>::iterator it = eventIds.find(eventName);
    if (it != eventIds.end())
    {
        Event * event = cache.New();
        event->type = Event::TYPE_EVENT_END;
        event->id = it->second;
        event->time = SystemTimer::Instance()->AbsoluteMS();
        event->owner = owner;
        events.push_back(event);
    } 
#endif
}
    
void Stats::EnableStatsOutputEventNFrame(int32 _skipFrameCount)
{
#if defined(__DAVAENGINE_ENABLE_DEBUG_STATS__)
    skipFrameCount = _skipFrameCount;
#endif
}

void Stats::BeginFrame()
{
    
}
    

void Stats::EndFrame()
{
#if defined(__DAVAENGINE_ENABLE_DEBUG_STATS__)
    
    
    Map<uint32, uint64> timeForEvent;
    for (List<Event*>::reverse_iterator it = events.rbegin(); it != events.rend(); ++it)
    {
        Event * event = *it;
        if (event->type == Event::TYPE_EVENT_BEGIN)
        {
            timeForEvent[event->id] -= event->time;
            if (eventMap[event->id].parent != (uint32)-1)
                timeForEvent[eventMap[event->id].parent] -= event->time;
        }else if (event->type == Event::TYPE_EVENT_END)
        {
            timeForEvent[event->id] += event->time;
            if (eventMap[event->id].parent != (uint32)-1)
                timeForEvent[eventMap[event->id].parent] += event->time;
        }
    }
    events.clear();
    
    
    if (skipFrameCount != -1)
    {
        if (frame > skipFrameCount)
        {
            int32 size = sortedNames.size();
            for (int32 k = 0; k < size; ++k)
            {
                uint32 eventId = sortedNames[k].second;
                Logger::Debug("%s - %lld ms", sortedNames[k].first.c_str(), timeForEvent[eventId]);
            }
            frame = 0;
        }
    }   
    frame++;
#endif
}
    
    
    

};
